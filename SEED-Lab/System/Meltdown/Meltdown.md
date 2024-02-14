# Meltdown Attack Lab
- discovered in 2017 and publicly disclosed in January 2018, the Meltdown exploits critical vulnerabilities existing in many modern processors, including those from Intel and ARM. The vulnerabilities allow a user-level program to read data stored inside the kernel memory. Such an access is not allowed by the hardware protection mechanism implemented in most CPUs, but a vulnerability exists in the design of these CPUs that makes it possible to defeat the hardware protection. Because the flaw exists in the hardware, it is very difficult to fundamentally fix the problem, unless we change the CPUs in our computers
- the Meltdown vulnerability represents a special genre of vulnerabilities in the design of CPUs
- topics
    - Meltdown attack
    - side channel attack
    - CPU caching
    - out-of-order execution inside CPU microarchitecture
    - kernel memory protection in OS
    - kernel module

- **Lab Environment**
    - SEED Ubuntu 16.04 VM (on 20.04, only task 1 to 6 work as expected, task 7 and 8 will not work due to the countermeasure implemented inside the OS)
    - Meltdown vulnerability is a flaw inside Intel CPUs, the attack will not work on ARM CPUs

## Code Compilation
- *for Ubuntu 16.04 OS*: for most of tasks, we need to add `-march=native` flag when compiling the code with `gcc`. The `march` flag telss the compiler to enable all instruction subsets supported by the local machine

## Task 1 and 2: Side Channel Attacks via CPU Caches
- both the Meltdown and Spectre attacks use CPU cache as a side channel to steal a protected secret. The technique used in this side-channel attack is called *FLUSH+RELOAD*
- a CPU cache is a hardware cache used by the CPU of a computer to reduce the average cost (time or energy) to access data from the main memory. Accessing data from CPU cache is much faster than accessing from the main memory. When data are fetched from the memory, they are usually cached by the CPU, so if the same data are used again, the access speed will be much faster. When a CPU needs to access some data, it first looks at its caches, if the data is there (this is called *cache hit*), it will be fetched directly. Otherwise (this is called *cache miss*), the CPU will go to the main memory to get the data. The time spent in the letter case is significant longer

### Task 1: Reading from Cache versus from Memory
- the cache memory is used to provide data to the high speed processors at a faster speed. Use `CacheTime.c` to see the time difference between reading data from cache and main memory. In the code, we have an array of size `10*4096`, we first access two of its elements, `array[3*4096]` and `array[9*4096]`. Then, the pages containing these two elements will be cached. We then read the elements from `array[0*4096] to array[9*4096]` and measure the time spent in the memory reading
- caching is done at the cache block level, not at the byte level, a typical block size is 64 bytes. We use `array[k*4096]`, so no two elements will fall into the same cache block
- compile (with `-march=native` option) and run the code (run 10 times), here is one of the results
    ```
    Access time for array[0*4096]: 378 CPU cycles
    Access time for array[1*4096]: 256 CPU cycles
    Access time for array[2*4096]: 252 CPU cycles
    Access time for array[3*4096]: 48 CPU cycles
    Access time for array[4*4096]: 260 CPU cycles
    Access time for array[5*4096]: 252 CPU cycles
    Access time for array[6*4096]: 254 CPU cycles
    Access time for array[7*4096]: 50 CPU cycles
    Access time for array[8*4096]: 248 CPU cycles
    Access time for array[9*4096]: 278 CPU cycles
    ```
- in all 10 times, the time for accessing `array[3*4096]` and `array[7*4096]` (less than 100 CPU cycles) are much shorter than accessing other elements (more than 200 CPU cycles). To distinguish the two types of memory access, we can use `150` as a threshold

### Task 1: Using Cache as a Side Channel
- the objective of this task is to use the side channel to extract a secret value used by the victim function. Assume there is a victim function that uses a secret value as index to load some values from an array. Also assume that the secret value cannot be accessed from the outside. Our goal is to use side channel to get this secret value. We will use FLUSH+RELOAD technique, it consists of 3 steps:
    1. FLUSH the entire array from the cache memory to make sure the array is not cached
    2. Invoke the victim function, which accesses one of the array elements based on the value of the secret. This action cause the corresponding array element to be cached
    3. RELOAD the entire array, and measure the time it takes to reload each element. If one specific element's loading time is shorter, it is very likely that element is already in the cache
- `FlushReload.c` uses FLUSH+RELOAD to find out a one-byte secret value contained in the variable `secret`. Since there are 256 possible values for a one-byte secret, we need to map each value to an array element. The naive way is to define an array of 256 elements, but it doesn't work, as caching is done at a block level, e.g., when element `0` is cached, it will cache a block from element `0` to `63` (assume the block size is 64). We create an array of `256*4096` bytes, each element used in our RELOAD step is `array[k*4096]`, this ensures that each 2 elements cannot be in the same cache block. We also need to avoid using `array[0*4096]`, as it is adjacent to some other variables in the memory, and be cached when those variables are cached. Just use a `DELTA=1024`, we RELOAD `array[k*4096+1024]`
- in the code, it uses 80 as the accessing time threshold, using all values from 80 to 200 should work
- compile and run `FlushReload.c`, we can get the secret
    ```
    $ gcc -march=native -o FlushReload FlushReload.c
    $ ./FlushReload
    array[94*4096 + 1024] is in cache.
    The Secret = 94.
    ```

## Task 3-5: Preparation for the Meltdown Attack
- memory isolation is the foundation of system security. In most OS, kernel memory is not directly accessible to user-space programs. This isolation is achieved by a *supervisor bit* of the processor that defines whether a memory page of the kernel can be accessed or not. This bit is set when CPU enters the kernel space and cleared when it exits to the user space. With this feature, kernel memory can be safely mapped into the address space of every process, so the page table does not need to change when a user-level program traps into the kernel. However, this isolation is broken by Meltdown attack, which allow unprivileged user-level programs to read arbitrary kernel memory

### Task 3: Place Secret Data in Kernel Space
- to simplify our attack, we store a secret data in the kernel space, and we show how a user-level program can find out what the secret data is. We use a kernel module to store the secret data. The implementation of the kernel module is provided in `MeltdownKernel.c`
- two important conditions need to be held, or the attack will fail
    1. we need to know the address of the target secret data. The kernel module saves the address of the secret data into the kernel message buffer, which is publicly accessible, we can get the address from there. In real attacks, attackers have to figure out a way to get the address, or they have to guess
    2. the secret data need to be cached. To achieve this, we just need to use the secret once. We create a data entry `/proc/secret_data`, which provides a window for user-level program to interact with the kernel module. When a user-level program reads from this entry, the `read_proc()` function in the KM will be invoked, inside which, the secret wariable will be loaded and thus be cached by the CPU. `read_proc()` doesn't return the secret data to the user space, so it does not leak the secret data
- compile and execution, use the `Makefile`, the address of the secret data is `0xfbce3000`
    ```
    $ make
    ...
    $ sudo insmod MeltdownKernel.ko <-- insert the KM
    $ dmesg | grep 'secret data address'
    secret data address: fbce3000
    ```

### Task 4: Access Kernel Memory from User Space
- do an experiment to see whether we can directly get the secret from this address or not
    ```
    char *secret_addr = (char *) 0xfbce3000;
    char secret_value = *secret_addr;
    printf("secret has been read\n");
    ```
    ```
    $ gcc -o ReadSecret ReadSecret.c
    $ ./ReadSecret
    Segmentation fault <-- fail to the secret from kernel memory
    ```

### Task 5: Handle Error/Exceptions in C
- in the Meltdown attack, we need to do something after accessing the kernel memory to prevent the program from crashing
- accessing prohibited memory location will raise a SIGSEGV signal, if the program does not handle this exception by itself, the OS will handle it and terminate the program. There are several ways to prevent programs from crashing. One way is to define our own signal handler in the program to capture the exceptions. Unlike C++ or other high-level languages, C does not provide direct support for error handling (aka. exception handling), such as `try/catch` clause. However, we can emulate the `try/catch` clause using `sigsetjmp()` and `siglongjmp()`, here is an example code `ExceptionHandling.c`
    ```
    static sigjmp_buff jbuf;
    static void catch_segv() {
        /* roll back to the checkpoint set by sigsetjmp()
         * the state saved in jbuf will be copied back in the processor, 
         * and computation starts over from the return point of the sigsetjump()
         * but the returned value of the sigsetjmp() is the second argument of the siglongjmp(),
         * which is 1 in our case
         */
        siglongjmp(jbuf, 1);
    }
    int main() {
        unsigned long kernel_data_addr = 0xfbce3000;
        /* register a signal handler
         * when a SIGSEGV signal is raised, catch_segv() will be invoked
         */
        signal(SIGSEGV, catch_segv);

        /* sigsetjmp(jbuf, 1) saves the stack context in jbuf
         * returns 0 when the checkpoint is set up
         */
        if(sigsetjmp(jbuf, 1) == 0) {
            // a SIGSEGV signal will be raised
            char kernel_data = *(char*) kernel_data_addr;
            // this will not be printed out
            printf("Kernel data at address %lu is: %c\n", kernel_data_addr, kernel_data);
        }
        else {
            printf("Memory access violation!\n");
        }
    }
    ```
## Task 6: Out-of-Order Execution by CPU
- from the previous task, we know in the following code, line 3 will raise an exception, so number will always be 0
    ```
    number = 0
    *kernel_addr = (char*) 0xfbce3000;
    kernel_data  = *kernel_addr;        <---- line 3
    number = number + kernel_data;      <---- line 4
    ```
- however, the above statement is not completely true if we get into the CPU, and look at the execution sequence at the microarchitectural level. If we do that, we will find out that line 3 will successfully get the kernel data, and line 4 will be executed. This is due to an important optimization technique adopted by modern CPUs, called *out-of-order execution*

- instead of executing the instructions strictly in their original order, modern high performance CPUs allow out-of-order execution to exhaust all the execution units. Executing instructions one after another may lead to poor performance and inefficient resource usage. With the out-of-order execution feature, CPU can run ahead once the required resources are available

- in the code above, at the microarchitectural level, line 3 involves 2 operations: load the data (usually into a register), and check whether the data access is allowed or not. If the data is already in the CPU cache, the first operation will be quite fast, while the second operation may take a while. To avoid waiting, the CPU will continue executing line 4, and subsequent instructions, while conducting the access check in parallel. This is out-of-order execution. The results of the execution will not be committed before the access check finished. In our case, the check fails, so all the results caused by the out-of-order execution will be discarded like it never happened

- Intel and several CPU makers made a severe mistake in the design of the out-of-order execution. They wipe out the effects of the out-of-order execution on registers and memory if such an execution is not supposed to happen, so the execution doesn't lead to any visible effect. However, they forgot the effect on CPU caches. During the out-of-order execution, the referenced memory is fetched into a register and also cached in CPU

- we can use an example to observe this effect: after the code executed, the check will fail, but `array[7*4096+DELTA]` will be cached in the CPU
    ```
    char kernel_data = 0;
    kernel_data = *(char*) 0xfbce3000;
    array[7*4096+DELTA] += 1;
    ```
    ```
    $ ./MeltdownExperiment
    Memory access violation!
    array[7*4096 + 1024] is in cache
    The Secret = 7.
    ```

## Task 7: The Basic Meltdown Attack
- how far a CPU can go in the out-of-order execution depends on how slow the access check, which is done in parallel, is performed. This is a typical race condition situation. In this task, we will exploit this race condition to steal a secret from the kernel

### Task 7.1: A Native Approach
- in previous task, although we can observe that `array[7*4096 + DELTA]` is in the CPU cache, we do not get any useful information about the secret. If instead of using `array[7*4096 + DELTA]`, we access `array[kernel_data*4096 + DELTA]`, which brings it into the CPU cache. Using the FLUSH+RELOAD technique, we check the access time of `array[i*4096 + DELTA]` for `i=0, ..., 255`, if we find out that only `array[k*4096 + DELTA]` is in the cache, we can infer that the value of the secret is `k`
- modify `MeltdownExperiment.c` to implement this method
- it seems that my Intel CPU has already fixed Meltdown vulnerability, according to my observation， we cannot get the secret value (`'S', 83`): 
    ```
    char kernel_data = 20;
    kernel_data = *(char*) 0xfbce3000;
    array[kernel_data*4096 + DELTA] += 1;
    ```
    - no matter what the initial value of `kernel_data` is, we always get `array[0*4096 + 1024] is in cache`
### Task 7.2: Imporve the Attack by Getting the Secret Data Cached
- to success, we need the out-of-order execution faster than the access check
- in practce, if a kernel data item is not cached, using Meltdown to steal the data will be difficult
- get the kernel data cached before launching the attack

### Task 7.3: Using Assembly Code to Trigger Meltdown
- we probably still cannot succeed, even with secret data being cached by CPU
- improve more by adding a few lines of assembly instructions before the kernel memory access
- in `meltdown_asm()`: do a loop for 400 times, inside the loop, it simply add a number `0x141` to the `eax` register. This code basically does useless computations, these extra lines of code give the algorithmic units something to chew while memory access is being speculated
- after all the improvement, we still get `array[0*4096 + 1024] is in cache`, that means the Meltdown vulnerability may have been patched in my CPU

## Task 8: Make the Attack More Practical
- **TO-DO**: I will redo task 7 and 8 when I find a computer with an old Intel CPU that has not been patched
