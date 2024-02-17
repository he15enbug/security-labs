# Public-Key Infrastructure Lab
- public key cryptography is the foundation of today's secure communication, but it is subject to man-in-the-middle attacks when one side of communication sends its public key to the other side, the fundamental problem is that there is no easy way to verify the ownership of a public key. The *Public-Key Infrastructure, PKI* is a practical solution to this problem
- topics
    - Public-Key Infrastructure, PKI
    - Certificate Authority (CA), X.509 certificate, and root CA
    - Apache, HTTP, and HTTPS
    - Man-In-The-Middle attacks

- DNS setup
    ```
    10.9.0.80   www.bank32.com
    10.9.0.80   www.smith2020.com
    ```

## Task 1: Becoming a Certificate Authority (CA)
- a *Certificate Authority, CA* is a trusted entity that issues digital certificates. The digital certificate certifies the ownership of a public key by the named subject of the certificate. A number of commercial CAs are treated as root CAs. Root CAs certificates are self-signed
- in this task, we will make ourselves a root CA, and generate a certificate for this CA. Root CA's certificates are usually pre-loaded into most OSes, web browsers, and other software that rely on PKI. Root CA's certificates are unconditionally trusted

- *the configuration file `openssl.conf`*: to use `OpenSSL` to create certificates, we need to have a configuration file (`.conf`). It is used by three `OpenSSL` commands: `ca`, `req`, and `x509`. By default, `OpenSSL` use the configuration file from `/usr/lib/ssl/openssl.conf`. We will copy it to our current directory and instruct `OpenSSL` to use this copy instead
- the `[CA_default]` section of the configuration file shows the default setting that we need to prepare (we need to create some folders and files). Configure `unique_subject=no` to allow creation of several certificates with same subject. For `serial` file, put a single number in string format (e.g., `1000`) in the file
    ```
    [ CA_default ]

    dir         = ./demoCA          # Where everything is kept
    certs       = $dir/certs        # Where the issued certs are kept
    crl_dir     = $dir/crl          # Where the issued crl are kept
    database    = $dir/index.txt    # database index file
    unique_subject  = no            # Set to 'no' to allow creation of
                                    # several certs with same subject
    new_certs_dir   = $dir/newcerts # default place for new certs
    serial          = $dir/serial   # The current serial number
    ```
    ```
    mkdir -p demoCA
    mkdir -p demoCA/certs
    mkdir -p demoCA/crl
    touch demoCA/index.txt
    mkdir demoCA/newcerts
    echo 1000 > demoCA/serial
    ```
- *CA*: generate a self-signed certificate for our CA, its certificate will serve as the root certificate
    - `openssl req -x509 -newkey rsa:4096 -sha256 -days 360 -keyout ca.key -out ca.crt`
    - two files will be generated:
        1. `ca.key`: contains the CA's private key
        2. `ca.crt`: contains the CA's public key
- look at the decoded content of the X509 certificate and the RSA key (`-text` means decoding the content into plain text; `-noout` means not to print out the encoded version)
    - `openssl x509 -in ca.crt -text -noout`
    - `openssl rsa  -in ca.key -text -noout`

## Task 2: Generating a Certificate Request for your Web Server
- a company called `bank32.com` wants to get a public-key certificate from our CA. First, it needs to generate a Certificate Signing Request (CSR), which basically includes the company's public key and identity information. The CSR will be sent to the CA, who will verify the identity information in the request, and then generate a certificate

## Task 3: Generating a Certificate for your Server

## Task 4: Deploying Certificate in an Apache-Based

## Task 5: Launching a Man-In-The-Middle Attack

## Task 6: Launching a Man-In-The-Middle Attack with a Compromised CA