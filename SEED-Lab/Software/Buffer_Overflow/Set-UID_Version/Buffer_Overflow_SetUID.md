# Buffer Overflow Attack Lab (Set-UID Version)
- this is another lab in *Buffer Overflow*, however, this time, we will exploit the vulnerability in a local Set-UID root program to run arbitrary command with root privilege
- topics
    - stack layout
    - address randomization, non-executable stack, and StackGuard
    - shellcode (32-bit and 64-bit)
    - the return-to-libc attack (which aims at defeating the non-executable stack countermeasure, is covered in a separate lab)
## Lab Setup
- turn off address space randomization `sudo sysctl -w kernel.randomize_va_space=0`
- configure `/bin/sh`: in the recent versions of Ubuntu OS, `/bin/sh` is a symbolic link points to `/bin/dash`, which has a countermeasure to prevents itself from being executed in a Set-UID process, i.e., it will set the effective user ID to the process's real user ID, dropping the privilege, we reset `/bin/sh`, make it point to `/bin/zsh`:
    - `sudo ln -sf /bin/zsh /bin/sh`
- StackGuard and Non-Executable Stack: they are implemented in the system. We can turn them off during compilation (`-z execstack` and `-fno-stack-protector`)

## Task 1: Getting Familiar with Shellcode
- we have done this in the Buffer Overflow Server Version Lab

## Task 2: Understanding the Vulnerable Program
- just like the the Buffer Overflow Server Version Lab, the program first takes at most 517 bytes of data, then it uses `strcpy` to copy the data to a buffer smaller than 517 bytes, this is where the buffer overflow can happen
- the program is a root-owned Set-UID program, which means if a normal user can exploit the buffer overflow vulnerability, they can execute arbitrary code with root privilege
- set a program a Set-UID root program
    ```
    $ sudo chown root stack
    $ sudo chmod 4755 stack <-- other users have the read and execute permission
    ```

## Task 3: Launching Attack on 32-bit Program (Level 1)

## Task 4: Launching Attack without Knowing Buffer Size (Level 2)

## Task 5: Launching Attack on 64-bit Program (Level 3)

## Task 6: Launching Attack on 64-bit Program (Level 4)

## Task 7: Defeating dash's Countermeasure

## Task 8: Defeating Address Randomization

## Task 9: Experimenting with Other Countermeasures
