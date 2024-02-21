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
