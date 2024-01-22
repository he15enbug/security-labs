# Buffer Overflow (Server Version)

## Lab Environment Setup
- SEED Ubuntu 20.04 VM
- Turn off countermeasures
    - `$ sudo /sbin/sysctl -w kernel.randomize_va_space=0`

- Compilation
    - `gcc -DBUF_SIZE=$(L1) -o stack -z execstack -fno-stack-protector stack.c`
    - `-z execstack` Turn off the non-executable stack protection
    - `-fno-stack-protector stack.c` Turn off the StackGuard