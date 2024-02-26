# Hash Length Extension
- when a client and a server communicate over the internet, they are subject to MITM attacks. An attacker can intercept the request, modify it, and send the modified request to the server. In such a scenario, the server needs to verify the integrity of the request received. The standard way to verify the integrity of the request is to attach a tag called *Message Authentication Code, MAC* to the request. There are many ways to calculate MAC, and some of the methods are not secure
- MAC is calculated from a secret key and a message, a naive way to calculate MAC is to concatenate the key with the message and calculate the one way hash of the resulting string. This method is subject to an attack called length extension attack, which allows attackers to modify the message while still be able to generate a valid MAC based on the modified message, without knowing the secret key
- *lab setup*: add an entry to `/etc/hosts`: `10.9.0.80 www.seedlab-hashlen.com`

## Task 1: Send Request to List Files
- send a benign request: `http://www.seedlab-hashlen.com?myname=he15enbug&uid=<?>&lstcmd=1&mac=<?>`
- we need to pick a `uid` and use its `key` to compute the MAC, e.g., choose `1002:983abe`
    ```
    1001:123456
    1002:983abe
    1003:793zye
    1004:88zjxc
    1005:xciujk
    ```
- to compute the MAC, concatenate the parameter part `R` of the URL with the key, `-n` option in `echo` command is used to output only the string, no newline character
    ```
    // Key:R = 983abe:myname=he15enbug&uid=1002&lstcmd=1

    $ echo -n "983abe:myname=he15enbug&uid=1002&lstcmd=1" | sha256sum
    f45212a2c35ae17687d00d0bd4f0c248eaaabb1893399da1a12683b29779b7ed  -
    ```
- request URL: `http://www.seedlab-hashlen.com?myname=he15enbug&uid=1002&lstcmd=1&mac=f45212a2c35ae17687d00d0bd4f0c248eaaabb1893399da1a12683b29779b7ed`, we can get a message on the page: `Yes, your MAC is valid`

## Task 2: Create Padding
- we need to understand how padding is calculated for one-way hash. The block size of SHA-256 is 64 bytes, so a message `M` will be padded to the multiple of 64 bytes during the hash calculation. According to *RFC 6234*, paddings for SHA-256 consist of one byte of `\x80`, followed by many `0`s, followed by a 64-bit (8 bytes) length field, the length is the number of **bits** in the `M`

- *RFC 6234* detail: no matter how long the original message is, it will first append a byte `0x80` to the message. Then, if the current length (in byte) is not congruent to `56`, it will keep padding `0x00` until the length modulo `64` is `56`. Finally, it will add an 8-byte big-endian number representing the length of the original message

- example: the original message is `M = "This is a test message"` (22 bytes), the padding is `64 - 22 = 42` bytes, including 8 bytes of the length field, its value is `22 * 8 = 0xB0`, so the padded message fed into SHA-256 is
    ```
    "This is a test message"
    "\x80"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\xB0"
    ```
- *important*: the length field uses the *Big-Endian* byte order
- *task*: construct the padding for `983abe:myname=he15enbug&uid=1002&lstcmd=1`, I wrote a C program to perform padding for a give string
    ```
    $ gcc -o sha256_pad sha256_pad.c
    $ ./sha256_pad
    Padded message:
    \x39\x38\x33\x61\x62\x65\x3a\x6d\x79\x6e\x61\x6d\x65\x3d\x68\x65\x31\x35\x65\x6e\x62\x75\x67\x26\x75\x69\x64\x3d\x31\x30\x30\x32\x26\x6c\x73\x74\x63\x6d\x64\x3d\x31\x80\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x48
    Padded message (URL encoding):
    %39%38%33%61%62%65%3a%6d%79%6e%61%6d%65%3d%68%65%31%35%65%6e%62%75%67%26%75%69%64%3d%31%30%30%32%26%6c%73%74%63%6d%64%3d%31%80%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%01%48
    ```

## Task 3: The Length Extension Attack
- we will generate a valid MAC for a URL without knowing the MAC key. Assume that we know the MAC of a valid request `R`, and we also know the size of the MAC key. Our job is to forge a new request based on `R`, while still being able to compute the valid MAC
- given the original message `This is a test message` and its MAC value, we will show how to add a message `Extra message` to the end of the padded `M`
    ```
    $ echo -n "This is a test message" | sha256sum
    6f3438001129a90c5b1637928bf38bf26e39e57c6e9511005682048bedbef906  -
    ```
- the program can be used to compute the MAC for the new message
    ```
    #include <stdio.h>
    #include <arpa/inet.h>
    #include <openssl/sha.h>

    int main(int argc, const char *argv[]) {
        int i;
        unsigned char buffer[SHA256_DIGEST_LENGTH];
        SHA256_CTX c;

        SHA256_Init(&c);
        for(i = 0; i < 64; i++) {
            SHA256_Update(&c, "*", 1);
        }

        // MAC of the original message M (padded)
        c.h[0] = htole32(0x6f343800);
        c.h[1] = htole32(0x1129a90c);
        c.h[2] = htole32(0x5b163792);
        c.h[3] = htole32(0x8bf38bf2);
        c.h[4] = htole32(0x6e39e57c);
        c.h[5] = htole32(0x6e951100);
        c.h[6] = htole32(0x5682048b);
        c.h[7] = htole32(0xedbef906);

        // Append additional message
        SHA256_Update(&c, "Extra message", 13);
        SHA256_Final(buffer, &c);

        for(i = 0; i < 32; i++) {
            printf("%02x", buffer[i]);
        }
        printf("\n");
        return 0;
    }
    ```
- compilation `gcc -o ext length_ext.c -lcrypto`, the result of the program is `eb71f88b08909fa9fe582c994a6f620b739045287104bf44fad9a2d0e28d6bf3`
- think about how the hash algorithm works, it will first add padding to the original data, make its size the multiple of 64 bytes, then it will start from the first 64-byte block, compute an output, and fed into the next block, with the next block of data. When we get the hash value of `This is a test message`, we can continue the computation, extend a block and get the hash of `This is a test message<PADDING>Extra message` (there will also be padding after `Extra message`), to verify this, we can use the following command, and we can get the same result:
    ```
    (IMPORTANT: no spaces or lines at the beginning or end of each line)
    $ printf "This is a test message"\
    > "\x80"\
    > "\x00\x00\x00\x00\x00\x00\x00\x00"\
    > "\x00\x00\x00\x00\x00\x00\x00\x00"\
    > "\x00\x00\x00\x00\x00\x00\x00\x00"\
    > "\x00\x00\x00\x00\x00\x00\x00\x00"\
    > "\x00\x00\x00\x00\x00\x00\x00\x00"\
    > "\xB0"\
    > "Extra message" | sha256sum
    eb71f88b08909fa9fe582c994a6f620b739045287104bf44fad9a2d0e28d6bf3  -
    ```
- *task*
    - generate a valid MAC for the following request
        - `http://www.seedlab-hashlen.com?myname=he15enbug&uid=1002&lstcmd=1&mac=<mac>`
        - the MAC is
            ```
            $ printf "983abe:myname=he15enbug&uid=1002&lstcmd=1" | sha256sum
            f45212a2c35ae17687d00d0bd4f0c248eaaabb1893399da1a12683b29779b7ed  -
            ```
    - based on this MAC, construct a new MAC value for this request
        - `http://www.seedlab-hashlen.com?myname=he15enbug&uid=1002&lstcmd=1&download=secret.txt&mac=<new-mac>`
    
- *solution*
    - based on previous experiment, we know that after getting the hash of `<KEY>:content<PADDING>`, we can extend arbitrary content and get the hash of `<KEY>:<CONTENT><PADDING><EXTRA_CONTENT><PADDING>`
    - first, we can use the C program, extend `f45212a2c35ae17687d00d0bd4f0c248eaaabb1893399da1a12683b29779b7ed` with extra data `&download=secret.txt` (20 bytes)
        ```
        $ ./ext
        db6f423fe3f2537cba8555e940c559cd388a3cf8a5ad4fcf0be8ea067e1a6880
        ```
    - this MAC is for the following message (the padding after `&download=secret.txt` is automatically added by the hash algorithm)
        ```
        983abe:myname=he15enbug&uid=1002&lstcmd=1<PADDING>&download=secret.txt
        ```
    - in real attack, attacker doesn't need to know the key, as long as the attacker knows the length of the data `983abe:myname=he15enbug&uid=1002&lstcmd=1`, they can figure out the padding (URL encoded) `%80%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%01%48`
    - let's verify this (this step is only for us to verify that we get the correct results so far)
        ```
        (IMPORTANT: no spaces or lines at the beginning or end of each line)
        $ printf \
        > "983abe:myname=he15enbug&uid=1002&lstcmd=1"\
        > "\x80"\
        > "\x00\x00\x00\x00\x00\x00\x00\x00"\
        > "\x00\x00\x00\x00\x00\x00\x00\x00"\
        > "\x00\x00\x00\x00"\
        > "\x01\x48"\
        > "&download=secret.txt" | sha256sum
        db6f423fe3f2537cba8555e940c559cd388a3cf8a5ad4fcf0be8ea067e1a6880  -
        ```
    - the complete URL that the attack can use is
        ```
        http://www.seedlab-hashlen.com?myname=he15enbug&uid=1002&lstcmd=1%80%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%01%48&download=secret.txt&mac=db6f423fe3f2537cba8555e940c559cd388a3cf8a5ad4fcf0be8ea067e1a6880
        ```
    - result: the page will display the following information (it indicates that we successfully get the content of `secret.txt`)
        ```
        Yes, your MAC is valid
        File Content
        TOP SECRET.
        DO NOT DISCLOSE.
        ```

## Task 4: Attack Mitigation using HMAC
- concatenating the key and the message is an insecure way to compute a MAC. The standard way to calculate MACs is to use HMAC, we need to modify the server program's `verify_mac()` function and use Python's `hmac` module to calculate the MAC (this function resides in `lab.py`). Given a key and message (both of type string), the HMAC can be computed as shown below
    ```
    real_mac = hmac.new(bytearray(key.encode('utf-8')),
                msg=message.encode('utf-8', 'surrogateescape'),
                digestmod=hashlib.sha256).hexdigest()
    ```
- after making changes, rebuild and restart all containers
- construct a request to do task `1` again, we use `hmac` to calculate the MAC, and then include it in the URL, then we can find `"Yes, your MAC is valid"` on the webpage
    ```
    $ ./hmac_calc.py 
    0fa86fdd06ed6c929e23684da37e8e2b534d5794eca7229988769de287d50eb9
    ```
    - URL: `http://www.seedlab-hashlen.com?myname=he15enbug&uid=1002&lstcmd=1&mac=0fa86fdd06ed6c929e23684da37e8e2b534d5794eca7229988769de287d50eb9`
- why length extension attack will fail MAC verification when the client and server use HMAC? The step of HMAC calculation is below
    1. key padding: make the key the same size as the block size of the hash function, if it is longer, hash it; if it is shorter, pad it
    2. define 2 constants `ipad` and `opad`, both equal to the block size, `inner_pad = key XOR ipad`, `outer_pad = key XOR opad`
    3. `h1 = hash(inner_pad || message)`
    4. `h2 = hash(outer_pad || h1)`
    5. the output is `h2`
    - if an attacker extend the message, they can get correct `h1` for that new message, but the calculation of `h2` will need `outer_pad`, which the attacker cannot know, so the hash length extension attack will fail
