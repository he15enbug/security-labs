#!/bin/bash

echo -n "hello, world!" > plain.txt

# ECB
echo "AES 128 ECB Encryption..."
openssl enc -aes-128-ecb -e -in plain.txt -out ecb.bin \
                  -K 00112233445566778889aabbccddeeff \
# CBC
echo "AES 128 CBC Encryption..."
openssl enc -aes-128-cbc -e -in plain.txt -out cbc.bin \
                  -K 00112233445566778889aabbccddeeff \
                  -iv 0102030405060708
# CFB
echo "AES 128 CFB Encryption..."
openssl enc -aes-128-cfb -e -in plain.txt -out cfb.bin \
                  -K 00112233445566778889aabbccddeeff \
                  -iv 0102030405060708
# OFB
echo "Start AES 128 OFB Encryption..."
openssl enc -aes-128-ofb -e -in plain.txt -out ofb.bin \
                  -K 00112233445566778889aabbccddeeff \
                  -iv 0102030405060708

# Create 3 files with size 5, 10, 16 bytes
echo "Creating 3 files with size 5, 10, 16 bytes..."
echo -n "01234" > f1.txt
echo -n "0123456789" > f2.txt
echo -n "0123456789ABCDEF" > f3.txt

# Encrypt them with AES 128 CBC Encryption
echo "Encrypting the 3 files using AES 128 CBC Encryption..."
openssl enc -aes-128-cbc -e -in f1.txt -out f1.bin \
                  -K 00112233445566778889aabbccddeeff \
                  -iv 0102030405060708
openssl enc -aes-128-cbc -e -in f2.txt -out f2.bin \
                  -K 00112233445566778889aabbccddeeff \
                  -iv 0102030405060708
openssl enc -aes-128-cbc -e -in f3.txt -out f3.bin \
                  -K 00112233445566778889aabbccddeeff \
                  -iv 0102030405060708
# Display the size of all files
echo "Size of the Original Files:"
ls -l f*.txt
echo "Size of the Encrypted Files:"
ls -l f*.bin

# Decrypting
echo "Decrypting the Files..."
openssl enc -aes-128-cbc -d -nopad -in f1.bin -out f1_padded.txt \
                  -K 00112233445566778889aabbccddeeff \
                  -iv 0102030405060708
openssl enc -aes-128-cbc -d -nopad -in f2.bin -out f2_padded.txt \
                  -K 00112233445566778889aabbccddeeff \
                  -iv 0102030405060708
openssl enc -aes-128-cbc -d -nopad -in f3.bin -out f3_padded.txt \
                  -K 00112233445566778889aabbccddeeff \
                  -iv 0102030405060708
