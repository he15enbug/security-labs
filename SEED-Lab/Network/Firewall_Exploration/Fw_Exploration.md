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
- LKM allows us to add a new module to the kernel at the runtime. This new module enables us to extend the functionalities of the kernel, without rebuilding the kernel or even rebooting the computer
- get familiar with LKM: the following is a simple LKM that prints out `"Hello World!"` when the module is loaded. When the module is removed from kernel, it prints out `"Bye-byte World!"`. The messages are not printed out on the screen, but printed into the `/var/log/syslog` file, use `dmesg` to view the messages
- `hello.c`
    ```
    #include <linux/module.h>
    #include <linux/kernel.h>

    int initialization(void) {
        printk(KERN_INFO "Hello World!\n");
        return 0;
    }
    void cleanup(void) {
        printk(KERN_INFO "Bye-bye World!\n");
    }
    module_init(initialization);
    module_exit(cleanup);
    ```
- create `Makefile`, and type `make` to compile the above code into a loadable kernel module (in `Makefile`, indentation must be done with `Tab`, not spaces)
    ```
    obj-m += hello.o
    all:
        make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
    clean:
        make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
    ```
- the generated kernel module is `hello.ko`, use the following commands to load the module, list all modules, remove the module, and show information about a module
    ```
    $ sudo insmod hello.ko <-- inserting a module
    $ lsmod | grep hello   <-- list a module
    $ sudo rmmod hello     <-- remove the module
    $ modinfo hello.ko     <-- show information of a module
    $ dmesg                <-- check the messages
    ```
- test
    ```
    $ sudo insmod hello.ko
    $ sudo rmmod hello
    $ dmesg
    ...
    [499257.298256] Hello World!
    [499263.248654] Bye-bye World!
    ```
### Task 1.B: Implement a Simple Firewall Using Netfilter
- we will write our packet filtering program as an LKM, and then insert into the packet processing path inside the kernel. This cannot be easily done in the past before the `netfilter` was introduced into the Linux
- `Netfilter` is designed to facilitate the manipulation of packets by authorized users. It achieves this goal by implementing a number of hooks in the Linux kernel. These hooks are inserted into various places, including the packet incoming and outgoing paths. To manipulate the incoming packets, we simply need to connect our own programs (within LKM) to the correcsponding hooks. Once an incoming packet arrives, our program will be invoked. Our program can decide whether this packet should be blocked or not, and we can also modify the packets in the program

- in this task, use LKM and `Netfilter` to implement a packet filtering module. This module will fetch the firewall policies from a data structure, and use the policies to decide whether packets should be blocked or not

- *Hooking to Netfilter*: we need to hook our functions (in the kernel module) to the corresponding `netfilter` hooks. An example:
    ```
    # Register hook functions to netfilter
    static struct nf_hook_ops hook1, hook2;

    int registerFilter(void) {
        printk(KERN_INFO "Registering filters.\n");
        // Hook 1
        hook1.hook     = printInfo;
        hook1.hooknum  = NF_INET_LOCAL_IN;
        hook1.pf       = PF_INET;
        hook1.priority = NF_IP_PRI_FIRST;
        // use nf_register_hook(&hook) for Ubuntu 16.04 VM
        nf_register_net_hook(&init_net, &hook1);

        // Hook 2
        hook2.hook     = blockUDP;
        hook2.hooknum  = NF_INET_POST_ROUTING;
        hook2.pf       = PF_INET;
        hook2.priority = NF_IP_PRI_FIRST;
        nf_register_net_hook(&init_net, &hook2);

        return 0;
    }

    void removeFilter(void) {
        printk(KERN_INFO "The filters are being removed.\n");
        // use nf_unregister_hook(&hook) for Ubuntu 16.04 VM
        nf_unregister_net_hook(&init_net, &hook1);
        nf_unregister_net_hook(&init_net, &hook2);
    }

    module_init(registerFilter);
    module_exit(removeFilter)
    ```
- To register a hook, we need to prepare a hook data structure (`struct nf_hook_ops`), and set all the needed parameters. The most important of which are a function name and a hook number, the hook number is one of the 5 hooks in `netfilter`, and the specified function will be invoked when a packet has reached this hook. In the example, when a packet gets to the `LOCAL_IN` hooks, the function `printInfo()` will be invoked. Once the hook data structure is prepared, we attach the hook to `netfilter` (`nf_register_net_hook()`)
- *Hook functions*: an example of hook function is given below, it only prints out the packet information. When `netfilter` invokes a hook function, it passes 3 arguments to the function, including a pointer to the actual packet `skb`
    ```
    unsigned int printInfo(void *priv, struct sk_buff *skb, const struct nf_hook_state *state) {
        struct iphdr *iph;
        char *hook;

        switch(state->hook) {
            case NF_INET_LOCAL_IN:
                printk("*** LOCAL_IN");
                break;
            // omitted
        }

        iph = ip_hdr(skb);
        printk("%pI4 --> %pI4\n", &(iph->saddr), &(iph->daddr));
        return NF_ACCEPT;
    }
    ```
- we can use the following functions defined in various header files to get the headers of other protocols. The definition of these structures can be found inside `/lib/modules/$(shell uname -r)/build/include/uapi/linux` folder
    ```
    struct iphdr   *iph   = ip_hdr(skb)     // #include<linux/ip.h>
    struct tcphdr  *tcph  = tcp_hdr(skb)    // #include<linux/tcp.h>
    struct udphdr  *udph  = udp_hdr(skb)    // #include<linux/udp.h>
    struct icmphdr *icmph = icmp_hdr(skb)   // #include<linux/icmp.h>
    ```

- *Blocking packets*: an example to block a packet if it satisfies the specified condition. The example blocks the UDP packets if their destination IP is `8.8.8.8` and the destination port is `53`, i.e., the DNS query to nameserver `8.8.8.8` will be blocked
    ```
    unsigned int printInfo(void *priv, struct sk_buff *skb, const struct nf_hook_state *state) {
        struct iphdr *iph;
        struct udphdr *udph;
        u32 ip_addr;
        char ip[16] = "8.8.8.8";
        // Convert the IPv4 address from dotted decimal to a 32-bit number
        in4_pton(ip, -1, (u8 *)&ip_addr, '\0', NULL);

        iph = ip_hdr(skb);
        if(iph->protocol == IPPROTO_UDP) {
            udph = udp_hdr(skb);
            if(iph->daddr == ip_addr && ntohs(udph->dest) == 53) {
                printk(KERN_DEBUG "****Dropping %pI4 (UDP), port %d\n",
                                    &(ip->daddr), port);
                return NF_DROP;
            }
        }
        return NF_ACCEPT;
    }
    ```
- *tasks*
    1. compile the sample code `seedFilter.c` using provided `Makefile`, load it into the kernel, and demonstrate that the firewall is working as expected, we can generate UDP packets to `8.8.8.8`: `dig @8.8.8.8 www.example.com`
        ```
        $ make
        ...
        $ make ins
        sudo dmesg -C
        sudo insmod seedFilter.ko
        $ lsmod | grep seed
        seedFilter    16384    0
        $ dig @8.8.8.8 www.example.com
        ...
        ;; connection timed out; no servers could be reached
        $ make rm
        sudo rmmod seedFilter
        $ dig @8.8.8.8 www.example.com
        ...
        ;; ANSWER SECTION:
        www.example.com    19275    IN    A    93.184.216.34
        ...
        ```
    2. Hook the `printInfo` function to all of the `netfilter` hooks
        - Macros of the hook numbers
            ```
            NF_INET_PRE_ROUTING
            NF_INET_LOCAL_IN
            NF_INET_FORWARD
            NF_INET_LOCAL_OUT
            NF_INET_POST_ROUTING
            ```
    3. Implement two more hooks to achieve the following. Implement 2 different hook functions, but register them to the same `netfilter` hook. We should decide what hook to use. Telnet's default port is TCP port 23. To test it, we can go to the `10.9.0.5` container and run the following commands: `ping 10.9.0.1` and `telnet 10.9.0.1 23`
        1. preventing other computers to ping the VM
            - filter out all ICMP echo request packets with destination IP `10.9.0.1` (the VM's IP address)
        2. preventing other computer to telnet into the VM
            - filter out all TCP packets with destination IP `10.9.0.1` and destination port `23`
        - in both cases, we are blocking incoming packets, we can use either `NF_INET_PRE_ROUTING` or `NF_INET_LOCAL_IN`, according to the printout information by `printInfo`, incomming packets will first reach `NF_INET_PRE_ROUTING`, so I used this hook to drop thoses packets at the first place

## Task 2: Experimenting with Stateless Firewall Rules
- Linux has a built-in firewall, called `iptables`, which is also based on `Netfilter`. Technically, the kernel part implementation of the firewall is called `Xtables`, while `iptables` is a user-space program to configure the firewall. However, `iptables` is often used to refer to both the kernel-part implementation and the user-space program

### Background of `iptables`
- the `iptables` firewall is designed not only to filter packets, but also make changes to packets
- to help manage these firewall rules for different purposes, `iptables` organizes all rules using a hierarchical structure: table, chain, and rules. There are several tables, each specifying the main purpose of the rules. For example, rules for packet filtering should be placed in `filter` table, while rules for making changes to packets should be placed in `nat` and `mangle` tables
- each table contains several chains, each of which corresponds to a `netfilter` hook. Rules on the `FORWARD` chain are enforced at the `NF_INET_FORWARD` hook, and rules on the `INPUT` chain are enforced at the `NET_INET_LOCAL_IN` hook
- each chain contains a set of firewall rules that will be enforced
    ```
    iptables Tables and Chains
    +------------------------------------------------+
    | Table   | Chain          | Functionality       |
    |---------+----------------+---------------------|
    | filter  | INPUT          | Packet filtering    |
    |         | FORWARD        |                     |
    |         | OUTPUT         |                     |
    |---------+----------------+---------------------|
    | nat     | PREROUTING     | Modifying source or |
    |         | INPUT          | destination network |
    |         | OUTPUT         | addresses           |
    |         | POSTROUTING    |                     |
    |---------+----------------+---------------------|
    | mangle  | PREROUTING     | Packet content      |
    |         | INPUT          | modification        |
    |         | FORWARD        |                     |
    |         | OUTPUT         |                     |
    |         | POSTROUTING    |                     |
    +------------------------------------------------+
    ```
### Using `iptables`
- manual of `iptables`: run `man iptables`
- we need to specify a table name (the default is `filter`), a chain name, and an operation on the chain, after that, we specify the rule, which is basically a pattern that will be matched with each of the packets passing through, if there is a match, an action will be performed on this packet. The general structure of the command is depicted in the following:
    ```
    iptables -t <table> -<operation> <chain> <rule> -j <target>
             ---------- -------------------- ------ -----------
                Table          Chain          Rule     Action
    ```
- the rule is the most complicated part of the `iptable` command. Some commonly used commands
    ```
    // List all the rules in a table
    iptables -t nat -L -n
    iptables -t filter -L -n --line-numbers

    // Delete rule No. 2 in the INPUT chain of the filter table
    iptables -t filter -D INPUT 2

    // Drop all the incoming packets that satisfy the <rule>
    iptables -t filter -A INPUT <rule> -j DROP
    ```
- *note*: Docker relies on `iptables` to manage the network it creates, so it adds many rules to the `nat` table. Be careful not to remove Docker rules

### Task 2.A: Protecting the Router
- set up rules to prevent outside machines from accessing the router machine, except ping
- run commands on router:
    ```
    iptables -A INPUT -p icmp --icmp-type echo-request -j ACCEPT
    iptables -A OUTPUT -p icmp --icmp-type echo-reply -j ACCEPT
    iptables -p OUTPUT DROP
    iptables -p INPUT DROP
    ```
- test on `10.9.0.5`
    1. can you ping the router (**Yes**)
    2. can you telnet into the router (**No**)
        ```
        # telnet 10.9.0.11 23
        Trying 10.0.0.11... <--- unable to connect
        ```
- cleanup
    ```
    iptables -F
    iptables -P OUTPUT ACCEPT
    iptables -P INPUT ACCEPT
    ```
### Task 2.B: Protecting the Internal Network
- set up firewall rules on the router to protect the internal network `192.168.60.0/24`
- the direction of packets in the `INPUT` and `OUTPUT` chains are clear, this is not true for `FORWARD` chain, which is bi-directional. To specify the direction, we can add the interface option using `-i xyz` (coming in from interface `xyz`) and `-o xyz` (going out from `xyz`), the interfaces for the interal and external networks are different, we can find out the interface names using `ip addr` or `ifconfig`
- objective: enforce the following restrictions on the ICMP traffic
    1. Outside hosts cannot ping internal hosts
    2. Outside hosts can ping the router
    3. Internal hosts can ping outside hosts
    4. All other packets between the interal and external networks should be blocked
- we can use `iptables -p icmp -h` to find out all the ICMP match options
- rules
    ```
    iptables -A INPUT -p icmp --icmp-type echo-request -j ACCEPT
    iptables -A OUTPUT -p icmp --icmp-type echo-reply -j ACCEPT

    # only allow ICMP request going out and ICMP reply coming in (eth1)
    iptables -A FORWARD -i eth1 -p icmp --icmp-type echo-request -j ACCEPT
    iptables -A FORWARD -o eth1 -p icmp --icmp-type echo-reply -j ACCEPT

    iptables -P FORWARD DROP
    iptables -P OUTPUT DROP
    iptables -P INPUT DROP
    ```
- cleanup
    ```
    iptables -F
    iptables -P OUTPUT ACCEPT
    iptables -P INPUT ACCEPT
    ```
### Task 2.C: Protecting Internal Servers
- protect the TCP servers inside the internal network `192.168.60.0/24`, specifically, we need to achieve the following objectives
    1. All the interal hosts run a telnet server (listening to port `23`). Outside hosts can only access the telnet server on `192.168.60.5`, not the other internal hosts
    2. Outside hosts cannot access other interal servers
    3. Internal hosts can access all the internal servers
    4. Internal hosts cannot access external servers
    5. In this task, the connection tracking mechanism is not allowed
- use `-p tcp` for TCP protocol
- an example `iptables -A FORWARD -i eth0 -p tcp --sport 5000 -j ACCEPT`
- rules
    ```
    iptables -A FORWARD -s 192.168.60.5 -i eth1 -p tcp --sport 23 -j ACCEPT
    iptables -A FORWARD -d 192.168.60.5 -o eth1 -p tcp --dport 23 -j ACCEPT
    iptables -P FORWARD DROP
    ```
- cleanup
## Task 3: Connection Tracking and Stateful Firewall

## Task 4: Limiting Network Traffic

## Task 5: Load Balancing
