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
- use `pcap` library
### Task 2.2: Spoofing
### Task 2.3: Sniff and then Spoof