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

- *the configuration file `openssl.cnf`*: to use `OpenSSL` to create certificates, we need to have a configuration file (`.cnf`). It is used by three `OpenSSL` commands: `ca`, `req`, and `x509`. By default, `OpenSSL` use the configuration file from `/usr/lib/ssl/openssl.cnf`. We will copy it to our current directory and instruct `OpenSSL` to use this copy instead
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
- the command to generate a CSR is similar to the one we used to self-sign certificate for our root CA. The only difference is the `-x509` option. Without it, the command generates a request; with it, the commands generates a self-signed certificate. The following command will generate a pair of public/private key, and create a certificate signing request from the public key
    ```
    openssl req -newkey rsa:2048 -sha256 \
                -keyout server.key -out server.csr \
                -subj "/CN=www.bank32.com/O=Bank32 Inc./C=US" \
                -passout pass:dees
    ```
- check the csr `openssl req -in server.csr -text -noout`
- *adding alternative names*
    - many websites have different URLs. Due to the hostname matching policy enforced by browsers, the common name in a certificate must match with the server's hostname, or browser will refuse to communicate with the server
    - X509 specification defines extensions to be attached to a certificate. This extension is called *Subject Alternative Name (SAN)*. Using SAN, we can specify several hostnames in the `subjectAltName` field of a certificate (`subjectAltName` must contains the hostname in the `/CN` field, or the common name won't be accepted as a valid common name)
        ```
        openssl req -newkey rsa:2048 -sha256 \
                -keyout server.key -out server.csr \
                -addext "subjectAltName = DNS:www.bank32.com, \
                                          DNS:www.bank32A.com, \
                                          DNS:www.bank32B.com" \
                -subj "/CN=www.bank32.com/O=Bank32 Inc./C=US" \
                -passout pass:dees
        ```
## Task 3: Generating a Certificate for your Server
- the CSR file needs to have the CA's signature to form a certificate. The following command turns the certificate signing request (`server.csr`) into an X509 certificate (`server.crt`), using the CA's `ca.crt` and `ca.key`
    ```
    openssl ca -config my_openssl.cnf -policy policy_anything \
               -md sha256 -days 3650 \
               -in server.csr -out server.crt -batch \
               -cert ca.crt -keyfile ca.key
    ```
- `my_openssl.cnf` is the configuration file we copied and modified in task `1`. `policy_anything` is a policy defined in the configuration file, it doesn't enforce any matching rule
- *copy the extension field*: for security reasons, the default setting in `openssl.cnf` does not allow the `openssl ca` command to copy the extension field from the request to the final certificate, to enable that, we need to add `copy_extensions = copy` to `my_openssl.cnf`
- then, we regenerate `server.crt`, and use `openssl x509 -in server.crt -text -noout` to see the content, we will find the following field
    ```
    X509v3 Subject Alternative Name:
        DNS:www.bank32.com, DNS:www.bank32A.com, DNS:www.bank32B.com
    ```
## Task 4: Deploying Certificate in an Apache-Based
- an Apache server can simultaneously host multiple websites. It need to know the directory where a website's files are stored. This is done via its `VritualHost` file, located in `/etc/apache2/sites-available` directory. In our container, we have a file called `bank32_apache_ssl.conf`
    ```
    <VirtualHost *:443>
        DocumentRoot /var/www/bank32
        ServerName www.bank32.com
        ServerAlias www.bank32A.com
        ServerAlias www.bank32B.com
        DirectoryIndex index.html
        SSLEngine On
        SSLCertificateFile      /certs/bank32.crt
        SSLCertificateKeyFile   /certs/bank32.key
    </VirtualHost>
    ```
- to make the website work, we need to enable Apache's `ssl` module and then enable the site (already enabled when the container was built)
    - `# a2enmod ssl`
    - `# a2ensite bank32_apache_ssl`
- *starting the Apache server*
    - the Apache server is not automatically started in the container, because of the need to type the password to unlock the private key. Run `service apache2 start` in the container
    - when Apache starts, it needs to load the private key for each HTTPS site. Our private key is encrypted, so Apache will ask us to type the password
        ```
        # service apache2 start
        * Starting Apache httpd web server apache2
        Enter passphrase for SSL/TLS keys for www.bank32.com:443 (RSA):
        * 
        ```
- *browsing the website*
    - we will not be able to load `https://www.bank32.com` (Error code: `SEC_ERROR_UNKNOWN_ISSUER`), because we are using our own root CA, now what we need to do is to add our root CA to the browser, and we also need to replace the `bank.crt` and `bank.key` with the files generated in previous tasks
    1. copy the generated certificate and private key to `/certs/` in the container, rename them to `bank32.crt` and `bank32.key`, respectively
    2. in Firefox, add our CA: type `aboutLpreference#privacy` in the address bar to go to the privacy settings, click `View Certificates`, in `Authorities` tab, we can see a list of certificates accepted by Firefox, we need to import `ca.crt`, and select `Trusted this CA to identify web sites`
    - *important*: after replacing the certificate and key file, we need to restart apache2 `service apache2 restart`, otherwise, we still cannot visit `https://www.bank32.com`

## Task 5: Launching a Man-In-The-Middle Attack
- *Man-In-The-Middle Attack*: assume Alice wants to visit `example.com` via the HTTPS protocol, she needs to get the public key from `example.com` server. Alice will generate a secret, and encrypt the secret using the server's public key, and sent it to the server. If an aatacker can intercept the communication between Alice and the server, the attacker can replace the server's public key with its own public key. Alice's secret will be encrypted with the attacker's public key, so the attacker will be able to read the secret. And the attacker can re-encrypt it using the server's public key. Overall, the attacker can impersonate Alice to the server, and impersonate the server to Alice

- *step 1: setting up the malicious website*
    - use the same Apache server to impersonate `www.example.com`, to achieve that, we will follow the instruction in task `4` to add a `VirtualHost` entry to Apache's SSL configuration file: `ServerName` should be `www.example.com`, the rest of the configuration can be the same as that used in Task `4`. In the real world, we won't be able to get a valid certificate for `www.example.com`, so we will use the same certificate that we used for our own server
    - goal: when a user tries to visit `www.example.com`, we are going to get the user to land in our server, which hosts a fake website for `www.example.com`
- *step 2: becoming the man in the middle*
    - there are several ways to get the user's HTTPS request to land in our web server. One way is to attack the routing, so the user's HTTPS request is routed to our web server. Another way is to attack DNS, so when the victim's machine tries to find out the IP address of the target web server, it gets the IP address of our web server. In this task, we will simulate the attack-DNS approach by simply modifying the victim's `/etc/hosts`, i.e., adding `10.9.0.80 www.example.com`
- *step 3: browse the target website*
    - before browsing the target website, we need to enable the configuration file for the fake `www.example.com`, and restart `apache2`
        ```
        # a2ensite example_apache_ssl.conf
        # server apache2 restart
        (Enter password for the private key file of www.example.com)
        ```
- *result*: we won't be able to load the fake `www.example.com`, the error code is `SSL_ERROR_BAD_CERT_DOMAIN`, because we are using the certification for `www.bank32.com` (in real world, we won't be able to get the private key of the certificate for the real `www.example.com`, unless we can compromise the CA)

## Task 6: Launching a Man-In-The-Middle Attack with a Compromised CA
- suppose that the attacker gets the private key of the root CA in previous tasks. Therefore, the attacker can generate any arbitrary certificate using this CA's private key
- the attacker can generate a valid certificate for the `www.example.com`
    ```
    openssl req -newkey rsa:2048 -sha256 \
                -keyout example.key -out example.csr \
                -subj "/CN=www.example.com/O=EXAMPLE Inc./C=US" \
                -passout pass:dees
    openssl ca -config my_openssl.cnf -policy policy_anything \
            -md sha256 -days 3650 \
            -in example.csr -out example.crt -batch \
            -cert ca.crt -keyfile ca.key
    ```
- modify `example_apache_ssl.conf`, replace the certificate and private key file
    ```
    SSLCertificateFile      /certs/example.crt
    SSLCertificateKeyFile   /certs/example.key    
    ```
- then restart Apache, and visit `www.example.com` in browser, the fake site will be visited
