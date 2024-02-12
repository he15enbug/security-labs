#!/bin/bash

iptables -A FORWARD -s 192.168.60.5 -i eth1 -p tcp --sport 23 -j ACCEPT
iptables -A FORWARD -d 192.168.60.5 -o eth1 -p tcp --dport 23 -j ACCEPT
iptables -P FORWARD DROP
