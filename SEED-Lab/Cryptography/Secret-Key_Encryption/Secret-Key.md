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
- to understand the error propagation property of various encryption modes, we will do the following
    1. create a text file that is at least 1000 bytes
    2. encrypt the file using AES-128 cipher (try ECB, CBC, CFB, and OFB)
    3. unfortunately, a single bit of the 55th byte in the encrypted file got corrupted. (i.e., we manually modify it to another value)
    4. decrypt the corrupted ciphertext file
- how much information we can recover by decrypting the corrupted file
    - in OFB mode, after decrypting the corrupted ciphertext, only the 55th byte is different from the plaintext
    - in ECB and CFB, a block of 16 bytes including the 55th byte are different from the plaintext
    - in CBC, a block of 16 bytes and an extra byte (the 70th byte) are incfluenced
    - for CFB, the influenced block is from the 55th byte to 80th byte. While in CBC and ECB, we first divide the text into blocks of 16 bytes, and the influenced block is the block that the 55th byte is at, in this case, its from the 49th byte to 64th byte
    - for all four modes, the total length of the decrypted text is the same as the plaintext
## Task 6: Initial Vector (IV) and Common Mistakes
- most of encryption modes require an *Initial Vector (IV)*. Properties of an IV depend on the cryptographic scheme used. If we are not careful in selecting IVs, the data encrypted by us may not be secure at all, even though we are using a secure encryption algorithm and mode

### Task 6.1: IV Experiment
- a basic requirement for IV is *uniqueness*, which means that no IV may be reused under the same key
- encrypt the same plaintext using
    1. two different IVs
    2. the same IV
- result: with the same IV, it will generate the same ciphertext

### Task 6.2: Common Mistake: Use the Same IV
- one may argue that if the plaintext does not repeat, using the same IV is safe. Let's look at *Output Feedback (OFB)* mode. Assume that the attacker gets hold of a plaintext `P1` and a ciphertext `C1`, can they decrypt other encrypted messages if the IV is always the same? Given the following information, please try to figure out the actual content of `P2` based on `C2`, `P1`, and `C1`
    ```
    Plaintext  (P1): This is a known message!
    Ciphertext (C1): a469b1c502c1cab966965e50425438e1bb1b5f9037a4c159
    Plaintext  (P2): (unknown)
    Ciphertext (C2): bf73bcd3509299d566c35b5d450337e1bb175f903fafc159
    ```
- first we need to know how OFB works: OFB mode generates a key stream by encrypting the IV using the block cipher algorithm (e.g., AES) with the secret key. The resulting ciphertext is XORed with the plaintext to produce the ciphertext. This process is repeated for each block of plaintext. So, for a given key, if the IV is the same, then the generated key stream is the same, `C1 = P1 XOR KEY_STREAM`, `C2 = P2 XOR KEY_STREAM`, we `XOR` the two formulas, and get `C1 XOR C2 = P1 XOR P2`, i.e., `P2 = C1 XOR C2 XOR P1`
- use the provided sample code
    ```
    #!/usr/bin/python3

    # XOR two bytearrays
    def xor(first, second):
    return bytearray(x^y for x,y in zip(first, second))

    MSG   = "This is a known message!"
    HEX_1 = "a469b1c502c1cab966965e50425438e1bb1b5f9037a4c159"
    HEX_2 = "bf73bcd3509299d566c35b5d450337e1bb175f903fafc159"

    # Convert ascii string to bytearray
    D1 = bytes(MSG, 'utf-8')

    # Convert hex string to bytearray
    D2 = bytearray.fromhex(HEX_1)
    D3 = bytearray.fromhex(HEX_2)

    tmp = xor(D1, D2)
    P2  = xor(tmp, D3)

    print(P2.hex())
    print(P2.decode('utf-8'))
    ```
- result
    ```
    $ ./sample_code.py 
    4f726465723a204c61756e63682061206d697373696c6521
    Order: Launch a missile!
    ```
- the attack used in this experiment is called the *known-plaintext attack*, which is an attack model for cryptanalysis where the attacker has access to both the plaintext and its encrypted version (ciphertext). If this can lead to the revealing of further secret information, the encryption scheme is not considered as secure

- if we replace OFB with CFB, how much of `P2` can be revealed?
    - first, we create `P1` and `P2`, then encrypt them to `C1` and `C2`, respectively
        ```
        echo -n "This is a known message!" > P1
        echo -n "Order: Launch a missile!" > P2

        openssl enc -aes-128-cfb -e -in P1 -out C1 \
                        -K 00112233445566778889aabbccddeeff \
                        -iv 0102030405060708
        openssl enc -aes-128-cfb -e -in P2 -out C2 \
                        -K 00112233445566778889aabbccddeeff \
                        -iv 0102030405060708
        ```
    - `C1`: `D3EEE656E156C9F1CEBED4731A67324DC90CDB5E85222AD4`
    - `C2`: `C8F4EB40B3059A9DCEEBD17E1D303D4D59AB113CA9F2B307`
    - compare the result of `P1 XOR C1 XOR C2` with the result in OFB, we can only recover the first block of the data (16 bytes), the reason is simple:
        1. in both modes, each processing a block, there will be 3 inputs: a vector, a key, and plaintext. For the first block, the vector is IV
        2. however, for later blocks, the vector that OFB uses is generated from previous vector and the key, which means the new vector is not related to the ciphertext, this is the key that we can use `P1 XOR C2 XOR C2` to get `P2`
        3. in CFB, the vector is generated from not only the key and previous vector, but also the ciphertext of the previous block, so, for different plaintext, the key stream after the first block is different, so `P1 XOR C2 XOR C2` will not be able to get the correct plaintext after the first block (unless the first block of `P1` and `P2` are the same, in that case, we can recover the second block)
        ```
        4f726465723a204c 61756e6368206120 fdc2b9114db7fcf2 <-- result in CFB
        4f726465723a204c 61756e6368206120 6d697373696c6521 <-- result in OFB
        ```
### Task 6.3: Common Mistake: Use a Predictable IV
- another requirement on IV is that IVs need to be unpredictable for many schemes, i.e., IVs need to be randomly generated
- assume that Bob just sent out an encrypted message, and Eve knows what its content is either `Yes` or `No`. Eve can see the ciphertext and the IV used to encrypt the message, but since the encryption algorithm AES is quite strong, Eve has no idea what the actual content is. However, since Bob uses predictable IVs, Eve knows exactly what IV Bob is going to use next
- a good cipher should not only tolerate the known-plaintext attack, it should also tolerate the *chosen-plaintext attack*, which is an attack model for cryptanalysis where the attacker can obtain the ciphertext for an arbitrary plaintext. Since AES is a strong cipher that can tolerate the chosen-plaintext attack, Bob does not mind encrypting any plaintext given by Eve, he does use a different IV for each plaintext, unfortunately, the IVs he generates are not random, and they can always be predictable
- *task*: construct a message and ask Bob to encrypt it and get the ciphertext. Then use this opportunity to figure out whether an actual content of Bob's secret message is `Yes` or `No`. An encryption oracle is given, which simulates Bob and encrypt message with 128-bit AES with CBC mode
- first, we pad `Yes` and `No` to 16 bytes
    ```
    5965730D0D0D0D0D0D0D0D0D0D0D0D0D <-- Yes
    4e6f0E0E0E0E0E0E0E0E0E0E0E0E0E0E <-- No
    ```
- our padded data is only 16 bytes, so we can only consider the first block. In CBC mode, it will first calculate `P XOR IV`, then use the key and chosen cipher to encrypt the result, and get the ciphertext. Now, we know the IV for a message (`Yes` or `No`), denote the IV as `IV0`, and its ciphertext `C0`, and we can predict the next IV, what we can do is to construct the result of `P XOR IV`, make it the same as `PAD("Yes") XOR IV0` or `PAD("No") XOR IV0`, and compare the ciphertext to `C0`, then we can know what the orginal message is
- steps
    1. calculate `PAD("Yes") XOR IV0`, and `PAD("No") XOR IV0`
        ```
        C0:  35a8e7c8d7515ee8a01f19693c617072
        IV0: 1700d2a48d1e1138da3112d44fda20a6
        PAD("Yes"): 5965730D0D0D0D0D0D0D0D0D0D0D0D0D
        PAD("No"):  4e6f0E0E0E0E0E0E0E0E0E0E0E0E0E0E
        PAD("Yes") XOR IV0: 4e65a1a980131c35d73c1fd942d72dab
        PAD("No") XOR IV0:  596fdcaa83101f36d43f1cda41d42ea8
        ```
    2. the next IV is `IV1=e9d563218e1e1138da3112d44fda20a6`, we need to provide a plaintext `P1`, such that `P1 XOR IV1 == 4e65a1a980131c35d73c1fd942d72dab`, i.e., `P1 = IV1 XOR 4e65a1a980131c35d73c1fd942d72dab`
        - input `P1: a7b0c2880e0d0d0d0d0d0d0d0d0d0d0d` to the oracle, get the ciphertext is `35a8e7c8d7515ee8a01f19693c617072099a133d8089a84005deffb5923e437d`, we can ignore the second block, as `P1` is 16 bytes, CBC added another 16 bytes padding, the second block of the ciphertext is for the padding only. We can find that the first block of the ciphertext is the same as the ciphertext of `Yes`, so, the original message is `Yes`
    3. I tried `Yes` first, and fortunately got the correct result. If in the previous step, the first block of the cipher text is not the same as `C0=35a8e7c8d7515ee8a01f19693c617072`, we need to use the next IV `IV2=431c545e8e1e1138da3112d44fda20a6`, calculate `P2 = IV2 XOR 596fdcaa83101f36d43f1cda41d42ea8`, and input `P2` to the oracle

## Task 7: Programming using the Crypto Library
