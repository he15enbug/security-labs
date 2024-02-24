#!/bin/bash

openssl enc -aes-128-ecb -e -in my_pic2.bmp -out ecb.bmp \
                  -K 00112233445566778889aabbccddeeff \

openssl enc -aes-128-cbc -e -in my_pic2.bmp -out cbc.bmp \
                  -K 00112233445566778889aabbccddeeff \
                  -iv 0102030405060708

head -c 54 my_pic2.bmp  > head
tail -c +55 ecb.bmp > ecb_body
tail -c +55 cbc.bmp > cbc_body
cat head ecb_body > pic_ecb.bmp
cat head cbc_body > pic_cbc.bmp
