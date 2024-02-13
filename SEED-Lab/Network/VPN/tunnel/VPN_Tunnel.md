# VPN Lab (Tunneling Part)
## Task 1: Network Setup
- create a VPN tunnel between a client and a gateway, allowing the client to securely access a private network via the gateway
- the client can access the host in the private network (`192.168.60.0/24`) via the VPN server
- network `10.9.0.0/24`
    - user (host U, VPN client) `10.9.0.5`
    - router (VPN server) `10.9.0.11`
- network `192.168.60.0/24`
    - router (VPN server) `192.168.60.11`
    - host V `192.168.60.5`
    - `192.168.60.6`
    - `192.168.60.7`

- test
    - host U can communicate with VPN Server
    - host V can communicate with VPN Server
    - host U can NOT communicate with Host V (because on host U's route table, there is no entry for `192.168.60.0/24`, and by default it will send packets to `10.9.0.1`, which cannot communicate with hosts on `192.168.60.0/24`)
    - run `tcpdump` on the router, and sniff the traffic on each of the network, to ensure we can capture packets

## Task 2: Create and Configure TUN Interface
- [TUN/TAP](https://en.wikipedia.org/wiki/TUN/TAP): TUN and TAP are virtual network kernel drivers
    - TAP simulates an Ethernet device and operates with **data link layer (layer-2)** packets such as Ethernet frames
    - TUN simulates a network layer device and operates with **network layer (layer-3)** packets such as IP packets

- `tun.py` - code to create a TUN interface
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
    1. On Host U, ping a host in the `192.168.53.0/24` network, the tunnel interface can see these ICMP request packets, but there won't be response, because the host `192.168.53.100` doesn't exist
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
    2. On Host U, ping a host in the `192.168.60.0/24` network, no response, because we haven't configure the VPN server yet
        ```
        # ping 192.168.60.100
        ```
        ```
        # ./tun.py
        Interface Name: he15enbug0
        (NOTHING PRINTED)
        ```

### Task 2.d: Write to the TUN Interface
- since the TUN interface is a virtual network interface, whatever is written to the interface by the application will appear in the kernel as an IP packet
- modify `tun.py`, after getting a packet from the TUN interface, construct a new packet based on the received packet, and write it back to the TUN interface. Specifically, for each ICMP request packet, send a reply packet
    ```
    while True:
    # Get a packet from the tun interface
    packet = os.read(tun, 2048)
    if packet:
        ip = IP(packet)
        print(ip.summary())

        # Send out a spoofed ICMP echo reply packet using the tun interface
        newip = IP(src=ip.dst, dst=ip.src)
        newicmp = ip.payload
        newicmp.type = 0 # 0: echo-reply
        newicmp.chksum = None # clear the chksum, Scapy will recalculate it
        newpkt = newip/newicmp
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
- test on terminal
    ```
    # ./tun.py
    Interface Name: he15enbug0
    IP / ICMP 192.168.53.99 > 192.168.53.100 echo-request 0 / Raw
    IP / ICMP 192.168.53.99 > 192.168.53.100 echo-request 0 / Raw
    IP / ICMP 192.168.53.99 > 192.168.53.100 echo-request 0 / Raw
    ```
    ```
    # ping 192.168.53.100
    PING 192.168.53.100 (192.168.53.100) 56(84) bytes of data.
    64 bytes from 192.168.53.100: icmp_seq=1 ttl=64 time=1.56ms
    64 bytes from 192.168.53.100: icmp_seq=2 ttl=64 time=1.34ms
    64 bytes from 192.168.53.100: icmp_seq=3 ttl=64 time=1.46ms
    ```
- instead of writing IP packets to the tunnel, write arbitrary data
    ```
    data = b'hello world'
    os.write(tun, bytes(data))
    ```
    - the raw data will be send without any processing, we can use `tcpdump -i he15enbug0 -X` on host U to display the hex data
        ```
        # tcpdump -i he15enbug0 -X
        ...
        12:47:23.210526 [|ip6]
            0x0000:  6865 6c6c 6f20 776f 726c 64              hello.world
        ...
        12:47:24.235558 [|ip6]
            0x0000:  6865 6c6c 6f20 776f 726c 64              hello.world
        ```

## Task 3: Send the IP Packet to VPN Server Through a Tunnel
- put the packet from TUN interface into the UDP payload field of a new IP packet
- send the new packet to another machine
- placing the original packet inside a new packet is called **IP tunneling**, the tunnel can be built on top of TCP or UDP

- run `tun_server.py` on VPN Server
    ```
    #!/usr/bin/env python3

    from scapy.all import *

    # '0.0.0.0' indicating that the server listens to packets from all interfaces
    IP_A = '0.0.0.0'
    PORT = 9090

    # socket.AF_INET: specifies the address family for IPv4
    # socket.SOCK_DGRAM: the socket will be a UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((IP_A, PORT))

    while True:
        data, (ip, port) = sock.recvfrom(2048)
        print("{}:{} --> {}:{}".format(ip, port, IP_A, PORT))

        # convert the data into an IP object
        pkt = IP(data)
        print("    Inside: {} -> {}".format(pkt.src, pkt.dst))
    ```

- run `tun_client.py` on the VPN Client (Host U)
    - replace the `while` loop in `tun.py`
    ```
    # Create UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    while True:
        # Get a packet from the tun interface
        packet = os.read(tun, 2048)
        if packet:
            # Send the packet via the tunnel
            sock.sendto(packet, (SERVER_IP, SERVER_PORT))
    ```

- On Host U, i.e., the VPN Client: `ping 192.168.53.100`. The ICMP packet first get to `tun_client` on U, and then the program `tun_client.py` reads this packet, and send it to the server using socket, the server gets the packet from the socket, and write it to `tun_server` interface, if `ping 192.168.53.100` existed, the packet would get to it
    ```
    (VPN Server)
    # ./tun_server.py
    10.9.0.5:35657 --> 0.0.0.0:9090
        Inside: 192.168.53.99 --> 192.168.53.100
    10.9.0.5:35657 --> 0.0.0.0:9090
        Inside: 192.168.53.99 --> 192.168.53.100
    ...
    ```
    - data flow (ICMP request packet `192.168.53.99` -> `192.168.53.100`):
        - ping program ----> tun interface
        - `tun_client` ----> `tun_client.py`
        - `tun_client.py` --Socket--> `tun_server.py`
        - `tun_server.py` ----> `tun_server`

- Our ultimate goal is to access the hosts inside the private network `192.168.60.0/24`, when we ping `192.168.60.5` on host U, we want the packet to be routed to the tunnel, configure the route table (we can use either of the two commands):
    - `ip route add 192.168.60.0/24 via 192.168.53.99 dev tun_client`: on U, the ICMP will be routed to `192.168.53.99`, since `tun_client0 (192.168.53.88)` is on the same LAN with `192.168.53.99`, so the packet will be routed to `tun_client0` first
    - `ip route add 192.168.60.0/24 via 192.168.53.88 dev tun_client`: on U, the ICMP will be directly routed to `tun_client0 (192.168.53.88)`
    - on Host U, `ping 192.168.60.5`, the ICMP packet will go through the `tun_client0` interface, we can observe packets information on both `tun_client.py` and `tun_server.py`

## Task 4: Set Up the VPN Server
- `tun_server.py`: forward the packet to its destination, this needs to be done through a TUN interface
- subtasks (modify `tun_server.py`):
    1. create a TUN interface
    2. get data from the socket interface
    3. write the packet to the TUN interface
    ```
    (just copy the code in tun.py)
    ```
- configure VPN Server as a gateway (enable the IP forwarding)
    - set `net.ipv4.ip_forward=1` in `/etc/sysctl.conf`, active the changes `sudo sysctl -p`

- run `tcpdump -i eth0` on Host V (`192.168.60.5`), it will receive ICMP packets from the tun interface (`tun_client0 (192.168.53.99)`) on Host U, we can observe packets `192.168.53.88 ---> 192.168.60.5 (V)` on `Wireshark`. We will also observe ICMP reply packets, but it will not get back to host U, because we haven't configured that yet

## Task 5: Handling Traffic in Both Directions
- In task 4, Host V can receive packets from Host U, but the reply will not get back to Host U
- By running `tcpdump -i eth1` we can see that the reply pakets got to the server (`eth1: 192.168.60.11/24`), but it was dropped somewhere

- Performance:
    - Non-Blocking IO: idle loop wastes CPU time
    - Blocking IO: block on one interface until data is ready
    - IO Multiplexing: block on all interfaces altogether, and get notified when some of them are ready
        1. [`select`](https://en.wikipedia.org/wiki/Select_(Unix))
        2. [`poll`](https://en.wikipedia.org/wiki/Poll_(Unix))
        3. [`epoll`](https://en.wikipedia.org/wiki/Epoll)
- In this lab, we are going to use `select`
- code on the server `tun_server2.py` (it is similar on the client):
    ```
    while True:
        # parameters: 1. readable, 2. writable, 3. exceptional, (4. timeout)
        ready, _, _ = select.select([sock, tun], [], [])

        for fd in ready:
            if fd is sock:
                data, (ip, port) = sock.recvfrom(2048)
                pkt = IP(data)
                print("From socket <==: {}:{} --> {}:{}".format(ip, port, SERVER_IP, SERVER_PORT))
                print("    Inside: {} --> {}".format(pkt.src, pkt.dst))
                os.write(tun, bytes(pkt))

            if(fd is tun):
                packet = os.read(tun, 2048)
                pkt = IP(data)
                print("From socket <==: {} --> {}".format(pkt.src, pkt.dst))

                sock.sendto(packet, (IP, PORT))
    ```
- first, run `tun_server2.py` and `tun_client2.py` on the VPN server and client (host U) respectively
- then, configure the routing rule on host U: `ip route add 192.168.60.0/24 via 192.168.53.88 dev tun_client0` (each time we restart `tun_client2.py`, the interface `tun_client0` will be removed and recreated, so we need to configure the routing rule)
- test: on host U, run `ping 192.168.60.5`
    ```
    // host U
    # ping 192.168.60.5
    PING 192.168.60.5 (192.168.60.5) 56(84) bytes of data.
    64 bytes from 192.168.60.5: icmp_seq=19 ttl=63 time=4.41 ms

    // client (host U)
    # ./tun_client2.py 
    Interface Name: tun_client0
    From tun    ==>: 192.168.53.88 --> 192.168.60.5
    From socket <==: 10.9.0.11:9090 --> 10.9.0.5:9090
        Inside: 192.168.60.5 --> 192.168.53.88
    // server
    # ./tun_server2.py 
    Interface Name: tun_server0
    From socket <==: 10.9.0.5:9090 --> 10.9.0.11:9090
        Inside: 192.168.53.88 --> 192.168.60.5
    From tun    ==>: 192.168.60.5 --> 192.168.53.88
    ```
- test `telnet`
    ```
    // host U
    # telnet 192.168.60.5 23
    Trying 192.168.60.5...
    Connected to 192.168.60.5.
    ...
    Password: <---- it works
    ```

## Task 6: Tunnel-Breaking
- on host U, `telnet` to Host V, while keeping the `telnet` connection alive, we break the VPN tunnel by stopping the client or server program, then type something in `telnet` window:
    1. do we see what we type? (**No**)
    2. is the TCP connection broken?
        - **No**, the TCP connection is still alive, because to close a TCP connection, one of the side needs to send a FIN+ACK packet or RST packet to close the connection. When we stop the VPN server program, none of the two sides knows about that
- reconnect: after reconnecting in a short time, we will find what we typed earlier has been displayed on the `telnet` window, in the case of stopping the server program, when host U sends the packet to the server, it will found that the destination is unreachable, and will keep trying. When we restart the server program, the packet will finally get to the destination and get response, so what we typed before is displayed

## Task 7: Routing Experiment on Host V
- in an real VPN system, the traffic will be encrypted, that means the return traffic must come back from the same tunnel. How to get the return traffic from host V to the VPN server is non-trivial. Our setup simplifies the situation. In our setup, host V's routing table has a default setting: packets to any destination except the `192.168.60.0/24` network, will be routed to the VPN server
- in the real world, host V may be a few hops away from the VPN server, and the default routing entry may not guarantee to route the return packet back to the VPN server. Routing tables inside a private network have to be set up properly to ensure that packets going to the other end of the tunnel will be routed to the VPN server
- to simulate this scenario, we will remove the default entry from host V, and add a more specific entry to the routing table, so the return packets can be routed back to the VPN server
- *solution*
    ```
    (configure the routing table of host V 192.168.60.5)
    # ip route del default
    (if we ping 192.168.60.5 on host U at this moment, we won't succeed)
    # ip route add 192.168.53.0/24 via 192.168.60.11 dev eth0
    (now, we can get response on host U)
    ```

## Task 8: VPN Between Private Networks
- in this task, we are setting up a VPN between two private networks, the whole setup is described in `docker-compose2.yml`, use `-f docker-compose2.yml` to ask `docker-compose` to use this file
    ```
    $ docker-compose -f docker-compose2.yml build
    $ docker-compose -f docker-compose2.yml up
    $ docker-compose -f docker-compose2.yml down
    ```
- *network setup*
    - network `10.9.0.0/24`
        - VPN client `10.9.0.12`
        - VPN server `10.9.0.11`
    - network `192.168.50.0/24`
        - VPN client `192.168.50.12`
        - host U `192.168.50.5`
    - network `192.168.60.0/24`
        - VPN server `192.168.60.11`
        - host V `192.168.60.5`
- this setup simulates a situation where an organization has two sites, each having a private network. The only way to connect these two networks is through the Internet. Our task is to setup a VPN between these two sites, so the communication between these two networks will go through a VPN tunnel
- we can use code developed earlier, but we need to correctly set up the routing:
    ```
    (client)
    # ip route add 192.168.60.0/24 via 192.168.53.88 dev tun_client0
    
    (host U)
    # ip route add 192.168.60.0/24 via 192.168.50.12 dev eth0
    # ip route add 192.168.53.0/24 via 192.168.50.12 dev eth0

    (server)
    # ip route add 192.168.50.0/24 via 192.168.53.99 dev tun_server0
    
    (host V)
    # ip route add 192.168.53.0/24 via 192.168.60.11 dev eth0
    # ip route add 192.168.50.0/24 via 192.168.60.11 dev eth0
    ```
- after configuring the routing table, we can ping each other on host U and V, both will be able to get the reply packets

## Task 9: Experiment with the TAP Interface
- a simple experiment with the TAP interface
- the way how TAP works is similar to TUN, the main difference is that the kernel end of the TUN interface is hooked to the IP layer, while the kernel end of the TAP interface is hooked to the MAC layer, therefore, packet going through the TUN interface includes the MAC header. Other than getting the frames containing IP packets, using the TAP interface, applications can also get other types of frames, such as ARP frames
- example program
    ```
    ...
    tap = os.open('/dev/net/tun, os.O_RDWR')
    ifr = struct.pack('16sH', b'tap%d', IFF_TAP | IFF_NO_PI) # use IFF_TAP
    ifname_bytes = fcntl.ioctl(tap, TUNSETIFF, ifr)
    ifname = ifname_bytes.decode('UTF-8')[:16].strip('\x00')
    ...
    while True:
        packet = os.read(tap, 2048)
        if packet:
            ether = Ether(packet)
            print(ether.summary())
    ```
- run this on VPN client, and ping any host on `192.168.53.0/24`, the program will not see those ICMP packets (because they are IP packets), instead, it will see the ARP packets send by the client to ask for the MAC address of `192.168.53.100`, and the ICMP echo request packets
    ```
    # ./tap.py 
    Interface Name: tap0
    Ether / ARP who has 192.168.53.100 says 192.168.53.99
    Ether / ARP who has 192.168.53.100 says 192.168.53.99
    Ether / IP / ICMP 192.168.53.99 > 192.168.53.100 echo-request 0 / Raw
    ```
- spoof fake reply
    ```
    FAKE_MAC = 'aa:bb:cc:dd:ee:ff'
    if ARP in ether and ether[ARP].op == 1:
        arp      = ether[ARP]
        newether = Ether(dst=ether.src, src=FAKE_MAC)
        # request: op=1, reply: op=2
        newarp   = ARP(psrc=arp.pdst, hwsrc=FAKE_MAC, pdst=arp.psrc, hwdst=ether.src, op=2)
        newpkt   = newether/newarp

        print('***** Fake response: {} *****'.format(newpkt.summary()))
        os.write(tap, bytes(newpkt))
    ```
- use `arping -I tap0 192.168.53.100` to test it, we can get fake responses
    ```
    # arping -I tap0 192.168.53.100
    ARPING 192.168.53.100
    42 bytes from aa:bb:cc:dd:ee:ff (192.168.53.100): index=0 time=1.229 msec
    42 bytes from aa:bb:cc:dd:ee:ff (192.168.53.100): index=1 time=1.161 msec
    42 bytes from aa:bb:cc:dd:ee:ff (192.168.53.100): index=2 time=1.825 msec
    ```
    ```
    # ./tap.py 
    Interface Name: tap0
    ----------------------
    Ether / ARP who has 192.168.53.100 says 192.168.53.99 / Padding
    ***** Fake response: Ether / ARP is at aa:bb:cc:dd:ee:ff says 192.168.53.100 *****
    ----------------------
    Ether / ARP who has 192.168.53.100 says 192.168.53.99 / Padding
    ***** Fake response: Ether / ARP is at aa:bb:cc:dd:ee:ff says 192.168.53.100 *****
    ----------------------
    Ether / ARP who has 192.168.53.100 says 192.168.53.99 / Padding
    ***** Fake response: Ether / ARP is at aa:bb:cc:dd:ee:ff says 192.168.53.100 *****
    ```
