#!/bin/bash

iptables -t nat -A PREROUTING -p udp --dport 8080 \
		 -m statistic --mode nth --every 3 --packet 0 \
		 -j DNAT --to-destination 192.168.60.5:8080
iptables -t nat -A PREROUTING -p udp --dport 8080 \
		 -m statistic --mode nth --every 3 --packet 1 \
		 -j DNAT --to-destination 192.168.60.6:8080
iptables -t nat -A PREROUTING -p udp --dport 8080 \
		 -m statistic --mode nth --every 3 --packet 2 \
		 -j DNAT --to-destination 192.168.60.7:8080