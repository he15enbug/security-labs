# Spectre Attack Lab
- the Spectre attack exploits critical vulnerabilities existing in many modern processors, including those from Intel, AMD, and ARM. The vulnerabilities allow a program to break inter-process and intra-process isolation, so a malicious program can read the data from the area that is not accessible to it
- topics
    - Spectre attack
    - side channel attack
    - CPU caching
    - out-of-order execution and branch prediction inside CPU microarchitecture

- **Lab Environment**
    - works on both Ubuntu 16.04 and 20.04 VM
    - some new CPUs may have patched this vulnerability

## Code Compilation
- *for Ubuntu 16.04 OS*: for most of tasks, we need to add `-march=native` flag when compiling the code with `gcc`. The `march` flag telss the compiler to enable all instruction subsets supported by the local machine

## Task 1 and 2: Side Channel Attacks via CPU Caches
- both the Meltdown and Spectre attacks use CPU cache as a side channel to steal protected secret. The technique is called FLUSH+RELOAD, which has been introduced in the [Meltdown Attack Lab](https://github.com/he15enbug/security-labs/blob/main/SEED-Lab/System/Meltdown/Meltdown.md)
- task `1` and `2` is the same as in the Meltdown Attack Lab, so they are skipped here

## Task 3: Out-of-Order Execution and Branch Prediction
- *our-of-order execution*: At the microarchitectural level, line 2 involves 2 operations: load the value of `size` from the memory, and compare the value with `x`. If `size` is not in the CPU caches, it may take hundreds of CPU clock cycles before the value is read, instead of sitting idle, modern CPUs try to predict the outcome of the comparison, and speculatively execute the branches based on the estimation. Since such execution starts before the comparison finishes, it is called out-of-order execution
    ```
    data = 0;
    if(x < size) {       <--- line 2
        data = data + 5;
    }
    ```
- for CPUs to perform a speculative execution, they should be able to predict the outcome of the `if` condition. CPUs keep a record of the branches taken in the past, and then use them to predict what branch should be taken in a speculative execution. If we would like a particular branch to be taken in a speculative execution, we should train the CPU
    ```
    int size = 10;
    void victim(size_t x) {
        if(x < size) {
            temp = array[x * 4096 + DELTA];
        }
    }
    int main() {
        // train the CPU to predict `x < size` as true
        for(i = 0; i < 10; i++) {
            victim(i);
        }
    }
    // make accessing value of size to cost more time
    _mm_clflush(&size);
    for(i = 0; i < 256; i++) _mm_clflush(&array[i * 4096 + DELTA]);
    victim(97); // due to previous training, CPU is likely to predict x < size in this function as true
    ```
- result
    ```
    $ ./SpectreExperiment
    array[97 * 4096 + 1024] is in cache
    The Secret = 97
    ```
- if we comment the line `_mm_clflush(&size);`, we will not be able to succeed, because when `size` is in cache, checking `x < size` can be very fast, and when it finishes, the speculative execution haven't reach `temp = array[x * 4096 + DELTA]` yet (because it needs to get the value of `array[x * 4096 + DELTA]` from main memory)

## Task 4: The Spectre Attack
- the fundamental problem of the Meltdown and Spectre attack is that some CPUs with the out-of-order execution feature do not clean the cache, so some information of the out-of-order execution is left behind, which can be used to steal protected secrets

- the secrets can be data in another process (hardware level isolation) or data in the same process (isolation via software, such as sandbox mechanisms). The Spectre attack can be launched against both types of secret, although stealing data from another process is much harder than stealing data from the same process. For the sake of simplicity, this lab only focuses on stealing data from the same process
- with the Spectre attack, we can get CPUs to execute (out-of-order) a protected code branch even if the condition checks fails, essentially defeating the access check

- *the setup for the experiment*
    ```
    +---------------------+
    |       secret        |  (Access NOT allowed)
    +---------------------+  <-- Protection: ensure x<=9
    |      buffer[9]      |
    |---------------------|
    |        ...          |  (Access allowed)
    |---------------------|
    |      buffer[0]      |
    +---------------------+  <-- Protection: ensure x>=9
    |       secret        |  (Access NOT allowed)
    +---------------------+
    ```
- there are 2 types of regions: restricted region and non-restricted region. The restriction is achieved via an if-condition implemented in a sandbox function described below. The sandbox function returns the value of `buffer[x]` for an `x` value provided by users, only if `x` is between the buffer's lower and upper bounds. Therefore, this sandbox function will never return anything in the restricted area to users
    ```
    unsigned int bound_lower = 0;
    unsigned int bound_upper = 9;
    uint8_t buffer[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    // Sandbox Function
    uint8_t restrictAccess(size_t x) {
        if(x <= bound_upper && x >= bound_lower) {
            return buffer[x];
        }
        else {
            return 0;
        }
    }
    ```
- there is a secret value in the restricted area (either above or below the buffer), the attacker knows its address, but the attacker cannot directly access the memory holding the secret value

- basic Spectre attack (assume we cannot directly access the `secret`)
    ```
    $ gcc -march=native -o SpectreAttack SpectreAttack.c
    $ ./SpectreAttack
    secret: 0x80487a0
    buffer: 0x804a024
    index of secret (out of bound): -6276
    array[0*4096 + 1024] is in cache
    The secret = 0().
    array[83*4096 + 1024] is in cache
    The secret = 83(S).
    ```
- in most cases, I can get the secret `83`, sometimes, I cannot get it. This is due to the speed of the CPU, the faster the CPU is, the more instructions it can execute (out of order) before getting the value of `bound_lower` and `bound_upper` from main memory, and the higher the probability it loads `array[secret*4096 + 1024]` to the cache. My CPU is fast enough, so in most cases, the attack will succeed
- one problem is that there may be noise in the cache (because sometimes CPU load extra values in cache), I wrote a shell script to run the program for 5000 times, I got each of `array[3*4096 + 1024]`, `array[4*4096 + 1024]`, `array[227*4096 + 1024]`, and `array[228*4096 + 1024]` in the cache for a few times

## Task 5: Improve the Attack Accuracy
- use a statistical approach, just like what we did in task `8` of Meltdown Attack Lab, but we need to exclude `0` first, because in each of the loops, it will eventually load `array[0*4096 + 1024]` to cache, because after the check fails, the function will return `0`

- we need to adjust the value in `usleep()` to increase success rate

## Task 6: Steal the Entire Secret String
- we just need to launch Spectre attack for indexes in `[index_beyond, index_beyond+16]` (because the secret is 17 bytes)
- result
    ```
    $ gcc -march=native -o SpectreEntireSecret SpectreEntireSecret.c
    $ ./SpectreEntireSecret
    The Entire Secret:
    Some Secret Value
    ```
