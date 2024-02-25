# Padding Oracle Attack Lab
- learning an interesting attack on crypto systems. Some systems, when decrypting a given ciphertext, verify whether the padding is valid or not, and throw an error if the padding is invalid. This seemly-harmless behavior enables a type of attack called *padding oracle attack*. Many well-known systems were found vulnerable to this attack, including Ruby on Rails, ASP.NET, and OpenSSL
- in this lab, we are given 2 oracle servers running inside a container. Each oracle has a secret message hidden inside, we can know the ciphertext and the IV, but not the plaintext or the encryption key. We can send a ciphertext (and the IV) to the oracle, it will then decrypt the ciphertext using the encryption key, and tell us whether the padding is valid or not. Our job is to use the response from the oracle to figure out the content of the secret message
- topics
    - Secret-key encryption
    - Encryption modes and paddings
    - Padding oracle attack

## Task 1: Getting Familiar with Padding
- For some block ciphers, when the size of a plaintext is not a multiple of the block size, padding may be required. The *PKCS#5* padding scheme is widely used by many block ciphers. However, *PKCS#5* is only defined for block sizes of 8 bytes. This proves problematic for block ciphers which have block sizes longer than 8 bytes, like AES. To fix this issue, the *PKCS#7* padding scheme was invented. We will conduct the following experiments to understand how this type of padding works
- Create a file: `echo -n "12345" > P`
- Use `openssl enc -aes-128-cbc -e -in P -out C` to encrypt the file. To see what is added to the padding during the encryption, decrypt `C` using `openssl enc -aes-128-cbc -d -in C -out P_dec` with `nopad` option, which will not remove the padding after decryption 
- Actually, I have done this in Secret-Key Encryption lab, the conclusion is that when encrypting a file, it will pad the file size to multiple of block size (16 bytes in this case), if the original file is already multiple of 16 bytes, it will pad a full block. The content of the padding is the length of the padding, e.g., for a 5-byte file, we need to pad 11 bytes, so the padding is 11 bytes of `0x0B`
- Why a full block of padding is added to the 16 bytes file? If there is no padding for file that is already multiple of 16 bytes, what if we have a file with the content (hex) `770F0F0F0F0F0F0F0F0F0F0F0F0F0F0F`? When decrypting the ciphertext for this file, we can't know whether the file it self is 16 bytes, or it is a 1-byte file (`77`) with 15-byte padding (`0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F`)

## Task 2: Padding Oracle Attack (Level 1)
- *The Oracle Setup*
    - use the padding oracle hosted on port 5000
        ```
        $ nc 10.9.0.80 5000
        01020304050607080102030405060708a9b2554b0944118061212098f2f238cd779ea0aae3d9d020f3677bfcb3cda9ce
        ```
    - the oracle accepts input from us, the format of the input is the same as the message above: 16 bytes of the IV, concatenated by the ciphertext. The oracle will decrypt the ciphertext using its own secret key `K`, and the IV provided by us. It will not tell us the plaintext, but it does tell us whether the padding is valid or not. Our task is to use the information provided by the oracle to figure out the actual content of the secret message (for simplicity, we only need to find out one block of the secret message)
    - for debugging, the secret message is here
        ```
        static std::array<unsigned char, 29> PLAIN_TEXT = {
            0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
            0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
            0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
            0xaa, 0xbb, 0xcc, 0xdd, 0xee
        };
        ```
- *Deriving the Plaintext Manually*
    - a skeleton code `manual_attack.py` has been provided, we can construct our attack based on it. The secret has 2 blocks `P1` and `P2`, we only need to get `P2`
    - **Decryption Process** of AES-128-CBC (for 2 blocks)
        - ciphertext: `C1 || C2`
        - use the key `K` and block cipher to decrypte `C1` and `C2` to `D1` and `D2`, respectively
        - `P1 = D1 XOR IV`
        - `P2 = D2 XOR C1`
    - to get `C2`, we need to figure out `P2` by computing `D2 XOR C1`
    - the general idea of the attack is to send to the oracle a ciphertext with modified `C1` (let's call it `CC1`), the server will not tell us the result of `D2 XOR CC1`, but it will tell us whether the result has a valid padding or not, this information allows us to learn information about `D2`, and eventually get all 16 bytes of it. Specifically, let's consider modifying the last byte of `C1`, and get `CC1`. We try all 256 values of this byte, there must be a value that makes the padding valid, and the last byte of the padding is `0x01`, i.e., `CC1[15] XOR D2[15] == 0X01`. One thing we have to notice is that with low probability, there will be multiple values of `CC1[15]` that makes the padding valid (although only one of them makes the last byte of the padding `0x01`)
    - for example, if the second-last byte of the padding is `0x02`, then either `0x02` or `0x01` as the last byte will make this padding valid. To identify which value of `CC1[15]` corresponds to `0x01` in the padding, we can change the `CC1[14]` to another value, then `CC1[14] XOR D2[14]` cannot be `0x02` anymore, and we can get the only 1 value of `CC1[15]` that makes the padding valid, and the last byte is `0x01`
    - once we get the value of `CC1[15]`, we can know `D2[15] = 0x01 XOR CC1[15]`, then we need to modify `CC1[15]` to `D2[15] XOR 0X02`, i.e., make the last byte of the padding `0x02`, the purpose of this is that when we enumerating the value of `CC1[14]`, the only valid padding should have `0x02` in its second-last byte
- *experiment*
    1. in the given code, we start from `CC1` of 16 bytes zeros, only when the last byte is `0xCF`, the padding is valid, so we can get the last byte of the plaintext: `P[15] = D2[15] XOR C1[15] = 0x01 XOR 0xCF XOR C1[15] = 0x03`
        ```
        P[15]  = 0x03
        D2[15] = 0xce
        ```
    2. modify `CC1[15]` to `0x02 XOR D2[15]=0xCC`, enumerate `CC1[14]`, the only value that makes the padding valid is `0x39`, so `P[14] = 0x02 XOR 0x39 XOR C1[14] = 0x03`
        ```
        P[14]  = 0x03
        D2[14] = 0x3B
        ```
    3. modify `CC1[14]` to `0x03 XOR D2[14]=0x38`, modify `CC1[15]` to `0x03 XOR D2[15]=0xCD`, enumerate `CC1[14]`, the only value that makes the padding valid is `0xF2`, so `P[13] = 0x03 XOR 0xF2 XOR C1[13] = 0x03`
        ```
        P[13]  = 0x03
        D2[13] = 0xF1
        ```
    - the rest is similar, doing this manually would be tedious, in the next task, we will automate the attack

## Task 3: Padding Oracle Attack (Level 2)
- this time, we will use port 6000. We need to automate the attack process, and each time we make a new connection to the oracle server at port 6000, the IV and ciphertext will change, it is better get the secret message in one run (we can also get one block in each run, because the secret message is fixed)
- first, try `nc 10.9.0.80 6000` in command line, and we can know that this time, the ciphertext is 48 bytes (3 blocks). The core code for getting a block of the plaintext is shown below:
    ```
    def get_block(IV, C, C_NEXT, oracle):
    P  = bytearray(16)
    D  = bytearray(16)
    CC = bytearray(16)
    # Any value is ok
    for i in range(0, 16):
        D[i]  = 0x00
        CC[i] = 0x77
        
    for K in range(1, 17):
        pad_byte = K
        for i in range(256):
            CC[16 - K] = i
            status = oracle.decrypt(IV + CC + C_NEXT)
            if status == "Valid":
                print("Valid: i = 0x{:02x}".format(i))
                print("CC: " + CC.hex())
                break
        # compute D[16 - K] and P[16 - K]
        D[16 - K] = pad_byte  ^ CC[16 - K]
        P[16 - K] = D[16 - K] ^  C[16 - K]
        print("D: i = 0x{:02x}".format(D[16 - K]))
        print("P: i = 0x{:02x}".format(P[16 - K]))
        
        # adjust CC to make the padding from index (16-K) to 15 become (K+1)
        for i in range(16 - K, 16):
            CC[i] = D[i] ^ (K+1)
    print("P: " + P.hex())
    return P
    ```
- result (since the code containing the secret message for this task is not given, I cannot verify this result. But before I do this task, I have tested my code on port 5000, it can get the message correctly)
    ```
    $ ./auto_attack.py
    ...
    The Plaintext (Padded):
    285e5f5e29285e5f5e29205468652053454544204c616273206172652067726561742120285e5f5e29285e5f5e290202
    ```
