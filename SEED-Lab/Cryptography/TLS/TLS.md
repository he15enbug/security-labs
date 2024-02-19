# Transport Layer Security Lab
- data are transmitted over a public network unprotected, they can be read or even modified by others
- To achieve interoperability, i.e., allowing different applications to communicate with one another, these applications need to follow a common standard, *Transport Layer Security, TLS*. Most web servers are using HTTPS, which is built on top of TLS
- topics
    - Public-Key Infrastructure (PKI)
    - Transport Layer Security (TLS)
    - TLS programming
    - HTTPS proxy
    - X.509 certificates with the Subject Alternative Name (SAN) extensions
    - Man-In-The-Middle attacks
- prerequisite: this lab depends on the PKI lab

- *lab setup*
    - client: `10.9.0.5`
    - server: `10.9.0.43`
    - proxy: `10.9.0.143`

## Task 1: TLS Client
- incrementally build a simple TLS client program
### Task 1.a: TLS handshake
- before a client and a server can communicate securely, several things need to be set up first, including what encryption algorithm and key will be used, what MAC algorithm will be used, what algorithm should be used for key exchange, etc. These cryptographic parameters need to be agreed upon by the client and the server. This is he primary purpose of the TLS Handshake Protocol
- example program for the handshake protocol `handshake.py`
    ```
    import socket, ssl, sys, pprint
    hostname = sys.argv[1]
    port = 443
    cadir = '/etc/ssl/certs'

    # Set up the TLS context
    context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
    context.load_verify_locations(capath=cadir)
    context.verify_mode = ssl.CERT_REQUIRED
    context.check_hostname = True

    # Create TCP connection
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((hostname, port))
    input("After making TCP connection. Press any key to continue ...")

    # Add the TLS
    ssock = context.wrap_socket(sock, server_hostname=hostname, do_handshake_on_connect=False)
    ssock.do_handshake() # Start the handshake
    pprint.pprint(ssock.getpeercert())
    input("After handshake. Press any key to continue ...")

    # Close the TLS Connection
    ssock.shutdown(socket.SHUT_RDWR)
    ssock.close()
    ```
- *tasks*
    - use the code to communicate with a real HTTPS-based web server (additional code may need to be added to complete the tasks)
    - what is the cipher used between the client and server
        - code `print("=== Cipher used: {}".format(ssock.cipher()))`
        - output `=== Cipher used: ('TLS_AES_256_GCM_SHA384', 'TLSv1.3', 256)`
    - print out the server certificate in the program
        - code `pprint.pprint(ssock.getpeercert())`
    - explain the purpose of `/etc/ssl/certs`
        - this is the path for certificates of CAs, during the handshake, the client will get the server's certificate, it will use those CAs to verify the server's certificate
    - use Wireshark to capture the network traffic, explain which step triggers the TCP handshake, and which step triggers the TLS handshake, and explain the relationship between the TLS handshake and the TCP handshake
        1. the client will send SYN packet to the server, after TCP 3-way handshake, a TCP connection is established
        2. the client and the server will start the TLS handshake via this TCP connection
            - after each packet, the receiver will reply a separate TCP ACK packet
            1. C->S: Client Hello, include 31 cipher suites
            2. S->C: Hello Retry Request (choose cipher suite: `TLS_AES_256_GCM_SHA384`) + Change Cipher Spec
            3. C->S: Change Cipher Spec + Client Hello
            4. S->C: Server Hello + Application Data
            5. S->C: Application Data * 3
            6. C->S: Application Data
            7. S->C: Application Data
            8. S->C: Application Data
        3. close the TCP connection
            1. C->S: FIN+ACK
            2. S->C: ACK
            3. S->C: FIN+ACK
            4. C->S: ACK
### Task 1.b: CA's Certificate
- use our own CA certificate folder `./client-certs`
- we can use `pprint.pprint(context.get_ca_certs())` to see which CA is used to verify `www.example.com`, and then copy the certificate file of it to our folder
    ```
    [{'issuer': ((('countryName', 'US'),),
                (('organizationName', 'DigiCert Inc'),),
                (('organizationalUnitName', 'www.digicert.com'),),
                (('commonName', 'DigiCert Global Root G2'),)),
    'notAfter': 'Jan 15 12:00:00 2038 GMT',
    'notBefore': 'Aug  1 12:00:00 2013 GMT',
    'serialNumber': '033AF1E6A711A9A0BB2864B11D09FAE5',
    'subject': ((('countryName', 'US'),),
                (('organizationName', 'DigiCert Inc'),),
                (('organizationalUnitName', 'www.digicert.com'),),
                (('commonName', 'DigiCert Global Root G2'),)),
    'version': 3}]
    ```
- we can find a file `/etc/ssl/certs/DigiCert_Global_Root_G2.pem`, but it is a symbolic link, the actual file is `/usr/share/ca-certificates/mozilla/DigiCert_Global_Root_G2.crt`, we copy it to `Labsetup/volumes/client-certs`
    ```
    # ls -l /etc/ssl/certs/DigiCert_Global_Root_G2.pem
    lrwxrwxrwx 1 root root 62 Nov 24  2020 /etc/ssl/certs/DigiCert_Global_Root_G2.pem -> /usr/share/ca-certificates/mozilla/DigiCert_Global_Root_G2.crt
    ```
- copying it to `./client-certs` is not enough. When TLS tries to verify a server certificate, it will generate a hash value from the issuer's identify information, use this hash value as part of the file name, and then use this name to find the issuer's certificate in the `./client-certs` folder. Therefore, we need to rename each CA's certificate using the hash value generated from its subject field, or we can make a symbolic link out of the hash value
    ```
    # openssl x509 -in DigiCert_Global_Root_G2.crt -noout -subject_hash
    607986c7
    # ln -sf DigiCert_Global_Root_G2.crt 607986c7.0
    # ls -l 607986c7.0
    lrwxrwxrwx 1 seed seed 27 Feb 19 03:58 607986c7.0 -> DigiCert_Global_Root_G2.crt
    ```
- then, the handshake process should succeed

### Task 1.c: Experiment with the hostname check
- hostname check
- *step 1*: get the IP address of `www.example.com` using the dig command
    ```
    $ dig www.example.com
    ...
    ;; ANSWER SECTION:
    www.example.com.	491	IN	A	93.184.216.34
    ```
- *step 2*: modify `/etc/hosts` of the container, add `93.184.216.34 www.example2020.com`
- *step 3*: switch `context.check_hostname` in the client program between `True` and `False`, then connect the program to `www.example2020.com`
- *result*
    - `context.check_hostname=True`: the handshake process fails
        ```
        ssl.SSLCertVerificationError: [SSL: CERTIFICATE_VERIFY_FAILED] certificate verify failed: Hostname mismatch, certificate is not valid for 'www.example2020.com'. (_ssl.c:1123)
        ```
    - `context.check_hostname=False`: the handshake process succeeds
- *the importance of hostname check*: when `context.check_hostname=True`, the hostname will be checked during the handshake. An attacker can try to modify the victim's local DNS record, making `www.example.com` point to the attacker's server IP, which has a valid certificate, e.g., for `www.attacker.com`, if there is no hostname check, when the victim visits `www.example.com`, they will actually visit `www.attacker.com`, since `www.attacker.com` has a valid certificate, there won't be any error

### Task 1.d: Sending and getting Data



## Task 2: TLS Server
### Task 2.a: Implement a simple TLS server
### Task 2.b: Testing the server program using browsers
### Task 2.c: Certificate with multiple names

## Task 3: A Simple HTTPS Proxy
