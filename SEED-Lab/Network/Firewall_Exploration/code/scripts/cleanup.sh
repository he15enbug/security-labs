#!/bin/bash
iptables -F
iptables -P OUTPUT ACCEPT
iptables -P INPUT  ACCEPT