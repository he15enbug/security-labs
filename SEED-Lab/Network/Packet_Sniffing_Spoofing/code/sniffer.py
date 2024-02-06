#!/usr/bin/env python3

from scapy.all import *

interface = 'br-06753fb4dff8'

def print_pkt(pkt):
    pkt.show()

pkt = sniff(iface=interface, filter='icmp', prn=print_pkt)
