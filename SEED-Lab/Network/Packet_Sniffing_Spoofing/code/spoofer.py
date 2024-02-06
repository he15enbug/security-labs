#!/usr/bin/env python3

from scapy.all import *

a = IP()
a.src = '10.9.0.6'
a.dst = '10.9.0.5'
b = ICMP()
p = a/b
ls(p)
send(p)
