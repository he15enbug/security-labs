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
    - Host M (Attacker): `10.9.0.105`
    - Host A: `10.9.0.5`
    - Host B: `10.9.0.6`

## Task 1: ARP Cache Poisoning

## Task 2: MITM Attack on Telnet using ARP Cache Poisoning

## Task 3: MITM Attack on Netcat using ARP Cache Poisoning
