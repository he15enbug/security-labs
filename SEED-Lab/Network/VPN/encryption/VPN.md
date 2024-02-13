# Virtual Private Network (VPN) Lab
- a VPN is used for creating a private scope of computer communication or providing a secure extension of a private network in an insecure network such as the Internet. VPN can be built upon IPSec or TLS/SSL (Transport Layer Security/Secure Socket Layer). These are two fundamentally different approaches for building VPNs. In this lab, we focus on the TLS/SSL-based VPNs
- objective: implement a simple VPN
- topics
    - VPN
    - TUN/TAP, and IP tunneling
    - routing
    - public-key cryptography, PKI, and X.509 certificate
    - TLS/SSL programming
    - authentication

## Task 1: VM Setup
- we will create a VPN tunnel between a computer (client) and a gateway, allowing the computer to securely access a private network via the gateway. We need at least three VMs: VPN client (also serving as host U), VPN server (the gateway), and a host in the private network (host V)
- network setup
    - "NAT Network" Adapter
        - VPN client, host U, `10.0.2.7`
        - VPN Server, gateway, `10.0.2.8`
    - "Internal Network" Adapter
        - VPN Server, gateway, `192.168.60.1`
        - host V, `192.168.60.101`
- in practice, the VPN client and VPN server are connected via the Internet. For simplicity, we directly connect these two machines to the same LAN
