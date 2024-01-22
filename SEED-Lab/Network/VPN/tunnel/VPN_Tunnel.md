# VPN Lab (Tunneling Part)
## Task 1: Network Setup
- create a VPN tunnel between a client and a gateway, allowing the client to securely access a private network via the gateway
- 3 machines
    1. Host U: VPN client - `10.9.0.5`
    2. Router: VPN server (the gateway) 
        - `10.9.0.11`
        - `192.168.60.11`
    3. Host V: a host in the private Network - `192.168.60.5`
    - The client can access the host in the private network (`192.168.60.0/24`) via the VPN server

- communication
    - Host U can communicate with VPN Server
    - Host V can communicate with VPN Server
    - Host U can NOT communicate with Host V 

## Task 2: Create and Configure TUN Interface
- [TUN/TAP](https://en.wikipedia.org/wiki/TUN/TAP): TUN and TAP are virtual network kernel drivers
    - TAP simulates an Ethernet device and operates with **data link layer (layer-2)** packets such as Ethernet frames
    - TUN simulates a network layer device and operates with **network layer (layer-3)** packets such as IP packets

- `tun.py` - Code to create a TUN interface
    ```
    #!/usr/bin/env python3

    import fcntl
    import struct
    import os
    import time
    from scapy.all import *

    TUNSETIFF = 0x400454ca
    IFF_TUN   = 0x0001
    IFF_TAP   = 0x0002
    IFF_NO_PI = 0x1000

    # Create the tun interface
    tun = os.open("/dev/net/tun", os.O_RDWR)
    ifr = struct.pack('16sH', b'he15enbug%d', IFF_TUN | IFF_NO_PI)
    ifname_bytes = fcntl.ioctl(tun, TUNSETIFF, ifr)

    # Get the interface name
    ifname = ifname_bytes.decode('UTF-8'[:16].strip("\x00"))
    print("Interface Name: {}".format(ifname))

    while True:
        time.sleep(10)

    ```

### Task 2.a: Name of the Interface
- `chmod a+x tun.py`
- `./tun.py` (with root privilege)

- set the prefix of the name of the tun interface as `he15enbug`
    - `ifr = struct.pack('16sH', b'he15enbug%d', IFF_TUN | IFF_NO_PI)`

- test on terminal
    ```
    # ./tun.py
    Interface Name: he15enbug

    ```
    
    ```
    # ip address
    ...
    17: he15enbug0: <POINTOPOINT,MULTICAST,NOARP> mtu 1500 qdisc noop state DOWN group default qlen 500
        link/none 
    ...
    ```

### Task 2.b: Set up the TUN Interface
- 2 things to do
    1. assign an IP address to it
        - `ip addr add 192.168.53.99/24 dev he15enbug0`
    2. bring up the interface
        - `ip link set dev he15enbug0 up`
    - or do this in `tun.py` automatically
        - `os.system("ip addr add 192.168.53.99/24 dev {}".format(ifname))`
        - `os.system("ip link set dev {} up".format(ifname))`

- test on terminal
    ```
    # ip address
    ...
    2: he15enbug0: <POINTOPOINT,MULTICAST,NOARP,UP,LOWER_UP> mtu 1500 qdisc fq_codel state UNKNOWN group default qlen 500
        link/none 
        inet 192.168.53.99/24 scope global he15enbug0
            valid_lft forever preferred_lft forever
    ...
    ```

### Task 2.c: Read from the TUN Interface
- whatever comming out of the TUN interface is an IP packet, so we can cast the data received into a Scapy IP object
- modify the `while` loop in `tun.py`
    ```
    while True:
    # Get a packet from the tun interface
    packet = os.read(tun, 2048)
    if packet:
        ip = IP(packet)
        print(ip.summary())
    ```

- run `tun.py` on Host U

- test on terminal
    1. On Host U, ping a host in the `192.168.53.0/24` network
        ```
        # ping 192.168.53.100
        ```
        ```
        # ./tun.py
        Interface Name: he15enbug0
        IP / ICMP 192.168.53.99 > 192.168.53.100 echo-request 0 / Raw
        IP / ICMP 192.168.53.99 > 192.168.53.100 echo-request 0 / Raw
        IP / ICMP 192.168.53.99 > 192.168.53.100 echo-request 0 / Raw
        ...
        ```

    2. On Host U, ping a host in the `192.168.60.0/24` network
        ```
        # ping 192.168.60.100
        ```
        ```
        # ./tun.py
        Interface Name: he15enbug0
        
        (NOTHING PRINTED)
        ```

### Task 2.d: Write to the TUN Interface
- whatever is written to the interface by the application will appear in the kernel as an IP packet
- modify `tun.py`, after getting a packet from the TUN interface, construct a new packet based on the received packet, and write it back to the TUN interface
    ```
    while True:
    # Get a packet from the tun interface
    packet = os.read(tun, 2048)
    if packet:
        ip = IP(packet)
        print(ip.summary())

        # Send out a spoof packet using the tun interface
        newip = IP(src=ip.dst, dst=ip.src)
        newpkt = newip/ip.payload
        os.write(tun, bytes(newpkt))
    ```
- test on terminal
    ```
    # ./tun.py
    Interface Name: he15enbug0
    IP / ICMP 192.168.53.99 > 192.168.53.100 echo-request 0 / Raw
    IP / ICMP 192.168.53.99 > 192.168.53.100 echo-reply 0 / Raw
    IP / ICMP 192.168.53.99 > 192.168.53.100 echo-request 0 / Raw
    IP / ICMP 192.168.53.99 > 192.168.53.100 echo-reply 0 / Raw
    IP / ICMP 192.168.53.99 > 192.168.53.100 echo-request 0 / Raw
    IP / ICMP 192.168.53.99 > 192.168.53.100 echo-reply 0 / Raw
    ```
    ```
    # ping 192.168.53.100
    PING 192.168.53.100 (192.168.53.100) 56(84) bytes of data.
    64 bytes from 192.168.53.100: icmp_seq=1 ttl=64 time=5.81ms
    64 bytes from 192.168.53.100: icmp_seq=2 ttl=64 time=1.81ms
    64 bytes from 192.168.53.100: icmp_seq=3 ttl=64 time=1.69ms
    ```