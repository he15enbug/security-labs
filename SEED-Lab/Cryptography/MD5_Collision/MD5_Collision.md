# MD5 Collision Attack Lab
- A secure one-way hash function needs to satisfy two properties: the one-way property and the collision-resistance property. The one-way property ensures that given a hash value `h`, it is computationally infeasible to find an input `M`, such that `hash(M)=h`. The collision-resistance property ensures that it is computationally infeasbile to find two different inputs `M1` and `M2`, such that `hash(M1)=hash(M2)`
- Several widely-used one-way hash functions have trouble maintaining the collision-resistance property, e.g., MD5, SHA-1
- Topics
    - One-way hash function, MD5
    - The collision-resistance property
    - Collision attacks

## Task 1: Generating Two Different Files with the Same MD5 Hash
- use the `md5collgen` program, generate 2 files with the same MD5 hash value, and the begining parts of the 2 files need to be the same
    - usage: `./md5collgen -p prefix.txt -o out1.bin out2.bin`
    ```
    $ diff out1.bin  out2.bin 
    Binary files out1.bin and out2.bin differ
    $ md5sum out1.bin 
    3d9241df9e72289af04191e3b8a0dbd5  out1.bin
    $ md5sum out2.bin
    3d9241df9e72289af04191e3b8a0dbd5  out2.bin
    ```
- if the length of the prefix file is not multiple of 64, the program will use `0x00` as padding to make the prefix size multiple of 64
- if we use a prefix file of exactly 64 bytes, no `0x00` padding will be added to the prefix
- there are only several bytes that are different

## Task 2: Understanding MD5's Property
- MD5 is a quite complicated algorithm, but from very high level, it is not so complicated. It divides the input data into blocks of 64 bytes, and then computes the hash iteratively on these blocks. The core of MD5 algorithm is a compression function, which takes 2 inputs, a 64-byte data block and the outcome of the previous iteration. The compression function produces a 128-bit IHV, which stands for *Intermediate Hash Value*. This output is then fed into the next iteration. If the current iteration is the last one, the IHV will be the final hash value. The IHV input of the first iteration `IHV0` is a fixed value
- Based on how MD5 works, we can derive the following property of the MD5 algorithm: Given two inputs `M` and `N`, if `MD5(M)=MD5(N)`, then for any input `T`, `MD5(M||T)=MD5(N||T)`, `||` represents concatenation
- We can also try a few cases to test it
    - write any content in a file `suffix.txt`
    ```
    $ echo saddsaajkdndjnkdsfkjna > suffix.txt
    $ cat out1.bin suffix.txt > o1suf
    $ cat out2.bin suffix.txt > o2suf
    $ md5sum o1suf
    24bf58c87b04267367217b94a214c4eb o1suf
    $ md5sum o2suf
    24bf58c87b04267367217b94a214c4eb o2suf
    ```

## Task 3: Generating Two Executable Files with the Same MD5 Hash
- given a C program, and generate 2 different versions of this program, such that the contents of their `xyz` arrays are different
    ```
    #include <stdio.h>
    unsigned char xyz[200] = {
        /* some contents */
    };
    int main() {
        int i;
        for(i = 0; i < 200; i++) {
            printf("%x", xyz[i]);
        }
        printf("\n");
    }
    ```
- we can fill `{1,2,3,4,5}` to the array, compile the code, name the executable `xyz1`, and use a hex editor to find the location of this array, and directly modify its content, e.g., in the editor, search `01 02 03 04 05`, I modified it to `99 98 97 96 95`, and named the new file `xyz2`
- test whether the new file can execute, and is different from the original one
    ```
    $ ./xyz1
    12345000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
    $ ./xyz2
    9998979695000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
    ```
- now, the code can be divided as the following
    - `[ A || content of array xyz || B ]`
    - based on two facts, we can generate 2 executables with the same MD5
        1. the program `md5collgen` (which generates 2 files with the same prefix and the same MD5)
        2. for any input `T`, `MD5(M)=MD5(N) => MD5(M||T)=MD5(N||T)`
- to make it easier to find the end of the array, I put 200 `7` in this array
- steps
    1. use part `A` as a prefix, generate 2 files with the same MD5 using `md5collgen`, the maximum length of each of the files is `length(A)+127` (to get the maximum, `A`'s length should be multiple `64`s plus `1`, and to make it multiply of `64`, the program will add `63` bytes padding)
        - in my case, the length of part `A` is `12320` bytes, I use it as a prefix, and generated 2 files with the same MD5, each of them is `12480` bytes
        ```
        $ head -c 12320 xyz > prefix
        $ md5collgen -p prefix -o pre1 pre2
        ```
    2. get the suffix, from where the array ends, the suffix is `4464` bytes
        ```
        $ tail -c 4464 xyz > suffix
        ```
    3. to make the total length of the executable the same, there is `40` bytes left
        ```
        $ head -c +12320 xyz > temp
        $ head -c 40 temp > content
        ```
    4. concatenate them into 2 executables
        ```
        $ cat pre1 content suffix > xyz1
        $ cat pre2 content suffix > xyz2
        ```
- test whether they are executables, and have the same MD5
    ```
    $ sudo chmod 777 xyz1
    $ sudo chmod 777 xyz2
    $ ./xyz1
    00000000000000000000000000000000f57d4cd7eeb7e5b282ae3985bd12a632e6d84a5926b78e3258a42ee98c1bfd4714ce2e0841c1f6bfcd9412ee64f5091786ef81cb9d731fcb6761c39c249611ea32a87bba14a9f9e9b8e6c15d3ecc7f6a7c817a246d26c567e850574dce25e776ca85142873668116292f49c5ce784e76a0d1e3d9c96017652decacda0777777777777777777777777777777777777777
    $ ./xyz2
    00000000000000000000000000000000f57d4cd7eeb7e5b282ae3985bd12a632e6d8425926b78e3258a42ee98c1bfd4714ce2e0841c1f6bfcd9412e66505091786ef81cb9d731fcb6f61c39c249611ea32a87bba14a9f9e9b8e6c15d3ecc7f6a74817a246d26c567e850574dce25e776ca85142873668116292749c5ce784e76a0d1e3d9c96017e52decacda0777777777777777777777777777777777777777
    $ diff xyz1 xyz2
    Binary files xyz1 and xyz2 differ
    $ md5sum xyz1
    3944cb69e3b40532d5d426b53d213d2f  xyz1
    $ md5sum xyz2
    3944cb69e3b40532d5d426b53d213d2f  xyz2
    ```

## Task 4: Making the Two Programs Behave Differently
- the basic idea is that we can use 2 arrays `X`, and `Y`, when they are the same, do something benign, otherwise, do something malicious
- structure of 2 programs, prefix is `A`, and we generate 2 files `A || B1` and `A || B2`, `B1` and `B2` are in array `X`, then we find the position of array `Y`, put `B1` somewhere in array `Y`
    ```
    benign      [ A || B1 || C || B1 || D]
    malicious   [ A || B2 || C || B1 || D]
    ```
- the rest is similar to task `3`
    - use content before array `X` as a prefix `A` to generate 2 files, `A`'s size is `12320` bytes, each of the file is `12480` bytes, so `B1` or `B2` is `160` bytes
        ```
        $ head -c 12320 xyz > prefix
        $ md5collgen -p prefix -o pre1 pre2
        ```
    - modify the first `160` bytes of `Y` to `B1`, from the beginning of `Y` to the end of the program is `4640` bytes. So, we need to concatenate `B1` with `40` bytes of `0x07`, and the last `4480` bytes
        ```
        $ tail -c 160 pre1 > b1
        $ tail -c 4480 xyz > rest
        $ cat b1 rest > suffix
        ```
    - get the middle part
        ```
        $ head -c 12544 xyz > temp
        $ tail -c 64 temp > middle
        $ cat pre1 middle suffix > benign
        $ cat pre2 middle suffix > malicious
        ```
- result
    ```
    $ md5sum benign
    6a0fbfa15b1c703c2fbcbdc5177bb478 benign
    $ md5sum malicious
    6a0fbfa15b1c703c2fbcbdc5177bb478 malicious
    $ chmod 777 benign  malicious
    $ ./benign 
    Benign.
    $ ./malicious 
    Malicious!!
    ```
- there are also some other methods, one method is that we use only one array `X`, and we can use the content before array `X` as prefix to generate 2 files, with the same MD5, and we contatenate each file with the rest of our program (ensure the total length is unchanged). In our program, the logic is that if the sum of `X` is even, it executes malicious code, otherwise, it executes benign code. We can check whether in one of the program the sum of `X` is odd, and in the other its even, if not, we invoke `md5collgen` repeatedly until we get what we need
