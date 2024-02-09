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

## Task 2: Launching the MITM Attack
