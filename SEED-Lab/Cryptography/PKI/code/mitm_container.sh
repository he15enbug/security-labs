#!/bin/bash

# use ca.crt and ca.key to generate a certificate of www.example.com
openssl req -newkey rsa:2048 -sha256 \
            -keyout example.key -out example.csr \
            -subj "/CN=www.example.com/O=EXAMPLE Inc./C=US" \
            -passout pass:dees
openssl ca -config my_openssl.cnf -policy policy_anything \
        -md sha256 -days 3650 \
        -in example.csr -out example.crt -batch \
        -cert ca.crt -keyfile ca.key

# copy the certificate and private key file to the shared folder
cp example.crt ../volumes
cp example.key ../volumes
