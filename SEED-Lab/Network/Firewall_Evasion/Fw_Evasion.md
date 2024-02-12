# Firewall Evasion Lab
- there are situations where firewalls are too restrictive, making it incovenient for users. For example, many companies and schools enfore egress filtering, which locks users inside of their networks from reaching out to certain websites or Internet services, such as game and social network sites. There are many ways to evade firewalls. A typical approach is to use tunneling technique, which hides the real purposes of network traffic. There are a number of ways to establishing tunnels, the two most common tunneling techniques are *Virtual Private Network (VPN)* and *port forwarding*
- topics
    - firewall evasion
    - VPN
    - port forwarding
    - SSH tunneling

## Task 0: Get Familiar with the Lab Setup
- network `10.8.0.0/24`
    - `10.8.0.1`
    - A1 `10.8.0.5`
    - A2 `10.8.0.6`
    - A `10.8.0.99`
    - router `10.8.0.11 (eth0)`
- network `192.168.20.0/24`
    - router `192.168.20.11 (eth1)`
    - B `192.168.20.99`
    - B1 `192.168.20.5`
    - B2 `192.168.20.6`
- *router configuration: setting up NAT*: the following command is included in the router configuration inside `docker-compose.yml` file. This command sets up a NAT on the router for the traffic going out from its `eth0` interface, **EXCEPT** for the packets to `10.8.0.0/24`. With this rule, for packets going out to the Internet, their source IP address will be replaced by the router's IP address `10.8.0.11`. Packets going to `10.8.0.0/24` will not go through NAT
    - `iptables -t nat -A POSTROUTING ! -d 10.8.0.0/24 -j MASQUERADE -o eth0`
    - **important**: `eth0` is the interface for `10.8.0.0/24`, if not, we need to modify the commands in `docker-compose.yml` and restart the containers
- *router configuration: firewall rules*:
    ```
    # Ingress filtering: only allows SSH traffic (dport 22)
    iptables -A FORWARD -i eth0 -p tcp -m conntrack \
             --ctstate ESTABLISHED,RELATED -j ACCEPT
    iptables -A FORWARD -i eth0 -p tcp --dport 22 -j ACCEPT
    iptables -A FORWARD -i eth0 -p tcp -j DROP

    # Egress filtering: block www.example.com
    iptables -A FORWARD -i eth1 -d 93.184.216.0/24 -j DROP
    ```
- block two more websites and add the filewall rules to the setup files
    ```
    # seedsecuritylabs.org
    iptables -A FORWARD -i eth1 -d 185.199.108.0/22 -j DROP
    # www.ed.ac.uk
    iptables -A FORWARD -i eth1 -d 23.185.0.1 -j DROP
    ```
    - test: ping any of the 2 websites on `192.168.20.5`, unable to get response

## Task 1: Static Port Forwarding
- the firewall in the lab setup prevents outside machines from connecting to any TCP server on the internal network, other than the SSH server
- use static port forwarding to evade this restriction. Specifically, we will use `ssh` to create a static port forwarding tunnel between host A (on the external network) and host B (on the internal network), so whatever data received on A's port X will be sent to B, from where the data is forwarded to the target T's port Y. The command to create such a tunnel
    ```
    $ ssh -4NT -L <A_IP>:<A_PORT>:<T_IP>:<T_PORT> <UID>@<B_IP>
    # -4: use IPv4 only
    # -N: do not execute a remote command
    # -T: disable pseudo-terminal allocation (save resources)
    ```
- regarding A's IP, typically we use `0.0.0.0`, indicating that our port forwarding will listen to the connection from all the interfaces on A. If we want to limit the connection from a particular interface, we should use that interface's IP address, e.g., using `127.0.0.1` or omitting the address (the default is `127.0.0.1`) will only allow programs on the local host to use this port forwarding

- *task*: use static port forwarding to create a tunnel between the external network and the inernal network, so we can telnet into the server on B1, specifically, we can do such telnet from hosts A, A1, and A2. On A (`10.8.0.99`)
    ```
    $ ssh -4NT -L 10.8.0.99:9090:192.168.20.5:23 seed@192.168.20.99
    ```
    - on host A1 (`10.8.0.5`)
        ```
        $ telnet 10.8.0.99 9090
        Trying 10.8.0.99...
        Connected to 10.8.0.99.
        a58f05db2fa0 login: <---- this is the container id of B1
        ```
- *questions*
    1. How many TCP connections are involved in this entire process (use `Wireshark`)
        - example: A2 telnet to B1 through the `ssh` tunnel
        1. a TCP connection created between A and B (through `ssh`, at the beginning)
        2. a TCP connection created between A1 and A (if we use A to telnet into B1, this connection will not be established)
        3. a TCP connection created between B and B1
    2. Why can this tunnel successfully help users evade the firewall rule specified in the lab setup
        - the lab setup disables TCP connections from external to internal hosts, except SSH connection. In the SSH tunnel, A connects to B through `ssh`, and when `telnet 10.8.0.99 9090` on external hosts, e.g., A1
            1. A1 communicates with A (external -> external)
            2. A communicates with B through the `ssh` tunnel (external -> internal, but through `ssh`)
            3. B communicates with the target B1 (internal -> internal)
            - none of these communications will be disallowed by the firewall rule

## Task 2: Dynamic Port Forwarding
- in the static port forwarding, each port-forwarding tunnel forwards the data to a particular destination. If we want to forward data to multiple destinations, we need to set up multiple tunnels. For example, if the router blocked many websites, to visit them, we need an SSH tunnel for each of them. Dynamic port forwarding can solve this problem
- block two more websites using the firewall rules [finished in task `0`]

### Task 2.1: Setting Up Dynamic Port Forwarding
- use `ssh` to create a dynamic port-forwarding tunnel between B and A. Run the following command on host B. In dynamic port forwarding, B is often called proxy
    - `$ ssh -4NT -D <B_IP>:<B_PORT> <UID>@<A_IP>`
- regarding B's IP, `0.0.0.0` indicate that our port forwarding will listen to the connection from all the interfaces on B. After the tunnel is set up, use `curl` to test it. We specify a proxy option, so `curl` will send its HTTP request to the proxy B, which listen on port X. The proxy forwards the data received on this port to the other end of the tunnel (host A), from where the data will be further forwarded to the target website. The type of proxy is called SOCKS version 5 (`socks5h`)
    - `$ curl --proxy socks5h://<B_IP>:<B_PORT> <BLOCKED_URL>`
- *task*: create a tunnel, ensure that we can visit all the blocked websites using `curl` from hosts B, B1, and B2
    - on host B: `$ ssh -4NT -D 0.0.0.0:9090 seed@10.8.0.99`
    - on host B, B1, or B2: `curl --proxy socks5h://192.168.20.99:9090 85.199.108.153`
- *questions*
    1. which computer establishes the actual connection with the intended web server
        - host A `10.8.0.99`
    2. how does this computer know which server it should connect to
        - if we look at Wireshark, before host A initiates a TCP connection with the server, there is an SSH encrypted packet from host B to A, this packet contains the server information
        - before that, we can find a packet from B1 to B, the payload of this packet contains `85.199.108.153`, i.e., the server's IP address
### Task 2.2: Testing the Tunnel Using Browser
- the host VM's IP address on the internal network `192.168.20.0/24` is `192.168.20.1`
- to use dynamic forwarding, we need to configure Firefox's proxy setting: type `about:preferences` in the URL field, on `General` page, click `Network Settings`, then click `Settings` button, choose manual proxy configuration, and fill in B's IP and port in the `SOCKS` field, choose `SOCKS v5`
- *task*: to test whether the traffic goes through the SSH tunnel, use `Wireshark` on the router, and point out the traffic involved in the entire port forwarding process.
    - `192.168.20.1` will initiate a TCP connection to B, then B will send an SSH encrypted packet to host A, and host A will initiate a TCP connection with the server
    - break the SSH tunnel, and try to browse a website: `Wireshark` will not capture any packets on network interfaces for `10.8.0.0/24` and `192.168.20.0/24`, because the VM doesn't send packets through these 2 interfaces

### Task 2.3: Writing a SOCKS Client Using Python
- for port forwarding to work, we need to specify where the data should be forwarded to (the final destination). In the static case, this piece of information is provided when setting up the tunnel. In the dynamic case, it is not specified during the setup, applications using a dynamic port forwarding proxy must tell the proxy where to forward their data. This is done through an additional protocol between the application and the proxy. A common protocol for this is the *SOCKS (SOCK Secure)* protocol, which becomes a de facto proxy standard
- the application software must have a native SOCKS support in order to use SOCKS proxies. Both Firefox and `curl` have such a support, but we cannot directly use this type of proxy for the telnet program, because it doesn't provide a native SOCKS support
- implement a simple SOCKS client program using Python
    ```
    #!/bin/env python3
    import socks

    hostname = 'www.example.com'
    s = socks.socksocket()
    s.set_proxy(socks.SOCKS, '192.168.20.99', 9090)
    s.connect((hostname, 80))
    
    req = b'GET / HTTP/1.0\r\nHOST: ' + hostname.encode('utf-8') + b'\r\n\r\n'
    s.sendall(req)
    response = s.recv(2048)
    while response:
        print(response.split(b'\r\n'))
        respons = s.recv(2048)
    ```
- create the dynamic port forwarding tunnel on B, then run this code on B, B1, and B2, we can get the content of `www.example.com`

## Task 3: Virtual Private Network (VPN)
- VPN is often used to bypass firewall. In this task, we will use VPN to bypass ingress and egress firewalls. OpenVPN is a powerful tool that we can use, but in this task, we simply use SSH, which is often called the poor man's VPN
- change some default SSH settings on the server to allow VPN creation, the changes made in `/etc/ssh/sshd_config` are listed in the following (already enabled inside the containers, so no action needed)
    ```
    PermitRootLogin yes
    PermitTunnel    yes
    ```
### Task 3.1: Bypassing Ingress Firewall
- to create a VPN tunnel from a client to a server, we run the following `ssh` command, it creates a TUN interface `tun0` on the VPN client and server machines, and then connect these two TUN interface using an encrypted TCP connection. Both zeros in option `0:0` means `tun0`. Detailed explanation of the `-w` option can be found in the manual of SSH
    - `# ssh -w 0:0 root@<VPN_SERVER_IP>`
- creating TUN interfaces requires the root privilege, so we need to have the root privilege on both ends of the tunnel. That's why we run it inside the root account, and also SSH into the root account on the server. The above command only creates a tunnel, further configuration is needed on both ends of the tunnel. The following improved command include some of the configuration commands
    ```
    # ssh -w 0:0 root@<VPN_SERVER_IP> \
          -o "PermitLocalCommand=yes" \
          -o "LocalCommand= ip addr add 192.168.53.88/24 dev tun0 && \
                            ip link set tun0 up" \
          -o "RemoteCommand=ip addr add 192.168.53.99/24 dev tun0 && \
                            ip link set tun0 up"
    root@<VPN_SERVER_IP> password: <-- dees
    ```
- the `LocalCommand` entry specifies the command running on the client side. It configures the client-side TUN interface: assigning the `192.168.53.88/24` address to the interface and bringing it up. The `RemoteCommand` entry specifies the command running on the VPN server, it configures the server-side TUN interface. The configuration is incomplete, and further configuration is still needed
- *task*: create a VPN tunnel between A and B, with B being the VPN server. Then conduct all the necessary configuration. Then telnet to B, B1, and B2 from the external network. Capture the packet trace, and explain why the packets are not blocked by the firewall
- use the above `ssh` command
- then, we need to configure the routing rule on host A, A1, and B1 (this will allow A or A1 telnet to B1)
    - on host A: `ip route add 192.168.20.5 via 192.168.53.99 dev tun0`, on A, packets to B1 should be sent via the VPN tunnel
    - on host A1: `ip route add 192.168.20.5 via 10.8.0.99 dev eth0`, on A1, packets to B1 to be routed by A, not the router
    - on host B1: `ip route add 192.168.53.88 via 192.168.20.99 dev eth0`, after A sends a packet through the tunnel, and the packet reaches the other side, the source IP will become `192.168.53.88`, to ensure that the response packets from B1 can also be sent through the tunnel, these packets should first be sent to B


### Task 3.2: Bypassing Egress Firewall
- bypass egree firewall using VPN
- in the lab setup, hosts on `192.168.20.0/24` cannot access the blocked 3 websites, the objective of this task is to use the VPN tunneling technique to bypass these rules
- use B as the VPN client and A as the VPN server
- it should be noted that when a packet generated on the VPN client is sent to the VPN server via the tunnel, the source IP address of the packet will be `192.168.53.88` according to our setup, when this packet goes out, it will go through VirtualBox's NAT server, where the source IP address will be replaced by the IP address of the host computer. The packet will eventually arrive at `www.example.com`, and the reply packet will come back to our host computer, and then be given to the same NAT server, where the destination address is translated back to `192.168.53.88`, this is where the problem comes up
- VB's NAT server knows nothing about the `192.168.53.0/24` network, because this is the one that we create internally for our TUN interface, and VB has no idea how to route to this network, much less knowing that the packet should be given to VPN server. As a result, the reply packet from `www.example.com` will be dropped (this situation is similar to `3.1`, the host B1 will send response packets to `192.168.53.88`, but it doesn't know the `192.168.53.0/24` network, we solve that problem by adding a routing rule to make B1 send those packets to B, the VPN server who knows the `192.168.53.0/24` network)
- *solution*: set up our own NAT server on VPN server, so when packets from `192.168.53.88` go out, their source IP  addresses are always replaced by the VPN server A's IP address (`10.8.0.99`). We can use the following command to create a NAT server on the `eth0` interface of the VPN server: `iptables -t nat -A POSTROUTING -j MASQUERADE -o eth0`

- *task*: set up a VPN tunnel between B and A, with A being the VPN server. Use this VPN to successfully reach the blocked websites from host B, B1, and B2
    ```
    # ssh -w 0:0 root@10.8.0.99 \
          -o "PermitLocalCommand=yes" \
          -o "LocalCommand= ip addr add 192.168.53.99/24 dev tun0 && \
                            ip link set tun0 up" \
          -o "RemoteCommand=ip addr add 192.168.53.88/24 dev tun0 && \
                            ip link set tun0 up"
    ```
- then, we also need to configure some routing rules
    - suppose we are trying to visit `www.example.com` from B1
    - on B1, send the packet to B, the VPN client: `ip route add 93.184.216.0/24 via 192.168.20.99 dev eth0`
    - on B, send the packet to the tunnel `ip route add 93.184.216.0/24 via 192.168.53.88 dev tun0`
    - on A, send the website's response packets through the tunnel `ip route add 192.168.20.5 via 192.168.53.99 dev tun0`
    - to solve the NAT problem, on A: `iptables -t nat -A POSTROUTING -j MASQUERADE -o eth0`
## Task 4: Comparing SOCKS5 Proxy and VPN
- SOCKS5 is more simple and straightforward to implement and use
- Using VPN, we also need to configure the routing rules
