# RSA Public-Key Encryption and Signature Lab
- *RSA (Rivest-Shamir-Adleman)* is one of the first public-key cryptosystems and it widely used for secure communication. The RSA algorithm first generates two large random prime numbers, and then use them to generate public and private key pairs, which can be used to do encryption, decryption, digital signature generation, and digital signature verification. The RSA algorithm is built upon number theories
- topics
    - public-key cryptography
    - the RSA algorithm and key generation
    - big number calculation
    - encryption and decryption using RSA
    - digital signature
    - X.509 certificate
## Background
- the RSA algorithm involves computations on large numbers. These computations cannot be directly conducted using simple arithmetic operators in programs, because thos operators can only operate on primitive data types, such as 32-bit and 64-bit integers. The numbers involved in the RSA algorithms are typically more than 512 bits long. There are serveral libraries that can perform arithmetic operations on integers of arbitrary size. We will use the Big Number library provided by `openssl`. To use this library, we will define each big number as a `BIGNUM` type, and use the APIs provided by the library for various operations, such as addition, multiplication, exponentiation, modular operations, etc
### BIGNUM APIs
- All the big number APIs can be found [here](https://linux.die.net/man/3/bn)
- needed for this lab
    - some of the library functions requires temporary variables. Since dynamic memory allocation to create BIGNUMs is quite expensive when used in conjunction with repeated subroutine calls, a BN_CTX structure is created to holds BIGNUM temporary variables used by library function, we need to create such a structure, and pass it to the functions that require it
    - `BN_CTX *ctx = BN_CTX_new()`
    - initialize a BIGNUM variable: `BIGNUM *a = BN_new()`
    - there are a number of ways to assign a value to a BIGNUM variable
        ```
        # From decimal number string
        BN_dec2bn(&a, "12345678912415454153456");
        # From hex number string
        BN_hex2bn(&a, "21351AB2C245F24AED3DDFA");
        # Random number of 128 bits
        BN_rand(a, 128, 0, 0)
        # Random prime number of 128 bits
        BN_generate_prime_ex(a, 128, 1, NULL, NULL, NULL);
        ```
    - print out a big number
        ```
        void printBN(char *msg, BIGNUM *a) {
            // Convert BIGNUM to number string
            char *number_str = BN_bn2dec(a);
            printf("%s %s\n", msg, number_str);
            // Free the dynamically allocated memory
            OPENSSL_free(number_str);
        }
        ```
    - operations
        ```
        BN_sub(res, a, b); // res = a - b
        BN_add(res, a, b); // res = a + b
        BN_mul(res, a, b, ctx); // res = a * b, a BN_CTX is needed
        BN_mod_mul(res, a, b, n, ctx); // res = (a * b) % n 
        BN_mod_exp(res, a, c, n, ctx); // res = a^c % n
        
        // modular inverse, given a, find b, such that a * b mod n = 1
        // the value b is called the inverse of a, with respect to modular n
        BN_mod_inverse(b, a, n, ctx);
        ```
- *compilation*: use `-lcrypto` option to tell the compiler to use `crypto` library

## Task 1: Deriving the Private Key
- let `p`, `q`, and `e` be 3 prime numbers, their values are listed below, let `n=p*q`, we will use `(e, n)` as the public key. Please calculate the private key `d`. Note that although `p` and `q` in this task are quite large number, they are not large enough to be secure, in practice, these numbers should be at least 512 bits long
    ```
    p = F7E75FDC469067FFDC4E847C51F452DF
    q = E85CED54AF57E53E092113E62F436F4F
    e = 0D88C3
    ```
- the private key is `d = modular_inverse(e, (p-1)*(q-1))`
- result
    ```
    $ gcc gen_private.c -o gen_private -lcrypto
    $ ./gen_private 
    private key =  3587A24598E5F2A21DB007D89D18CC50ABA5075BA19A33890FE7C28A9B496AEB
    ```
## Task 2: Encrypting a Message
- Let `(e, n)` be the public key, encrypt the message `A top secret!`, we need to convert this ASCII string to a hex string, and then convert the hex string to a BIGNUM
- use Python command to convert it to hex string `print('A top secret!'.encode('utf-8').hex())`
    - result: `4120746f702073656372657421`
- RSA parameters in hex
    ```
    n = DCBFFE3E51F62E09CE7032E2677A78946A849DC4CDDE3A4D0CB81629242FB1A5
    e = 010001
    M = 4120746f702073656372657421
    d = 74D806F9F3A62BAE331FFE3F0A68AFE35B3D2E4794148AACBC26AA381CD7D30D
    ```
- the ciphertext is `C = msg^e mod n`
- result
    ```
    $ ./encrypt 
    ciphertext for "4120746f702073656372657421": 6FB078DA550B2650832661E14F4F8D2CFAEF475A0DF3A75CACDC5DE5CFC5FADC
    ```

## Task 3: Decrypting a Message
- use the same key pair in task `2`, decrypt ciphertext `C`
    - `C = 8C0F971DF2F3672B28811407E2DABBE1DA0FEBBBDFC7DCB67396567EA1E2493F`
- to convert hex string to ASCII string, use `bytes.fromhex(hex_string).decode('utf-8')`
- the plaintext is `P = C^d mod n`
- result
    ```
    $ ./decrypt
    plaintext for "8C0F971DF2F3672B28811407E2DABBE1DA0FEBBBDFC7DCB67396567EA1E2493F": 50617373776F72642069732064656573
    ```
    ```
    >>> print(bytes.fromhex('50617373776F72642069732064656573').decode('utf-8'))
    Password is dees
    ```

## Task 4: Signing a Message
- only the owner of the private key can sign a message, and anyone can verify it using public key
- the signature is `S = M^d mod n` (in this task, we sign the original message, instead of its hash)
- sign `M = I owe you $2000` (hex string `49206f776520796f75202432303030`)
    - result: `80A55421D72345AC199836F60D51DC9594E2BDB4AE20C804823FB71660DE7B82`
- sign `M = I owe you $3000` (hex string `49206f776520796f75202433303030`)
    - result: `04FC9C53ED7BBE4ED4BE2C24B0BDF7184B96290B4ED4E3959F58E94B1ECEA2EB`
- the result suggests that even if there is only a small difference between the original messages, the signature will be significantly different
## Task 5: Verifying a Signature
- public key (hex)
    ```
    e = 010001
    n = AE1CD4DC432798D933779FBD46C6E1247F0CF1233595113AA51B450F18116115
    ```
- signature `643D6F34902D9C7EC90CB0B2BCA36C47FA37165C0005CAB026C0542CBDB6802F`
- claimed message `Launch a missile.` (`4C61756E63682061206D697373696C652E`)
- compute the original message `M = S^e mod n`, and compare it to the given message, we can convert the given message to `BIGNUM`, and use `BN_cmp(ori_msg, msg)` to compare them, it returns `0` if the 2 numbers are equal
- result
    ```
    $ ./sign_verify 
    ... (information for task 4 is omitted)
    original message being signed: 4C61756E63682061206D697373696C652E
    This is a valid message
    ```
## Task 6: Manually Verifying an X.509 Certificate
- manually verify an X.509 certificate using our program. An X.509 contains data about a public key and an issuer's signature on the data
### Step 1: Download a certificate from a real web server
- use `www.ed.ac.uk`
    ```
    $ openssl s_client -connect www.ed.ac.uk:443 -showcerts
    Certificate chain
     0 s:CN = www.ed.ac.uk
       i:C = US, O = Let's Encrypt, CN = R3
    -----BEGIN CERTIFICATE-----
    ...
    -----END CERTIFICATE-----
     1 s:C = US, O = Let's Encrypt, CN = R3
       i:C = US, O = Internet Security Research Group, CN = ISRG Root X1
    -----BEGIN CERTIFICATE-----
    ...
    -----END CERTIFICATE-----
     2 s:C = US, O = Internet Security Research Group, CN = ISRG Root X1
       i:O = Digital Signature Trust Co., CN = DST Root CA X3
    -----BEGIN CERTIFICATE-----
    ...
    -----END CERTIFICATE-----
    ```
- there are 3 certificates, the third certificate's subject is the second's issuer, and the second certificate's subject is the first certificate's issuer, there is a certificate chain, we can start from the server's certificate, and verify each certificate with its issuer, and finally, we will verify the last certificate with its issuer, which is a root CA, and its certificate can be found in the browser or in `/etc/ssl/certs`
- save these 3 certificates (from `BEGIN` to `END`, including these 2 lines) in file `c0.pem`, `c1.pem`, and `c2.pem`, respectively. 
    ```
    c0.pem
        subject:CN = www.ed.ac.uk
        issuer:C = US, O = Let's Encrypt, CN = R3
    c1.pem
        subject:C = US, O = Let's Encrypt, CN = R3
        issuer:C = US, O = Internet Security Research Group, CN = ISRG Root X1
    c2.pem
        subject:US, O = Internet Security Research Group, CN = ISRG Root X1
        issuer:O = Digital Signature Trust Co., CN = DST Root CA X3
    ```
### Step 2: Extract the public key `(e, n)` from the issuer's certificate
- extract the value of `n` using `-modulus`
- no specific command to extract `e`, but we will be able to find it by printing all fields
- `c0.pem`
    ```
    $ openssl x509 -in c0.pem -noout -modulus
    Modulus=C074A77A6BF4C19CDC5C080C901417CEDC2334716B1E922CE5AC6BCF3ADD970ED3F8664040B3E252F499985DD2711435362EFB33BC2FBE3F162A0126EA49A4206B2A12BD4DC153BFF4CF9B7CBE1E3F2D72C15F3B5B1ED939554AE0958932A72D3AD599CA7273DE35FAECD75AD3EF41FBEE3F3784C068BA549F1BBCDE631A4CBFA5FFCC988A7D2222DC7167761132F2FCA0C3BD056A858A08424ACCDF22946E7C61E3A447A874965EEDA2E2EC371E3EFC325D95A2FDA97561078274592BA6472093C9B284FFC03F34239376DD2809702900DDBD04786376FE6CC25589B0D656B56A8CBCFEC9BA5A00988E8CA3A6A7D3430BA2D93C9BA356C13E9AF611F32D9955
    $ openssl x509 -in c0.pem -text | grep Exponent
    Exponent: 65537 (0x10001)
    ```
- `c1.pem`
    ```
    $ openssl x509 -in c1.pem -noout -modulus
    Modulus=BB021528CCF6A094D30F12EC8D5592C3F882F199A67A4288A75D26AAB52BB9C54CB1AF8E6BF975C8A3D70F4794145535578C9EA8A23919F5823C42A94E6EF53BC32EDB8DC0B05CF35938E7EDCF69F05A0B1BBEC094242587FA3771B313E71CACE19BEFDBE43B45524596A9C153CE34C852EEB5AEED8FDE6070E2A554ABB66D0E97A540346B2BD3BC66EB66347CFA6B8B8F572999F830175DBA726FFB81C5ADD286583D17C7E709BBF12BF786DCC1DA715DD446E3CCAD25C188BC60677566B3F118F7A25CE653FF3A88B647A5FF1318EA9809773F9D53F9CF01E5F5A6701714AF63A4FF99B3939DDC53A706FE48851DA169AE2575BB13CC5203F5ED51A18BDB15
    $ openssl x509 -in c1.pem -text | grep Exponent
    Exponent: 65537 (0x10001)
    ```
- `c2.pem`
    ```
    $ openssl x509 -in c2.pem -noout -modulus
    Modulus=ADE82473F41437F39B9E2B57281C87BEDCB7DF38908C6E3CE657A078F775C2A2FEF56A6EF6004F28DBDE68866C4493B6B163FD14126BBF1FD2EA319B217ED1333CBA48F5DD79DFB3B8FF12F1219A4BC18A8671694A66666C8F7E3C70BFAD292206F3E4C0E680AEE24B8FB7997E94039FD347977C99482353E838AE4F0A6F832ED149578C8074B6DA2FD0388D7B0370211B75F2303CFA8FAEDDDA63ABEB164FC28E114B7ECF0BE8FFB5772EF4B27B4AE04C12250C708D0329A0E15324EC13D9EE19BF10B34A8C3F89A36151DEAC870794F46371EC2EE26F5B9881E1895C34796C76EF3B906279E6DBA49A2F26C5D010E10EDED9108E16FBB7F7A8F7C7E50207988F360895E7E237960D36759EFB0E72B11D9BBC03F94905D881DD05B42AD641E9AC0176950A0FD8DFD5BD121F352F28176CD298C1A80964776E4737BACEAC595E689D7F72D689C50641293E593EDD26F524C911A75AA34C401F46A199B5A73A516E863B9E7D72A712057859ED3E5178150B038F8DD02F05B23E7B4A1C4B730512FCC6EAE050137C439374B3CA74E78E1F0108D030D45B7136B407BAC130305C48B7823B98A67D608AA2A32982CCBABD83041BA2830341A1D605F11BC2B6F0A87C863B46A8482A88DC769A76BF1F6AA53D198FEB38F364DEC82B0D0A28FFF7DBE21542D422D0275DE179FE18E77088AD4EE6D98B3AC6DD27516EFFBC64F533434F
    $ openssl x509 -in c2.pem -text | grep Exponent
    Exponent: 65537 (0x10001)
    ```
### Step 3: Extract the signature from the server's certificate
- just print out all fields an find it
    ```
    $ openssl x509 -in c2.pem -text
    ...
    Signature Algorithm: sha256WithRSAEncryption
        3a:e5:52:eb:9e:8a:66:f3:61:ef:54:e9:27:72:3b:2a:93:e2:
        b3:97:ef:46:97:0b:f6:30:cc:68:09:20:0f:b6:a5:a8:0a:1c:
        a0:81:d1:a0:bb:67:30:0b:75:b4:cc:a8:b2:b5:c4:b5:b0:0a:
        e4:84:d6:bf:89:4b:2d:8e:2c:11:8a:82:4a:b9:07:f1:65:cb:
        8b:8f:14:19:1c:59:12:ae:89:9f:b9:23:ff:2f:bd:b4:30:68:
        25:9e:3f:a3:9e:04:e2:bf:a5:69:2f:08:d6:a1:30:26:43:6b:
        13:f2:96:04:80:35:2a:8c:97:e9:64:a3:1b:49:08:51:ee:64:
        f6:a7:ed:0e:9e:d2:35:33:23:6d:d1:c0:2c:f5:e6:88:cf:4f:
        c0:3f:73:75:18:be:87:38:d8:fb:3f:48:82:e8:f8:61:d1:6f:
        2a:03:7d:3e:d1:b0:c2:e1:bc:83:4c:6e:17:36:28:51:03:df:
        da:99:29:d8:22:68:27:b3:90:5f:e8:17:4f:bb:13:55:ec:fc:
        ca:5b:27:7c:8f:8c:bd:e4:e9:f4:6f:c7:65:74:a1:22:ab:3a:
        39:4e:74:9a:5d:b8:c2:ce:b9:db:dc:cd:ca:04:13:be:98:54:
        f6:6e:61:ff:5b:e2:32:da:49:4d:48:ad:50:02:20:b2:56:17:
        b6:6e:57:82
    ```
- save this in a file `server_signature`, and remove all spaces and `:`
    ```
    $ cat server_signature | tr -d '[:space:]:'
    3ae552eb9e8a66f361ef54e927723b2a93e2b397ef46970bf630cc6809200fb6a5a80a1ca081d1a0bb67300b75b4cca8b2b5c4b5b00ae484d6bf894b2d8e2c118a824ab907f165cb8b8f14191c5912ae899fb923ff2fbdb43068259e3fa39e04e2bfa5692f08d6a13026436b13f2960480352a8c97e964a31b490851ee64f6a7ed0e9ed23533236dd1c02cf5e688cf4fc03f737518be8738d8fb3f4882e8f861d16f2a037d3ed1b0c2e1bc834c6e1736285103dfda9929d8226827b3905fe8174fbb1355ecfcca5b277c8f8cbde4e9f46fc76574a122ab3a394e749a5db8c2ceb9dbdccdca0413be9854f66e61ff5be232da494d48ad500220b25617b66e5782
    ```
- save the result in `server_signature`
### Step 4: Extract the body of the server's certificate
- A CA generates the signature for a server certificate by first computing the hash of the certificate, and then sign the hash. To verify the signature, we also need to generate the hash from a certificate. Since the hash is generated before the signature is computerd, we need to exclude the signature block of a certificate when computing the hash
- X.509 certificates are encoded using the ASN.1 (Abstract Syntax Notation.One) standard, so if we can parse the ASN.1 structure, we can extract any field from a certificate. Use `asn1parse` to extract data from ASN.1 formatted data
    ```
    $ openssl asn1parse -i -in c0.pem
      0:d=0  hl=4 l=1254 cons: SEQUENCE 
      4:d=1  hl=4 l= 974 cons:  SEQUENCE  <--- BEGIN certificate body
      8:d=2  hl=2 l=   3 cons:   cont [ 0 ]
     10:d=3  hl=2 l=   1 prim:    INTEGER           :02
     13:d=2  hl=2 l=  18 prim:   INTEGER           :04C11E2F94A47B36D5833150FFF389E37A8F
     33:d=2  hl=2 l=  13 cons:   SEQUENCE
    ...
    982:d=1  hl=2 l=  13 cons:  SEQUENCE  <--- BEGIN signature block
    984:d=2  hl=2 l=   9 prim:   OBJECT            :sha256WithRSAEncryption
    ...
    ```
- so the certificate body is from offset `4` to `981`
- use `-strparse` option to get the field from offset `4`, which will give use the body of the certificate, excluding the signature block
    ```
    $ openssl asn1parse -i -in c0.pem -strparse 4 -out c0_body.bin -noout
    ```
- the hash
    ```
    $ sha256sum c0_body.bin 
    06f31cb82f47856dd892091ce5e34bb7680e07ccf8f9ca5138ee782b2cddc784  c0_body.bin
    ```
### Step 5: Verify the signature
- to verify the signature, we need
    1. the signature
        ```
        3ae552eb9e8a66f361ef54e927723b2a93e2b397ef46970bf630cc6809200fb6a5a80a1ca081d1a0bb67300b75b4cca8b2b5c4b5b00ae484d6bf894b2d8e2c118a824ab907f165cb8b8f14191c5912ae899fb923ff2fbdb43068259e3fa39e04e2bfa5692f08d6a13026436b13f2960480352a8c97e964a31b490851ee64f6a7ed0e9ed23533236dd1c02cf5e688cf4fc03f737518be8738d8fb3f4882e8f861d16f2a037d3ed1b0c2e1bc834c6e1736285103dfda9929d8226827b3905fe8174fbb1355ecfcca5b277c8f8cbde4e9f46fc76574a122ab3a394e749a5db8c2ceb9dbdccdca0413be9854f66e61ff5be232da494d48ad500220b25617b66e5782
        ```
    2. the hash of the server's certificate
        ```
        06f31cb82f47856dd892091ce5e34bb7680e07ccf8f9ca5138ee782b2cddc784
        ```
    3. the CA's public key `(e, n)`, which are extract from `c1.pem`
        ```
        n=BB021528CCF6A094D30F12EC8D5592C3F882F199A67A4288A75D26AAB52BB9C54CB1AF8E6BF975C8A3D70F4794145535578C9EA8A23919F5823C42A94E6EF53BC32EDB8DC0B05CF35938E7EDCF69F05A0B1BBEC094242587FA3771B313E71CACE19BEFDBE43B45524596A9C153CE34C852EEB5AEED8FDE6070E2A554ABB66D0E97A540346B2BD3BC66EB66347CFA6B8B8F572999F830175DBA726FFB81C5ADD286583D17C7E709BBF12BF786DCC1DA715DD446E3CCAD25C188BC60677566B3F118F7A25CE653FF3A88B647A5FF1318EA9809773F9D53F9CF01E5F5A6701714AF63A4FF99B3939DDC53A706FE48851DA169AE2575BB13CC5203F5ED51A18BDB15
        e=10001
        ```
- result, we can see that the message recovered from the signature is much longer (255 bytes) than the hash value (32 bytes), and the lowest 32 bytes of the message is the same as the hash value. This is because before signing a message, PKCS#1 v1.5 padding will be applied, which will get a result with the first 2 bytes `0x0001`, since the rest of the padding can consists of any non-zero bytes (can be random), we just need to ensure that the last 32 bytes are the same as the hash value we computed, and the first 2 bytes are `0x00` and `0x01`. Based on this, the following verification shows that the certificate is valid
    ```
    $ ./verify_X509 
    **********Message Being Signed**********
    01FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF003031300D06096086480165030402010500042006F31CB82F47856DD892091CE5E34BB7680E07CCF8F9CA5138EE782B2CDDC784
    **********Hash of the Server's Certificate**********
    06F31CB82F47856DD892091CE5E34BB7680E07CCF8F9CA5138EE782B2CDDC784
    ```
- since the certificate's issuer is not a root CA, and the certificate for this CA is provided by the server, in practice, we need to follow the certificate chain to verify each certificate, until we get root CA
