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
    ```
    
    ```


### Task 2.2: Spoofing
### Task 2.3: Sniff and then Spoof