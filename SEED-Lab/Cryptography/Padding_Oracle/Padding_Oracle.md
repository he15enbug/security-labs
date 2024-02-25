# Padding Oracle Attack Lab
- learning an interesting attack on crypto systems. Some systems, when decrypting a given ciphertext, verify whether the padding is valid or not, and throw an error if the padding is invalid. This seemly-harmless behavior enables a type of attack called *padding oracle attack*. Many well-known systems were found vulnerable to this attack, including Ruby on Rails, ASP.NET, and OpenSSL
- in this lab, we are given 2 oracle servers running inside a container. Each oracle has a secret message hidden inside, we can know the ciphertext and the IV, but not the plaintext or the encryption key. We can send a ciphertext (and the IV) to the oracle, it will then decrypt the ciphertext using the encryption key, and tell us whether the padding is valid or not. Our job is to use the response from the oracle to figure out the content of the secret message
- topics
    - Secret-key encryption
    - Encryption modes and paddings
    - Padding oracle attack

## Task 1: Getting Familiar with Padding


## Task 2: Padding Oracle Attack (Level 1)

## Task 3: Padding Oracle Attack (Level 2)