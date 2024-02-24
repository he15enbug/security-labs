# Secret-Key Encryption
- The concepts in the secret-key encryption and some common attacks on encryption. Many common mistakes have been made by developers in using the encryption algorithms and modes. These mistakes weaken the strength of the encryption, and eventually lead to vulnerabilities
- Topics
    - Secret-key encryption
    - Substitution cipher and frequency analysis
    - Encryption modes, IV, and paddings
    - Common mistakes in using encryption algorithms
    - Programming using the crypto library

## Task 1: Frequency Analysis
- monalphabetic substitution cipher is not secure, as it can be subjected to frequency analysis
- find out the original text (an English article) from a cipher-text using frequency analysis
- encryption process
    1. generate the encryption key, i.e., the substitution table. Just permute the alphabet from `a` to `z`
        ```
        #!/bin/env python3
        
        import random
        s = "abcdefghijklmnopqrstuvwxyz"
        list = random.sample(s, len(s))
        key = ''.join(list)
        print(key)
        ```
    2. simplify the original article: convert all upper cases to lowser cases, and then remove all punctuations and numbers. Spaces are kept (in real encryption using monoalphabetic cipher, spaces will be removed)
        ```
        $ tr [:upper:] [:lower:] < article.txt > lowercase.txt
        $ tr -cd '[a-z][\n][:space:]' < lowercase.txt > plaintext.txt
        ```
    3. we use `tr` command to do the encryption. We only encrypt letters, while leaving the space and return characters alone
        ```
        $ tr 'abcdefghijklmnopqrstuvwxyz' <Replaced_String> < plaintext.txt > ciphertext.txt
        ```
- a ciphertext has already been created, our goal is to get the plaintext using frequency analysis
    1. let's look at the most frequent 3-gram (`ytn`), assume they are `the`
        - `$ tr 'ytn' 'THE' < ciphertext.txt > temp.txt`
    2. the top 5 frequent letter in long English text is `etaoi`, while in the ciphertext, its `nyvxu`, beside, in the ciphertext, we can observe words `v`, `vT`, so it's very likely to be `I` or `A`. At the same time, `ing` and `and` are both very frequent 3-grams in English text, which corresponds `vup` or `mur`, in this case, at least we can assume that `u` is `N`
        - `$ tr 'u' 'N' < temp.txt > temp2.txt`
    3. we can assume `v` is `I` or `A`, in the first case, `pmr` is `GAD`; in the second case, `pmr` is `DIG`
        - case 1: `$ tr 'vpmr' 'IGAD' < temp2.txt > temp2-1.txt`
        - case 2: `$ tr 'vpmr' 'ADIG' < temp2.txt > temp2-2.txt`
    4. we can exclude the first case, as in `temp2-1.txt`, I found a single word `ING`, in `temp2-2.txt`, it's `AND`
    5. in `temp2-2.txt`, since we already get `THEANDIG`, we can use them to construct some frequent words, e.g., `THIS`, if we search `THI` in `temp2-2.txt`, we will fingure out that `q` is `S`
        - `$ tr 'q' 'S' < temp2-2.txt > temp3.txt`
    6. the result file is mostly readable, I found some words `SThANGE`, `hIGHT`, `THANsS`, `TIcES`, `Aii`, `AiSx` indicating that `hscix` is `RKMLO`
        - `$ tr 'hscix' 'RKMLO' < temp3.txt > temp4.txt`
    7. finally, by reading `temp4.txt`, we can easily recover the rest of the plaintext: `l->W` (`AlARDS`), `b->F` (`bIRST`, `AbTER`), `d->Y` (`SATISbdING`), `z->U` (`TzRNS`), `a->C` (`OSaARS`), `g->M` (`gE`, `gd`), `e->P` (`eAY`, `eOLITICS`), `j->Q` (`INEjUITY`, `jUIT`), `f->V` (`HAfE`), `k->X` (`EkTRA`), `o->J` (`oUST`), `w->Z` (`PRIwE`)
        - `$ tr 'lbdzagejfkow' 'WFYUCMPQVXJZ' < temp4.txt > temp5.txt`
    8. finally, letter `p` is not found in the result, and we can also find that there is no `B` in `temp5.txt`, so we can know that `p` is `B`

## Task 2: Encryption using Different Ciphers and Modes
- we can use `openssl enc` command to encrypt/decrypt a file (manual: `man openssl`, `man enc`)
    ```
    $ openssl enc -ciphertype -e -in plain.txt -out cipher.bin \
                  -K 00112233445566778889aabbccddeeff \
                  -iv 0102030405060708
    ```
- replace the `ciphertype` with a specific cipher type, such as `-aes-128-cbc`, `-bf-cbc`, `-aes-128-cfb`, etc.

## Task 3: Encryption Mode - ECB vs. CBC
- a sample picture `pic_original.bmp` is given. We will encrypt the picture using *ECB (Electronic Code Book)* and *CBC (Cipher Block Chaining)* modes, and then do the following
    ```
    openssl enc -aes-128-ecb -e -in my_pic.bmp -out ecb.bmp \
                  -K 00112233445566778889aabbccddeeff \
    openssl enc -aes-128-cbc -e -in my_pic.bmp -out cbc.bmp \
                    -K 00112233445566778889aabbccddeeff \
                    -iv 0102030405060708
    ```
    1. treat the encrypted picture as a picture, display it. For `bmp` file, the first 54 bytes contain the header information about the picture, we need to set it correctly (replace the header with the header of the original picture)
        ```
        $ head -c 54 my_pic.bmp > header
        $ tail -c +55 p2.bmp > body
        $ cat header body > new.bmp
        ```
    2. Display the encrypted picture using a picture viewing program (e.g., `eog`)
- *result*: from the picture encrypted using `ecb`, we can still distinguish some pattern of the original picture. While in the `cbc` encrypted picture, we can learn about nothing about the original picture
## Task 4: Padding
- for block ciphers, when the size of a plaintext is not a multiple of the block size, padding may be required. The *PKCS#5* padding scheme is widely used by many block ciphers. We conduct the following experiments to understand how this type of padding works
    1. use ECB, CBC, CFB, and OFB modes to encrypt a file (pick any cipher), which modes have paddings, and which ones do not, explain the result
        - commands
            ```
            # ECB
            echo "Start AES 128 ECB Encryption"
            openssl enc -aes-128-ecb -e -in plain.txt -out ecb.bin \
                            -K 00112233445566778889aabbccddeeff \
            # CBC
            echo "Start AES 128 CBC Encryption"
            openssl enc -aes-128-cbc -e -in plain.txt -out cbc.bin \
                            -K 00112233445566778889aabbccddeeff \
                            -iv 0102030405060708
            # CFB
            echo "Start AES 128 CFB Encryption"
            openssl enc -aes-128-cfb -e -in plain.txt -out cfb.bin \
                            -K 00112233445566778889aabbccddeeff \
                            -iv 0102030405060708
            # OFB
            echo "Start AES 128 OFB Encryption"
            openssl enc -aes-128-ofb -e -in plain.txt -out ofb.bin \
                            -K 00112233445566778889aabbccddeeff \
                            -iv 0102030405060708
            ```
        - after run these commands, we get the following result, we can know that only in CBC and ECB mode have applied padding (the result file is 16 bytes). CBC requires padding because it operates by XORing each plaintext block with the previous ciphertext block before encryption. In ECB mode, padding is not inherently required by the mode, but in practice, padding is often applied
            ```
            $ ./task4.sh 
            $ ls -l
            total 24
            -rw-rw-r-- 1 seed seed  16 Feb 24 07:53 cbc.bin
            -rw-rw-r-- 1 seed seed  13 Feb 24 07:53 cfb.bin
            -rw-rw-r-- 1 seed seed  16 Feb 24 07:53 ecb.bin
            -rw-rw-r-- 1 seed seed  13 Feb 24 07:53 ofb.bin
            -rw-rw-r-- 1 seed seed  13 Feb 24 07:53 plain.txt
            -rwxrwxrwx 1 seed seed 787 Feb 24 07:50 task4.sh
            ```
    2. create 3 files, which contain 5 bytes, 10 bytes, and 16 bytes, respectively. We can use `echo -n "example" > file.txt` to do this. Then, use `openssl enc -aes-128-cbc -e` to encrypt these three files using 128-bit AES with CBC mode, describe the size of the encrypted files. To see what is added to the padding during the encryption, we will decrypt these files using `openssl enc -aes-128-cbc -d`. Unfortunately, decryption by default will automatically remove the padding, the command does have an option called `-nopad`, which will keep the padding during decryption. The result shows that in CBC mode of 128-bit AES, when `file_bytes%16!=0`, it will pad the file to multiple of 16 bytes. When `file_bytes%16==0`, it will pad 16 bytes to the file
        ```
        Size of the Original Files:
        -rw-rw-r-- 1 seed seed  5 Feb 24 08:20 f1.txt
        -rw-rw-r-- 1 seed seed 10 Feb 24 08:20 f2.txt
        -rw-rw-r-- 1 seed seed 16 Feb 24 08:20 f3.txt
        Size of the Encrypted Files:
        -rw-rw-r-- 1 seed seed 16 Feb 24 08:20 f1.bin
        -rw-rw-r-- 1 seed seed 16 Feb 24 08:20 f2.bin
        -rw-rw-r-- 1 seed seed 32 Feb 24 08:20 f3.bin
        ```
        - decrypt these files with `-nopad` option to see what's in the padding. It is obvious that each padded byte represent the total length of the padding, e.g., `f1.txt` is 5 bytes, so the padding is 11 bytes (`0x0B`), so the padding consists of 11 bytes of `0xB0`
            ```
            f1_padded.txt
            30 31 32 33 34 0B 0B 0B 0B 0B 0B 0B 0B 0B 0B 0B
            f2_padded.txt
            30 31 32 33 34 35 36 37 38 39 06 06 06 06 06 06
            f3_padded.txt
            30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10
            ```
## Task 5: Error Propagation - Corrupted Cipher Text

## Task 6: Initial Vector (IV) and Common Mistakes

### Task 6.1: IV Experiment

### Task 6.2: Common Mistake: Use the Same IV

### Task 6.3: Common Mistake: Use a Predictable IV

## Task 7: Programming using the Crypto Library