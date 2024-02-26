# Pseudo Random Number Generation Lab
- Generating random number is a quite common task in security software. In many cases, encryption keys are not provided by users, but are instead generated inside the software. Their randomness is extremely important
- Many developers know how to generate random numbers (e.g., for Monte Carlo simulation) from their prior experiences, so they use similar methods to generate the random numbers for security purpose. Unfortunately, a sequence of random numbers may be good for Monte Carlo simulation, but they may be bad for encryption keys. Mistakes have been made in some well-known products, including Netscape and Kerberos
- Topics
    - Pseudo random number generation
    - Mistakes in random number generation
    - Generating encryption key
    - The `/dev/random` and `/dev/urandom` device files

## Task 1: Generate Encryption Key in a Wrong Way
- to generate good pseudo random numbers, we need to start with something that is random, otherwise, the outcome will be quite predictable. The following program uses the current time as a seed for the pseudo random number generator, `time()` returns the time as the number of seconds since the Epoch, `1970-01-01 00:00:00 +0000 (UTC)`
    ```
    #include <stdio.h>
    #include <stdlib.h>
    #include <time.h>

    #define KEYSIZE 16

    void main() {
        int i;
        char key[KEY_SIZE];
        printf("%lld\n", (long long) time(NULL));
        srand(time(NULL));

        for(i = 0; i < KEYSIZE; i++) {
            key[i] = rand() % 256;
            printf("%.2x", (unsigned char) key[i]);
        }
        printf("\n");
    }
    ```
- run the code, and comment out `srand(time(NULL))`, and run again. Explain the purpose of the `srand()` and `time()` functions in the code
    1. with `srand(time(NULL))`
        ```
        $ ./rd
        1708934103
        02cc857c06a01d66e4993e94ecd23307
        $ ./rd
        1708934103
        02cc857c06a01d66e4993e94ecd23307
        $ ./rd
        1708934104
        6138e5e84c78bab67e5dd12c0cc55eea
        ```
    2. comment out `srand(time(NULL))`
        ```
        $ ./rd
        1708934220
        67c6697351ff4aec29cdbaabf2fbe346
        $ ./rd
        1708934221
        67c6697351ff4aec29cdbaabf2fbe346
        ```
    - `srand(time(NULL))` use the current time as the seed to generate random numbers, so, inside a second, the keys generated are the same in each run, but in another second, the generated keys are different, and we cannot observe any pattern. If we comment out `srand(time(NULL))`, the keys generated in each run are always the same

## Task 2: Guessing the Key
- on April 17, 2018, Alice finished her tax return, and she saved the return (a PDF file) on her disk. To protect the file, she encrypted the PDF file using a key generated from the program described in Task 1. She wrote down the key in a notebook, which is securely stored in a safe. A few month later, Bob broke into her computer and get a copy of the encrypted tax return
- Bob cannot get the encryption key, but he got the key-generation program, and he noticed the timestamp of the encrypted file, which is `2018-04-17 23:08:49`, he guessed that the key may be generated within a two-hour window before the file was created
- Since the file is a PDF file, which has a header, the beginning part of the header is always the version number. Around the time when the file was created, PDF-1.5 was the most common version, i.e., the header starts with `%PDF-1.5`, which is 8 bytes of data. The next 8 bytes of the data are quite easy to predict as well. So, Bob got the first 16 bytes of the plaintext, and he knows that the file is encrypted using `aes-128-cbc` based on the metadata. Bob also knows the IV from the encrypted file (IV is never encrypted). Here is what Bob knows
    ```
    Plaintext:  255044462d312e350a25d0d4c5d80a34
    Ciphertext: d06bf9d0dab8e8ef880660d2af65aa82
    IV:         09080706050403020100A2B2C2D2E2F2
    ```
- *task*: find out Alice's encryption key
    - first, get the time range (in seconds)
        ```
        $ date -d "2018-04-17 23:08:49" +%s
        1524020929
        $ date -d "2018-04-17 21:08:49" +%s
        1524013729
        ```
    - write a C program to try all keys

    - result:
        ```
        Find the key:
        95 fa 20 30 e7 3e d3 f8 da 76 1b 4e b8 05 df d7 
        Timestamp:1524017695
        ```
## Task 3: Measure the Entropy of Kernel
- in the virtual world, it is difficult to create randomness, i.e., software alone is hard to create random numbers. Most systems resort to the physical world to gain the randomness. Linux gains the randomness from the following physical resources
    ```
    void add_keyboard_randomness(unsigned char scancode);
    void add_mouse_randomness(__u32 mouse_data);
    void add_interrupt_randomness(int irq);
    void add_blkdev_randomness(int major);
    ```
- the first two are quite straightforward to understand, the first one uses the timing between key presses; the second one uses mouse movement and interrupt timing; the third one gathers random numbers using the interrupt timing. Of course, not all interrupts are good sources of randomness. For example, the timer interrupt is not a good choice, because it is predictable. However, disk interrupts are a better measure. The last one measures the finishing time of block device requests
- the randomness is measured using *entropy*, which is different from the meaning of entropy in the information theory. Here, it simply means how many bits of random numbers the system currently has. We can find out how much entropy the kernel has at the current moment using this command:
    - `cat /proc/sys/kernel/random/entropy_avail`
- we can also use `watch` to monitor the change of the entropy, which will executes a program periodically, the following command runs the `cat` program every 0.1 second
    - `watch -n .1 cat /proc/sys/kernel/random/entropy_avail`
- when we run this command, move the mouse, click the mouse, and type something, read a large file, visit a website, and see what happens to the entropy

## Task 4: Get Pseudo Random Numbers from `/dev/random`
- Linux stores the random data collected from the physical resources into a random pool, and then uses two devices to turn the randomness into pseudo random numbers. These two devices are `/dev/random` and `/dev/urandom`. They have different behaviors. The `/dev/random` device is a blocking device. Namely, every time a random number is given out by this device, the entropy of the randomness will be decreased. When the entropy reaches zero, `/dev/random` will block, until it gains enough randomness
- run `$ cat /dev/random | hexdump` to keep reading pseudo random numbers from `/dev/random`, and print out them with a nice format using `hexdump`. At the same time, use `watch` to monitor the entropy
- result: after running `cat` command, it will print out a set of hex numbers, and we will see the entropy of the kernel decrease to a value less than 60. If we move our mouse, each time the entropy reaches around 60, it will become nearly 0, and a new number will be printed out by `cat`

- *Question*: If a server uses `/dev/random` to generate the random session key with a client, how to launch a DoS attack?
    - attackers can create a lot of connections with the server, if the creation of new connections is fast enough, it will keep the `/dev/random` blocking, and the server will not be able to generate session key for other clients 

## Task 5: Get Pseudo Random Numbers from `/dev/urandom`
- Linux provides another way to access the random pool via the `/dev/urandom` device, except that this device will not block, both `/dev/urandom` and `/dev/random` use the random data from the pool to generate pseudo random numbers. When the entropy is not sufficient, `/dev/random` will block, while `/dev/urandom` will keep generating new numbers. Think of the data in the pool as the seed, and as we know, we can use a seed to generate as many pseudo random numbers as we want
- similarly, run `cat /dev/urandom | hexdump`, this time, it will not stop even if we do not move the mouse for a long time
- we can use a tool called `ent` to measure the quality of the random number. `ent` applies various tests to sequences of bytes stored in files and reports the results of those tests. It is useful for evaluating pseudo-random number generators for encryption and statistical sampling applications, compression algorithms, and other applications where the information density of a file is of interest
- first, generate 1 MB of pseudo random number from `/dev/urandom` and save them in a file. Then, run `ent` on the file
    ```
    $ head -c 1M /dev/urandom > output.bin
    $ ent output.bin 
    Entropy = 7.999811 bits per byte.

    Optimum compression would reduce the size
    of this 1048576 byte file by 0 percent.

    Chi square distribution for 1048576 samples is 275.32, and randomly
    would exceed this value 18.24 percent of the times.

    Arithmetic mean value of data bytes is 127.4698 (127.5 = random).
    Monte Carlo value for Pi is 3.143635344 (error 0.07 percent).
    Serial correlation coefficient is -0.000732 (totally uncorrelated = 0.0).
    ```
- Theoretically speaking, the `/dev/random` device is more secure, but in practice, there is not much difference, because the "seed" used by `/dev/urandom` is random and non-predictable (it does re-seed whenever new random data become available). A big problem of the blocking behavior of `/dev/random` is that blocking can lead to denial of service attacks. Therefore, it is recommended that we use `/dev/urandom` to get random numbers. To do that in a program, we just need to read directly from this device file
    ```
    #define LEN 16
    unsigned char *key = (unsigned char *) malloc(sizeof(unsigned char) * LEN);
    FILE* random = fopen("/dev/urandom", "r");
    fread(key, sizeof(unsigned char) * LEN, 1, random);
    fclose(random);
    ```
- *task*: generate 256-bit (32 bytes) encryption key using `/dev/urandom`
    - solution: just `#define LEN 32`
    - result
        ```
        $ gcc -o key_gen_256 key_gen_256.c 
        $ ./key_gen_256 
        ae 69 85 55 71 19 23 75 d5 6a 6d 59 8e 5a 8d 0c 2c ac d7 e1 f1 15 d2 83 16 9e eb 69 7b af 5f c7
        ```
