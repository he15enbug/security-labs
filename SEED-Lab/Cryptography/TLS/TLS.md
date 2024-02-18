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
    - print out the server certificate in the program
    - explain the purpose of `/etc/ssl/certs`
    - use Wireshark to capture the network traffic, explain which step triggers the TCP handshake, and which step triggers the TLS handshake, and explain the relationship between the TLS handshake and the TCP handshake

### Task 1.b: CA's Certificate
### Task 1.c: Experiment with the hostname check
### Task 1.d: Sending and getting Data

## Task 2: TLS Server
### Task 2.a: Implement a simple TLS server
### Task 2.b: Testing the server program using browsers
### Task 2.c: Certificate with multiple names

## Task 3: A Simple HTTPS Proxy
