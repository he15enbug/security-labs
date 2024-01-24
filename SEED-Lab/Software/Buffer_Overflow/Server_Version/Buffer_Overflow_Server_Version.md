# Buffer Overflow (Server Version)

## Lab Environment Setup
- SEED Ubuntu 20.04 VM
- Turn off countermeasures
    - `$ sudo /sbin/sysctl -w kernel.randomize_va_space=0`

- Compilation
    - `gcc -DBUF_SIZE=$(L1) -o stack -z execstack -fno-stack-protector stack.c`
    - `-z execstack` Turn off the non-executable stack protection
    - `-fno-stack-protector stack.c` Turn off the StackGuard

## Task 1: Get Familiar with the Shellcode
- The goal of buffer-overflow attacks is to inject malicious code into the program.
- Shellcode is widely used in most code-injection attacks.
- **Shellcode**: a piece of code that launches a shell, and is usually written in assembly language.
- The shellcode is provided, modify it to run arbitrary command, but ensure not to change the length of the argument string. For more information about shellcode, refer to the **Shellcode Development Lab**

## Task 2: Level-1 Attack
- Target Server: `10.9.0.5:9090`
- Send a benign message to the server:
    ```
    $ echo hello | nc 10.9.0.5 9090
    
    ```
