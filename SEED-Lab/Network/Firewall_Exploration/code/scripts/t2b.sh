#!/bin/bash
iptables -A INPUT -p icmp --icmp-type echo-request -j ACCEPT
iptables -A OUTPUT -p icmp --icmp-type echo-reply -j ACCEPT

# only allow ICMP request going out and ICMP reply coming in (eth1)
iptables -A FORWARD -i eth1 -p icmp --icmp-type echo-request -j ACCEPT
iptables -A FORWARD -o eth1 -p icmp --icmp-type echo-reply -j ACCEPT

iptables -P FORWARD DROP
iptables -P OUTPUT DROP
iptables -P INPUT DROP
