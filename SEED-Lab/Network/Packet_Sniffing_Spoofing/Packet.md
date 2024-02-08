# Packet Sniffing and Spoofing Lab
- many tools for packet sniffing and spoofing, such as `Wireshark`, `Tcpdump`, `Netwox`, `Scapy`
- topics
    - How the sniffing and spoofing work
    - Packet sniffing using the `pcap` library and Scapy
    - Packet spoofing using raw socket and Scapy
    - Manipulating packets using Scapy

## Environment Setup
- network `10.9.0.0/24`
    - Host A `10.9.0.5`
    - Host B `10.9.0.6`
    - Attacker `10.9.0.1`
        - there is a shared foler `./volumes` (host) -> `/volumes` (container)
        - `network mode: host`
- network interface: prefix is `br-`

## Lab Task Set 1: Using `Scapy` to Sniff and Spoof Packets
### Task 1.1: Sniffing Packets
- `pkt = sniff(iface='br-...', filter='icmp', prn=print_pkt)`
- we need to define `print_pkt(pkt)` to do something to the packet, e.g., print it out (using `pkt.show()`)
#### Task 1.1A
- run the program with and without root privilege: when running without privilege, there will be an error: `"PermissionError: [Errno 1] Operation not permitted"`
#### Task 1.1B
- `Scapy`'s filter uses the BPF (Berkeley Packet Filter) syntax
    - Capture only ICMP packet: `"icmp"`
    - Capture any TCP packet that comes from a particular IP and with a destination port number 23: `tcp and src host 1.2.3.4 and dst port 23`
    - Capture packets comes from or to go to a particular subnet, e.g., `128.230.0.0/16`: `net 128.230.0.0/16`

### Task 1.2: Spoofing ICMP Packets
- construct an ICMP packet with spoofed source `10.9.0.6` and send it to `10.9.0.5`, use `Wireshark` to observe whether our request will be accepted by the receiver (if accepted, an echo reply packet will be sent back)
- the result shows that the receiver accepted this spoofed ICMP request packet, there is a ICMP echo reply packet from `10.9.0.5` to `10.9.0.6`
### Task 1.3: Traceroute
- use `Scapy` to estimate the distance, in terms of number of routers, between the VM and a selected destination
- set `ttl` of an IP packet, send it to `8.8.8.8`, and use `Wireshark` to observe whether there is a reply (capture packets from interface `enp0s3`)
- by adjusting `ttl`, we can know that when `ttl` is `18`, there will be a reply
### Task 1.4: Sniffing and then Spoofing
- run a sniff-and-then-spoof program on the VM, whenever a container pings an IP, the program should spoof a echo reply packet
    ```
    def spoof_reply(pkt):
        if ICMP in pkt and pkt[ICMP].type == 8: # 8 is echo request
            print('*****Original Packet*****');
            print(pkt[IP].src + '===>' + pkt[IP].dst)
            ip = IP(src=pkt[IP].dst, dst=pkt[IP].src, ihl=pkt[IP].ihl)
            icmp = ICMP(type=0, id=pkt[ICMP].id, seq=pkt[ICMP].seq)
            data = pkt[Raw].load

            newpkt = ip/icmp/data
            print('*****Spoofed Packet*****')
            print(newpkt[IP].src + '===>' + newpkt[IP].dst)
            send(newpkt, verbose=0)
    ```
- do the following test on `10.9.0.6`
    1. `ping 1.2.3.4 # a non-existing host on the Internet`: for each request, exactly 1 reply, this reply is spoofed by the program
        ```
        root@8aa2b03a317d:/# ping 1.2.3.4
        PING 1.2.3.4 (1.2.3.4) 56(84) bytes of data.
        64 bytes from 1.2.3.4: icmp_seq=1 ttl=64 time=76.1 ms
        ```
    2. `ping 10.9.0.99 # a non-existing host on the LAN`: when the host pings an IP on the LAN, before it sends the ICMP packets, it will first broadcast ARP packet to ask for `10.9.0.99` (packet information `Who has 10.9.0.99? Tell 10.9.0.6`), if no one knows, the host will not send the ICMP packet
        ```
        root@8aa2b03a317d:/# ping 10.9.0.99
        PING 10.9.0.99 (10.9.0.99) 56(84) bytes of data.
        From 10.9.0.6 icmp_seq=1 Destination Host Unreachable
        ```
    3. `ping 8.8.8.8 # an existing host on the Internet`: will receive duplicated reply (because the real destination can also receive the request, and will send a reply)
        ```
        root@8aa2b03a317d:/# ping 8.8.8.8 
        PING 8.8.8.8 (8.8.8.8) 56(84) bytes of data.
        64 bytes from 8.8.8.8: icmp_seq=1 ttl=110 time=83.0 ms
        64 bytes from 8.8.8.8: icmp_seq=1 ttl=64 time=87.1 ms (DUP!)
        ```
    4. `ping 10.9.0.5 # an existing host on the LAN`: will receive duplicated reply, same as `8.8.8.8`
        ```
        root@8aa2b03a317d:/# ping 10.9.0.5
        PING 10.9.0.5 (10.9.0.5) 56(84) bytes of data.
        64 bytes from 10.9.0.5: icmp_seq=1 ttl=64 time=0.461 ms
        64 bytes from 10.9.0.5: icmp_seq=1 ttl=64 time=50.5 ms (DUP!)
        ```
## Lab Task Set 2: Writing Programs to Sniff and Spoof Packets
- in this task set, write programs using C
### Task 2.1: Writing Packet Sniffing Program
- use `pcap` library, need to use `-lpcap` when compiling
- the sample code
    ```
    #include <pcap.h>
    #include <stdio.h>
    #include <stdlib.h>

    /* This function will be invoked by pcap for each captured packet, we can process each packet inside the function
    */
    void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
        printf("Got a packet\n");
    }
    int main() {
        pcap_t *handle; // this handle is used to manage the packet capture session
        char errbuf[PCAP_ERRBUF_SIZE]; // hold error messages
        struct bpf_program fp; // hold compiled BPF (Berkeley Packet Filter) program. BPF is a filtering mechanism used to specify which packets should be captured
        char filter_exp[] = "icmp"; // hold filter expression
        bpf_u_int32 net; // unsigned 32-bit integer, store the network address associated with the network interface used for packet capture

        /* Step 1: Open live pcap session on NIC with name "br-06753fb4dff8"
         * 1: whether to put the interface into promiscuous mode
         * 1000: the timeout value in milliseconds for capturing packets
         */
        handle = pcap_open_live("br-06753fb4dff8", BUFSIZ, 1, 1000, errbuf);

        /* Step 2: Compile filter_exp into BPF psuedo-code
         * &fp: will be filled in with the compiled filter program
         * 0: typically set to 0 for backward compatibility and is reserved for future use, can be safely ignored for now
         * net: the network mask of the network on which packets are being captured
         */
        pcap_compile(handle, &fp, filter_exp, 0, net);
        if(pcap_setfilter(handle, &fp) != 0) {
            pcap_perror(handle, "Error:");
        }

        /* Step 3: Capture packets
         * -1: `pcap_loop()` should loop indefinitely
         * NULL: a pointer to user data that can be passed to the callback function, in this case, no data is passed
         */
        pcap_loop(handle, -1, got_packet, NULL);

        pcap_close(handle); // Close the handle
        return 0;
    }
    ```
#### Task 2.1A: Understand How a Sniffer Works
- write a sniffer program to print out the source and destination IP addresses of each captured packet
    ```
    struct ethheader* eth = (struct ethheader*) packet;
    if(ntohs(eth->ether_type) == 0x0800) { // 0x0800 is IP type
        struct ipheader* ip= (struct ipheader*) (packet + sizeof(struct ethheader));
        printf("From: %s\n", inet_ntoa(ip->iph_sourceip));
        printf("  To: %s\n", inet_ntoa(ip->iph_destip));
    }
    ```
- describe the sequence of the library calls that are essential for sniffer programs
    1. open live session on a interface
    2. compile the filter expression to BPF program
    3. define a function to process the captured packet
    4. run the loop to capture packets, and process them with the function defined in previous step
    5. close the session

- why root privilege is needed to run a sniffer program? Where does the program fail if it is executed without root privilege
    - without root privilege, we can only run `pcap_open_live()`, then there will be a `Segment Fault` error. This is because when using `pcap` to sniff packets, the program needs low-level access to the network interface

- turn on and off the promiscuous mode in the sniffer program. The value `1` of the third parameter in `pcap_open_live()` turns on the promiscuous mode (and use `0` to turn it off). Demonstrate the difference when this mode is on and off. Use the following command to check whether an interface's promiscuous mode is on or off (look at the `promiscuity`'s value)
    - `# ip -d link show dev br-537c81d8b33b | grep promiscuity` (I restarted the container, so the network interface name changed)
    - run the sniff program on `10.9.0.1`, and test `ping 10.9.0.5` on host `10.9.0.6`
    - on
        - `promiscuity` is 1, can capture the packets between `10.9.0.5` and `10.9.0.6`
    - off
        - `promiscuity` is 0, cannot capture any packets
    - run the sniff program, this time, test `ping 10.9.0.99` on host `10.9.0.6`, this IP address is a non-existing IP on the LAN, host `10.9.0.6` will broadcast ARP packets to ask for the target host, no matter whether this mode is on, the program is able to capture the ARP packets
    - run the sniff program, `ping 10.9.0.1` on host `10.9.0.6`, in both cases the packets and reply packets are captured
    - the difference when promiscuous mode is on and off is that when this mode is off, the program can only capture packets to or from the address of the host that is running it

#### Task 2.1B: Writing Filters
- capture the ICMP packets between two specific hosts
    - `"icmp and host 10.9.0.5 and host 10.9.0.6"`
- capture the TCP packets with destination port number in the range from 10 to 100
    - `"tcp dst portrange 10-100"`
    - test this on `10.9.0.6` using command `echo "test" | nc -w1 10.9.0.5 10`
#### Task 2.1C: Sniffing Passwords
- show how to use sniffer program to capture the password when somebody is using `telnet` on the network that we are monitoring (print out the data part of a TCP packet)
    - core code: get the payload in TCP packet
        ```
        // remember to use ntohs() to convert the byte order
        int size_data = ntohs(ip->iph_len) - sizeof(struct ipheader) - sizeof(struct tcpheader);
        // skip ethernet header, IP header, and TCP header
        unsigned char *data = (unsigned char* ) (packet + sizeof(struct ethheader) + sizeof(struct ipheader) + sizeof(struct tcpheader));
        // for convenience, also write the data to a file
        for(int i = 0; i < size_data; i++) {
            if(file != NULL) fwrite(data, sizeof(char), 1, file);
            if(isprint(*data)) printf("%c", *data);
            else printf(".");
            data++;
        }
        ```
- tips: when dealing with some `4-byte` lengths (e.g., ip packet length, which is an unisigned short integer), we need to use `ntohs()` to convert the representation from big endian (network byte order) to little endian (our host byte order), but we shouldn't apply `ntohs()` to the ip header length, because it is `1 byte`
- test: use Scapy to send a TCP packet with data `"this is a test"`
    - construct IP, TCP, and data (a string), then `send(ip/tcp/data);`
    - in the printout of the sniff program, the string `"this is a test"` will be printed out
- sniffing the password: when `10.9.0.6` tried to login to `10.9.0.5` using `telnet 10.9.0.5 23`, it will provide the user name and the password, in the sniff program, use `"tcp and host 10.9.0.6 and host 10.9.0.5"` to capture only TCP packets between `10.9.0.6` and `10.9.0.5`
    - first, try this password `++++++++`, in the output file of the sniff program (open it with a hex reader), we cannot found a string `"++++++++"`, but we can find several character `+`, this is because when typing the password, `10.9.0.6` will send multiple TCP packets, instead of contain all characters of the password in a single packet, because `telnet` operates in a character-at-a-time mode, meaning that each keystroke is immediately transmitted to the server
    - each character is sent as the last byte of the payload in a packet, we can recover the password by observing a sequence of packets, e.g., if the password is `dees`, we can recover it from the payload of the following packets
        ```
        01 01 08 0A 30 97 B8 88 92 68 52 B9 64 <---- last byte: 0x64 ('d')
        01 01 08 0A 30 97 C3 46 92 68 72 C3 65 <---- last byte: 0x65 ('e')
        01 01 08 0A 30 97 CB 37 92 68 7D 56 65 <---- last byte: 0x65 ('e')
        01 01 08 0A 30 97 D6 7F 92 68 85 47 73 <---- last byte: 0x73 ('s')
        ```
    - how to know which packets the password is from: these packets are captured after a packet from server to client with string `"Password:"` in its payload, and before a packet with `"Welcome to Ubuntu"` in its payload, and the password should consists of printable character

### Task 2.2: Spoofing
- when a normal user sends out a packet, the OS usually don't allow users to set all the fields in the protocol headers (such as TCP, UDP, and IP headers). The OS only allow users to set a few fields, such as destination IP address, the destination port number, etc. However, if users have the root privilege, they can set any arbitrary field in the packet headers, this is called packet spoofing, and can be done through *raw sockets*
- using raw sockets involves 4 steps: 1. create a raw socket, 2. set socket option, 3. construct the packet, and 4. send out the packet through the raw socket
- a sample skeleton
    ```
    int sd;
    struct sockaddr_in sin;
    char buffer[1024];

    /* Create a raw socket with IP protocol
     * The IPPROTO_RAW parameter tells the system that the IP header is already included
     * this prevents the OS from adding another IP header 
     * returns a socket descriptor */
    sd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if(sd < 0) {
        perror("socket() error");
        exit(-1);
    }
    /* This data structure is needed when sending the packets using sockets
     * Normally, we nned to fill out several fields, but for raw sockets, we only need to
     * fill out this one field */
    sin.sin_family = AF_INET;

    // Construct the IP packet using buffer[]
    // - construct the IP header ...
    // - construct the TCP/UDP/ICMP header ...
    // - fill in the data part if needed ...
    // pay attention to the network/host byte order

    /* Send out the IP packet
     * ip_len is the actual size of the packet */
     if(sendto(sd, buffer, ip_len, 0, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("sendto() error");
        exit(-1);
     }
    ```
#### Task 2.2A: Write a spoofing program
- spoof an IP packet
- fill the IP header
    ```
    struct ipheader* ip  = (struct ipheader *) buffer;
    // fill in the IP header
    ip->iph_ihl             = 5;
    ip->iph_ver             = 4;
    ip->iph_ttl             = 20;
    ip->iph_sourceip.s_addr = inet_addr(SRC_IP); // Source IP
    ip->iph_destip.s_addr   = inet_addr(DEST_IP);  // Dest IP
    ip->iph_protocol        = IPPROTO_TCP; // The value is 6.
    // mind the byte order!
    ip->iph_len             = htons(sizeof(struct ipheader));
    // ip->iph_chksum will be set by the system
    ```
- this packet will be captured on Wireshark
#### Task 2.2B: Spoof an ICMP Echo Request
- besides filling in the IP header, also need to fill in the ICMP header
    ```
    struct icmpheader* icmp = (struct icmpheader *) (buffer + sizeof(struct ipheader));

    // fill in the ICMP header
    icmp->icmp_type   = 8; // 8: request, 0: reply
    icmp->icmp_chksum = in_cksum((unsigned short *) icmp, sizeof(struct icmpheader));
    ```
- By running this program we sent an ICMP request packet to `8.8.8.8`, and a reply packet was captured

- If we set the length field of IP packet to an arbitrary value:
    - if we set it smaller than `20` (the length of IP header), there will be an error when invoking `sendto`: `Invalid argument` (for IP packet), or a segment fault (for TCP packet)
    - if we set it to a larger value, there won't be any error, but if it is a TCP packet, it may lose some information, and become an IP packet

- Do we have to calculate the checksum for the IP header: No, the system will do it for us

- Why do we need the root privilege to run the spoof program
    - If the program is executed without root privilege, it will fail to create the raw socket, with the error information: `socket() error: Operation not permitted`

### Task 2.3: Sniff and then Spoof
- combine the sniffing and spoofing techniques to implement a sniff-and-then-spoof program
- the program runs on the attacker machine, and monitors the LAN through packet sniffing, whenever it sees an ICMP echo request, regardless of what the target IP address is, the program will immediately send out a spoofed echo reply packet
- just combine the previous tasks `2.1` and `2.2B`