#!/bin/bash

# Allow SYN packet and ACK packet from internal hosts to external servers (dport 23)
iptables -A FORWARD -p tcp -i eth1 --dport 23 --syn -j ACCEPT
iptables -A FORWARD -p tcp -i eth1 --dport 23 --tcp-flags SYN,ACK,RST,FIN ACK -j ACCEPT

# Allow SYN+ACK packet and ACK packet from external servers to internal hosts (sport 23)
iptables -A FORWARD -p tcp -o eth1 --sport 23 --tcp-flags SYN,ACK,RST,FIN SYN,ACK -j ACCEPT
iptables -A FORWARD -p tcp -o eth1 --sport 23 --tcp-flags SYN,ACK,RST,FIN ACK -j ACCEPT

# Allow FIN+ACK packet and RST packet from both side
iptables -A FORWARD -p tcp -i eth1 --dport 23 --tcp-flags SYN,ACK,RST,FIN FIN,ACK -j ACCEPT
iptables -A FORWARD -p tcp -i eth1 --dport 23 --tcp-flags SYN,ACK,RST,FIN RST -j ACCEPT
iptables -A FORWARD -p tcp -o eth1 --sport 23 --tcp-flags SYN,ACK,RST,FIN FIN,ACK -j ACCEPT
iptables -A FORWARD -p tcp -o eth1 --sport 23 --tcp-flags SYN,ACK,RST,FIN RST -j ACCEPT

iptables -A FORWARD -s 192.168.60.5 -i eth1 -p tcp --sport 23 -j ACCEPT
iptables -A FORWARD -d 192.168.60.5 -o eth1 -p tcp --dport 23 -j ACCEPT
iptables -P FORWARD DROP

