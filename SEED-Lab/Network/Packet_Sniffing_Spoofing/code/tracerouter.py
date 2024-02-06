#!/usr/bin/env python3

from scapy.all import *

a = IP()
a.ttl = 18
a.dst = '8.8.8.8'
b = ICMP()
p = a/b
ls(p)
send(p)
