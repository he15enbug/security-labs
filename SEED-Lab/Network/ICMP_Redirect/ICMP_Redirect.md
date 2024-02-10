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
