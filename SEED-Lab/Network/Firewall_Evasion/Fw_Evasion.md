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
- in the static port forwarding, each port-forwarding tunnel forwards the data to a particular destination. If we want to forward data to multiple destinations, we need to set up multiple tunnels. Dynamic port forwarding can solve this problem
- block two more websites using the firewall rules [finished in task `0`]

### Task 2.1: Setting Up Dynamic Port Forwarding

### Task 2.2: Testing the Tunnel Using Browser

## Task 3: Virtual Private Network (VPN)

## Task 4: Comparing SOCKS5 Proxy and VPN
