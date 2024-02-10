# ICMP Redirect Attack Lab
- An ICMP redirect is an error message sent by a router to the sender of an IP packet. Redirects are used when router believes a packet is being routed incorrectly, and it would like to inform the sender that it should use a different route for the subsequent packets sent to that same destination. ICMP redirect can be used by attackers to change a victim's routing
- topics:
    - the IP and ICMP protocols
    - ICMP redirect attack
    - routing

## Environment Setup
- network `10.9.0.0/24`
    - attacker `10.9.0.105`
    - victim `10.9.0.5`
    - malicious router `10.9.0.111`
    - router `10.9.0.11`
- network `192.168.60.0/24`
    - router `192.168.60.11`
    - `192.168.60.5`
    - `192.168.60.6`

## Task 1: Launching ICMP Redirect Attack
- in Ubuntu, there is a countermeasure against the ICMP redirect attack, it has been turned off in the docker compose file by configuring the victim container to accept ICMP redirect messages
    ```
    sysctls:
        - net.ipv4.conf.all.accept_redirects=1
    ```
- for this task, attack the victim container from the attacker container, the victim will use the router to get to `192.168.60.0/24` network, if we run `ip route` on victim, we will see the following
    ```
    # ip route
    default via 10.9.0.1 dev eth0 
    10.9.0.0/24 dev eth0 proto kernel scope link src 10.9.0.5 
    192.168.60.0/24 via 10.9.0.11 dev eth0
    ```
- verification: ICMP redirect messages will not affect the routing table, but the routing cache, entries in the routing cache overwrite those in the routing table, until they expire, or are cleared by `ip route flash cache`
    - scenario 1: send ICMP redirect packet to the victim, then check the routing cache of the victim, no entries has been added
    - scenario 2: on victim container, run `ping 192.168.60.5`, then stop, check the cache, there is an entry `192.168.60.5 via 10.9.0.11 dev eth0`, on the attacker container, send ICMP redirect packet, then check the routing cache of the victim, the old entry has not been modified
    - scenario 3: on victim container, run `ping 192.168.60.5`, at the same time, on the attacker container, send ICMP redirect packet, this will add a malicious entry to the victim's routing cache
        ```
        # ip route show cache
        192.168.60.5 via 10.9.0.111 dev eth0
            cache <redirected> expires 298sec
        ```
- *the issue*: in scenario 1 and 2, the attack will not succeed because the victim is not sending ICMP packets during the attack, however, if the victim is not a container but a VM, this issue will not exist. Besides, the `ip2` packet inside the ICMP redirect packet must match the type (ICMP, UDP, etc.) and destination IP address of the packets that the victim is currently sending
    - this might be caused by some kind of sanity check in the OS kernel, before an ICMP redirect packet is accepted

- *questions*
    1. can you use ICMP redirect attacks to redirect to a remote machine? Namely, the IP address assigned to `icmp.gw` is a computer not on the local LAN (**No**, no entry will be added to the victim's routing cache)
    2. can you use ICMP redirect attacks to redirect to a non-existing machine on the same network? (**No**, e.g., we set `icmp.gw='10.9.0.112'`, when the victim receives the redirect packet, it will broadcast an ARP packet `Who has 10.9.0.112`, it will not add an entry to its cache if it doesn't get reply)
    3. in `docker-compose.yml`, there are the following entries for the malicious router container, what are the purposes of them
        ```
        sysctls:
            - net.ipv4.conf.all.send_redirects=0
            - net.ipv4.conf.default.send_redirects=0
            - net.ipv4.conf.eth0.send_redirects=0
        ```
        - these entries are set to disable sending ICMP redirect messages, e.g., `net.ipv4.conf.eth0.send_redirects=0` disables sending ICMP redirect messages for the `eth0` interface
        - when the spoofed ICMP redirect packet changed the routing cache of the victim, it will use the malicious router to communicate with `192.168.60.5`, if we use `mtr -n 192.168.60.5`, we can see that the ICMP packet will first get to the malicious router, then it will be sent to the benign router, and finally it will get to `192.168.60.5`
        - the problem is that if we enable sending ICMP redirect packet on the malicious router, as it knows that the benign router is able to reach the destination, it will send another ICMP redirect packet to the victim, and change its routing cache back to `192.168.60.5 via 10.9.0.11 dev eth0`
        - test: when all of these 3 values are set to `1`, launch the attack, on Wireshark, we will see 2 ICMP redirect packets, the first one is spoofed by the attacker, the second is from the malicious router to tell the victim that `10.9.0.11` is a router that can reach the destination

## Task 2: Launching the MITM Attack
- after launching the ICMP redirect attack, all packets from the victim to `192.168.60.5` will be routed through the malicious router, we would like to modify these packets
- before launching the MITM attack, start a TCP client and server program using `netcat`
    ```
    # nc -lp 9090 <--- on 192.168.60.5 as the server

    # nc 192.168.60.5 9090 <--- on the victim machine as the client
    ```
- each line typed by the victim will be put into a TCP packet and sent to the server, which displays the message
- *objective*: - replace the data with our first name (e.g., `123456789` -> `heisenbug`), the length of the modified data should be the same as the original data, or we will mess up the TCP sequence number, and hence the entire TCP connection
- notice that when typing `ABC` and enter, there will be 4 bytes of data (`'ABC'` and `\n`), I kept the last byte `'\n'`
- disable IP forwarding on the malicious router, otherwise, it will forward the packets it received
- we need to filter out the packets sent by the malicious router itself
    - `filter='tcp and src host 10.9.0.5 and dst host 192.168.60.5 and ether dst 02:42:0a:09:00:6f'`
    - `02:42:0a:09:00:6f` is the MAC address of the malicious router, packets between the victim and `192.168.60.5` will go through the malicious router, this is done by setting the Ethernet destination as the malicious router's MAC address, so add `ether dst 02:42:0a:09:00:6f` to filter out packets sent from the malicious router
- the code for modifying the payload of TCP packets from victim to destination server is the same as the code in ARP Poisoning Lab
- result
    - on the victim
        ```
        # nc 10.9.0.6 9090
        123456789
        xxx
        ```
    - on `192.168.60.5`
        ```
        # nc -lp 9090
        heisenbug
        hei
        ```
- *questions*
    1. in the MITM program, you only need to capture `nc` traffics from one direction, indicate which direction and why
        - we only need to capture traffics from the victim to the destination, because the routing cache of the destination is not modified, and the packets it sends to the victim will not go through the malicious router
    2. in the MITM program, you can use A's (i.e., the victim's) IP or MAC address in the filter, one of the choices is not good and will cause issues, try both, and tell which choice is the good one
        - since I have added the malicious router's MAC address as the Ethernet destination in the filter `ether dst 02:42:0a:09:00:6f`, it won't cause a problem when using either A's IP or MAC address in the filter, but if we didn't add `ether dst 02:42:0a:09:00:6f` to the filter, then using A's MAC address is better
        - suppose we use A's IP address, the complete filter expression is `filter='tcp and src host 10.9.0.5 and dst host 192.168.60.5`, when the MITM program receives a packet X, it modifies it to packet Y, and sends it out. The problem is that Y's source IP is also A's IP, which means Y will also be captured by the program (at the same time, it will be sent to the destination), this will cause the program to send another packet, and it will be captured again... The result is that the amount of packets in the network will keep growing, although the victim only sends one packet X initially, this is called a *forwarding storm*
        - if we use A's MAC in this case, since packet Y's Ethernet source is not A's MAC address but the malicious router's, it will not be captured by the program again, so there won't be a forwarding storm
