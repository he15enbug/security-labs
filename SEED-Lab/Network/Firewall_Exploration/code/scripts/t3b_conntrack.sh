#!/bin/bash

# Allow packets via ESTABLISHED or RELATED connections
iptables -A FORWARD -p tcp -m conntrack --ctstate ESTABLISHED,RELATED -j ACCEPT
# Allow the interal hosts to initiate a TCP connection with external servers
iptables -A FORWARD -p tcp -i eth1 --dport 23 --syn -m conntrack --ctstate NEW -j ACCEPT

iptables -A FORWARD -s 192.168.60.5 -i eth1 -p tcp --sport 23 -j ACCEPT
iptables -A FORWARD -d 192.168.60.5 -o eth1 -p tcp --dport 23 -j ACCEPT
iptables -P FORWARD DROP
