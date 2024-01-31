# Dirty COW Attack Lab
- The Dirty COW vulnerability is an interesting case of the race condition vulnerability, it exists in the Linux kernel since September 2007, and was discovered and exploited in October 2016
- The vulnerability resides in the code of copy-on-write inside Linux kernel, by exploting it, attackers can modify any protected file

## Labsetup
- [`Ubuntu 12.04`](https://seedsecuritylabs.org/labsetup.html)

## Task 1: Modify a Dummy Read-Only File
- objective: write to a read-only file using the Dirty COW vulnerability
- create a dummy file: create `/zzz` with permission `644 (-rw-r--r--)`, and put some random content in it
    ```
    $ sudo touch /zzz
    $ ls -l /zzz
    -rw-r--r-- root root 0 Jan 31 05:26 /zzz
    $ sudo echo 111222333 > /zzz
    $ cat /zzz
    111222333
    $ echo 123 > /zzz
    bash: /zzz: Permission denied
    ```
    - our goal is to replace the pattern `"222"` with `"***"`

- set up the memory mapping thread
    - the program `cow_attack.c` has three threads:
        - the main thread: maps `/zzz` to memory, finds where the pattern `"222"` is, and then creates 2 threads to exploit the Dirty COW vulnerability in the OS kernel
        - the write thread
        - the madvise thred


## Task 2: Modify the Password File to Gain the Root Privilege