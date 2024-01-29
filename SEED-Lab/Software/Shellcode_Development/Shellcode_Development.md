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
