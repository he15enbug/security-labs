#!/usr/bin/env python3

from scapy.all import *

# interface = ['br-06753fb4dff8', 'enp0s3']
interface = 'br-06753fb4dff8'

def spoof_reply(pkt):
    print('get pkt')
    if ICMP in pkt and pkt[ICMP].type == 8:
        print('*****Original Packet*****')
        print(pkt[IP].src + ' ===> ' + pkt[IP].dst)
        ip = IP(src=pkt[IP].dst, dst=pkt[IP].src, ihl=pkt[IP].ihl)
        icmp = ICMP(type=0, id=pkt[ICMP].id, seq=pkt[ICMP].seq)
        data = pkt[Raw].load

        newpkt = ip/icmp/data
        print('*****Spoofed Packet*****')
        print(newpkt[IP].src + ' ===> ' + newpkt[IP].dst)
        send(newpkt, verbose=0)

pkt = sniff(iface = interface, filter = 'icmp', prn = spoof_reply)
