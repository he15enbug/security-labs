# Firewall Exploration Lab
- learning how firewalls work, and setting up a simple firewall for a network
- implement a simple stateless packet-filtering, which inspect packets, and decides whether to drop or forward a packet based on firewall rules
- Linux already has a built-in firewall, also based on `netfilter`, this firewall is called `iptables`
- topics
    - firewall
    - netfilter
    - loadable kernel module
    - using `iptables` to set up firewall rules
    - various applications of `iptables`

## Lab Setup
- network `10.9.0.0/24`
    - attacker `10.9.0.1`
    - `10.9.0.5`
    - router `10.9.0.11`
- network `192.168.60.0/24`
    - router `192.168.60.11`
    - `192.168.60.5`
    - `192.168.60.6`
    - `192.168.60.7`

## Task 1: Implementing a Simple Firewall
- implement a simple packet filtering type of firewall, which inspecting the incoming and outgoing packets, and enforces the firewall policies set by the administrator
- since the packet processing is done within the kernal, the filtering must also be done within the kernel. In the past, this had to be done by modifying and rebuilding the kernel, the modern Linux OS provide several new mechanisms to facilitate the manipulation of packets without rebuilding the kernel image, these 2 mechanisms are *Loadable Kernel Module (LKM)* and `Netfilter`
- *notes about containers*:
    - since all containers share the same kernel, kernel modules are global, if we set a kernel module from a container, it affects all the container and the host. For this reason, it does not matter where we set the kernel module, and we will just set the kernel module from the host VM
    - containers' IP addresses are virtual, packets going to these virtual IP addresses may not traverse the same path as what is described in the Netfilter document. Therefore, in this task, to avoid confusion, we will try to avoid using those virtual addresses. We do most tasks on the host VM, the containers are mainly for other tasks

### Task 1.A: Implement a Simple Kernel Module

### Task 1.B: Implement a Simple Firewall Using Netfilter

## Task 2: Experimenting with Stateless Firewall Rules

## Task 3: Connection Tracking and Stateful Firewall

## Task 4: Limiting Network Traffic

## Task 5: Load Balancing
