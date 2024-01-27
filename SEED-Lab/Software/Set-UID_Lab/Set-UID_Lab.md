# Environment Variable and Set-UID Lab
## Task 1: Manipulating Environment Variables
- Use `printenv` or `env` to print out the environment variables
    - To get some particular environment variables: `printenv PATH` or `env | gerp PATH`
- Use `export` and `unset` to set or unset environment variables (These are Bash's internal commands)
    ```
    $ export x=123
    $ printenv x
    123
    $ unset x
    $ printenv x
    $
    ```

## Task 2: Passing Environment Variables from Parent Process to Child Process
- In Unix, `fork()` creates a new process by duplicating the calling process, the child process is an exact duplicate of the parent process, but several things are not inherited by the child (check this by command `man fork`)
- compile and run `myprintenv.c`, modify it to print out the environment variable of child and parent process respectively, and save the result in `env_child` and `env_parent`
    ```
    $ diff env_child env_parent
    $
    ```
    - there is no difference, meaning that the child process inherited the environment variables of the parent process

## Task 3: Environment Variables and `execve()`
- `execve()` calls a system call to load a new command and execute it, this function never returns (When `execve()` is called successfully, it replaces the current process image with the specified program, and the new program takes over the execution flow)
- No new process is created, the calling process's text, data, bss, and stack are overwritten by that of the program loaded
- what about the environment variables:
    - compile and run `myenv.c`
        - core code: `execve("usr/bin/env", argv, NULL);`
        ```
        $ gcc -o myenv myenv.c
        $ ./myenv
        $
        (nothing printed)
        ```
    - modify, compile and run `myenv.c`
        - modified version: `execve("usr/bin/env", argv, environ);`
        ```
        $ gcc -o myenv myenv.c
        $ ./myenv
        SHELL=/bin/bash
        SESSION_MANAGER=...
        ...
        _=./myenv
        ```
    - conclusion: the new program got its environment variables from external variable `environ`
        1. The definition of `execve()`
            ```
            int execve(const char *filename, char *const argv[], char *const envp[]);
            ```
        2. `environ` is defined as a global variable in the glibc source file [`posix/environ.c`](https://sourceware.org/git/?p=glibc.git;a=blob;f=posix/environ.c)

## Task 4: Environment Variables and `system()`
- While `execve()` executes a command directly, `system()` actually executes `"/bin/sh -c command"`
- in the source code of`system()`, it uses `execl()` to execute `/bin/sh`, in the source code of `execl()`, it calls `execve()` and passing to it the environment variables array
- so when using `system()`, the environment variables of the calling process is passed to the new process `/bin/sh`

- verify with `system("/usr/bin/env")`

## Task 5: Environment Variables and `Set-UID` Programs
- `Set-UID`: when a `Set-UID` programs runs, it assumes the owner's privileges. If the program's owner is root, when anyone runs it, the program gains the root's privileges during execution
- User can affect the behaviors of `Set-UID` programs via environment variables

- check whether environment variables are inherited by the `Set-UID` program's process from user's process
    - write a program that prints out all environment variables in current process
        ```
        $ gcc -o task5 task5.c
        $ sudo chown root task5
        $ sudo chmod 4755 task5
        $ ls -l task5
        -rwsr-xr-x 1 root seed 16816 Jan 27 08:03 task5
        ```
    - set some environment variables (in the user `seed`'s shell process), and run the `Set-UID` program
        ```
        $ export LD_LIBRARY_PATH=x
        $ export DAVID=100
        $ export PATH=$PATH:__FLAG__
        $ ./task5 | grep DAVID
        100
        $ ./task5 | grep LD_LIBRARY_PATH
        $ ./task5 | grep PATH
        /usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin:.:__FLAG__
        ```
    - `PATH` and `DAVID` were passed to the `Set-UID` program, but `LD_LIBRARY_PATH` was **NOT**

## Task 6: `PATH` and `Set-UID` Programs
- In a `Set-UID` program, the code `system("ls")` which uses the relative path for `ls` command may run malicious code `/home/seed/ls`, rather than `/bin/ls`

- exploit ``system("ls")` to run arbitrary code, steps:
    1. prepare the malicious code and compile it
        - in this example, the malicious code runs `system("/bin/ls /usr/secret")`, where `/usr/secret` is a file that only root can read
        - `$ gcc -o ls mal.c`
    2. run the vulnerable program `vul`, then add `/home/seed/malicious_path` to `PATH`, and run `vul` again
        ```
        $ ./vul
        vul vul.c
        $ export PATH=/home/seed/malicious_path:$PATH
        $ ./vul
        $ /bin/ls
        my_virus vul vul.c
        ```
    - the file `my_virus` has been created, but this code is not running in root privilege

    - to know what the current user id is, replace `/home/seed/malicious_path/ls` with `/bin/id`, and run `vul` again: 
        ```
        $ ./vul
        uid=1000(seed) gid=1000(seed) groups=1000(seed),4(adm),24(cdrom),...
        ```

- countermeasure
    - `system()` executes `/bin/sh` to run the command
    - in Ubuntu 20.04 and several versions before, `/bin/sh` is actually a symbolic link pointing to `/bin/dash`
    - in `/bin/dash`, if it detects that it's executed in a `Set-UID` progress, it will change the effective user UI to the process's real user ID, essentially dropping the privilege

    - to make the attack success, link `/bin/sh` to `/bin/zsh`
        - `$ sudo ln -sf /bin/zsh /bin/sh`

    - after disable this countermeasure, try again, and `vul` runs with root privilege(`euid=0(root)`):
        ```
        $ ./vul
        uid=1000(seed) gid=1000(seed) euid=0(root) groups=1000(seed),4(adm),24(cdrom),...
        ```

## Task 7: 



