#!/usr/bin/env python3

import fcntl
import struct
import os
import time
from scapy.all import *

TUNSETIFF = 0x400454ca
IFF_TUN   = 0x0001
IFF_TAP   = 0x0002
IFF_NO_PI = 0x1000

# Create the tun interface
tun = os.open("/dev/net/tun", os.O_RDWR)
ifr = struct.pack('16sH', b'he15enbug%d', IFF_TUN | IFF_NO_PI)
ifname_bytes  = fcntl.ioctl(tun, TUNSETIFF, ifr)

# Get the interface name
ifname = ifname_bytes.decode('UTF-8')[:16].strip("\x00")
print("Interface Name: {}".format(ifname))

os.system("ip addr add 192.168.53.99/24 dev {}".format(ifname))
os.system("ip link set dev {} up".format(ifname))

SPOOF_REPLY = True
while True:
    # Get a packet from the tun interface
    packet = os.read(tun, 2048)
    if packet:
        ip = IP(packet)
        print(ip.summary())
        print(ip.payload.type)
        if SPOOF_REPLY:
            # Send out a spoof packet using the tun interface
            newip = IP(src=ip.dst, dst=ip.src)
            newicmp = ip.payload
            newicmp.type = 0
            # clear the chksum, then Scapy will recalculate it
            newicmp.chksum = None
            newpkt = newip/newicmp
            os.write(tun, bytes(newpkt))
        else:
            # Instead of IP packets, write arbitrary data to the tunnel
            data = b'hello world'
            os.write(tun, bytes(data))