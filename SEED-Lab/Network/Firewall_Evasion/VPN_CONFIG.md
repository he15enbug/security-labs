# Bypassing Ingress Firewall Using VPN
```
(run on host A)
# ssh -w 0:0 root@<VPN_SERVER_IP> \
        -o "PermitLocalCommand=yes" \
        -o "LocalCommand= ip addr add 192.168.53.88/24 dev tun0 && ip link set tun0 up" \
        -o "RemoteCommand=ip addr add 192.168.53.99/24 dev tun0 && ip link set tun0 up"
# ip route add 192.168.20.5 via 192.168.53.99 dev tun0

(run on host A1)
# ip route add 192.168.20.5 via 10.8.0.99 dev eth0

(run on host B1)
# ip route add 192.168.53.88 via 192.168.20.99 dev eth0
```
# Bypassing Egress Firewall Using VPN
```
(run on host B)
# ssh -w 0:0 root@10.8.0.99 \
          -o "PermitLocalCommand=yes" \
          -o "LocalCommand= ip addr add 192.168.53.99/24 dev tun0 && ip link set tun0 up" \
          -o "RemoteCommand=ip addr add 192.168.53.88/24 dev tun0 && ip link set tun0 up"
# ip route add 93.184.216.0/24 via 192.168.53.88 dev tun0

(run on host B1)
# ip route add 93.184.216.0/24 via 192.168.20.99 dev eth0

(run on host A)
# ip route add 192.168.20.5 via 192.168.53.99 dev tun0
# iptables -t nat -A POSTROUTING -j MASQUERADE -o eth0
```