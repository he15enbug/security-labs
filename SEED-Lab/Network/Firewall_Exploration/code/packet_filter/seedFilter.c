#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/icmp.h>
#include <linux/if_ether.h>
#include <linux/inet.h>


static struct nf_hook_ops hook1, hook2;
// add another 4 types of hooks to hook the printInfo function
static struct nf_hook_ops hooks[4];
int i;

// hooks for blockPing and blockTelnet
static struct nf_hook_ops hook_ping, hook_telnet;

unsigned int blockUDP(void *priv, struct sk_buff *skb, const struct nf_hook_state *state) {
    struct iphdr *iph;
    struct udphdr *udph;

    u16  port   = 53;
    char ip[16] = "8.8.8.8";
    u32  ip_addr;

    if (!skb) return NF_ACCEPT;

    iph = ip_hdr(skb);
    // Convert the IPv4 address from dotted decimal to 32-bit binary
    in4_pton(ip, -1, (u8 *)&ip_addr, '\0', NULL);

    if (iph->protocol == IPPROTO_UDP) {
        udph = udp_hdr(skb);
        if (iph->daddr == ip_addr && ntohs(udph->dest) == port) {
            printk(KERN_WARNING "*** Dropping %pI4 (UDP), port %d\n", &(iph->daddr), port);
            return NF_DROP;
        }
    }
    return NF_ACCEPT;
}

unsigned int printInfo(void *priv, struct sk_buff *skb, const struct nf_hook_state *state) {
    struct iphdr *iph;
    char *hook;
    char *protocol;

    switch (state->hook) {
        case NF_INET_LOCAL_IN:     hook = "LOCAL_IN";     break; 
        case NF_INET_LOCAL_OUT:    hook = "LOCAL_OUT";    break; 
        case NF_INET_PRE_ROUTING:  hook = "PRE_ROUTING";  break; 
        case NF_INET_POST_ROUTING: hook = "POST_ROUTING"; break; 
        case NF_INET_FORWARD:      hook = "FORWARD";      break; 
        default:                   hook = "IMPOSSIBLE";   break;
    }
    printk(KERN_INFO "*** %s\n", hook); // Print out the hook info

    iph = ip_hdr(skb);
    switch (iph->protocol) {
        case IPPROTO_UDP:  protocol = "UDP";   break;
        case IPPROTO_TCP:  protocol = "TCP";   break;
        case IPPROTO_ICMP: protocol = "ICMP";  break;
        default:           protocol = "OTHER"; break;
    }
    // Print out the IP addresses and protocol
    printk(KERN_INFO "    %pI4  --> %pI4 (%s)\n", &(iph->saddr), &(iph->daddr), protocol);

    return NF_ACCEPT;
}


unsigned int blockPing(void *priv, struct sk_buff *skb, const struct nf_hook_state *state) {
    /*
     * To prevent others to ping the VM (10.9.0.1), drop ICMP requests with destination 10.9.0.1
     */
    struct iphdr *iph;
    struct icmphdr *icmph;

    char ip[16] = "10.9.0.1";
    u32  ip_addr;

    if (!skb) return NF_ACCEPT;

    iph = ip_hdr(skb);
    // Convert the IPv4 address from dotted decimal to 32-bit binary
    in4_pton(ip, -1, (u8 *)&ip_addr, '\0', NULL);

    if (iph->protocol == IPPROTO_ICMP) {
        icmph = icmp_hdr(skb);
        if (iph->daddr == ip_addr && icmph->type == 8) { // icmp type 8 is request
            printk(KERN_WARNING "*** Dropping %pI4 (ICMP Request)\n", &(iph->daddr));
            return NF_DROP;
        }
    }
    return NF_ACCEPT;
}

unsigned int blockTelnet(void *priv, struct sk_buff *skb, const struct nf_hook_state *state) {
    /*
     * To prevent others to telnet into the VM (10.9.0.1)
     * drop TCP packets with destination IP 10.9.0.1 and port 23
     */
    struct iphdr *iph;
    struct tcphdr *tcph;

    u16  port   = 23;
    char ip[16] = "10.9.0.1";
    u32  ip_addr;

    if (!skb) return NF_ACCEPT;

    iph = ip_hdr(skb);
    // Convert the IPv4 address from dotted decimal to 32-bit binary
    in4_pton(ip, -1, (u8 *)&ip_addr, '\0', NULL);

    if (iph->protocol == IPPROTO_TCP) {
        tcph = tcp_hdr(skb);
        if (iph->daddr == ip_addr && ntohs(tcph->dest) == port) {
            printk(KERN_WARNING "*** Dropping %pI4 (Telnet), port %d\n", &(iph->daddr), port);
            return NF_DROP;
        }
    }
    return NF_ACCEPT;
}

void registerBlockPing(void) {
    hook_ping.hook = blockPing;
    hook_ping.hooknum = NF_INET_PRE_ROUTING;
    hook_ping.pf = PF_INET;
    hook_ping.priority = NF_IP_PRI_FIRST;
    nf_register_net_hook(&init_net, &hook_ping);
}
void registerBlockTelnet(void) {
    hook_telnet.hook = blockTelnet;
    hook_telnet.hooknum = NF_INET_PRE_ROUTING;
    hook_telnet.pf = PF_INET;
    hook_telnet.priority = NF_IP_PRI_FIRST;
    nf_register_net_hook(&init_net, &hook_telnet);
}

int registerFilter(void) {
    printk(KERN_INFO "Registering filters.\n");

    hook1.hook = printInfo;
    hook1.hooknum = NF_INET_LOCAL_OUT;
    hook1.pf = PF_INET;
    hook1.priority = NF_IP_PRI_FIRST;
    nf_register_net_hook(&init_net, &hook1);

    // hook printInfo to another 4 types of hooks
    i = 0;
    while(i < 4) {
        hooks[i].hook = printInfo;
        switch(i) {
            case 0: hooks[i].hooknum = NF_INET_PRE_ROUTING; break;
            case 1: hooks[i].hooknum = NF_INET_LOCAL_IN; break;
            case 2: hooks[i].hooknum = NF_INET_FORWARD; break;
            case 3: hooks[i].hooknum = NF_INET_POST_ROUTING; break;
            default: hooks[i].hooknum = NF_INET_LOCAL_OUT; break;
        }
        hooks[i].pf = PF_INET;
        hooks[i].priority = NF_IP_PRI_FIRST;
        nf_register_net_hook(&init_net, &hooks[i]);
        i++;
    }

    hook2.hook = blockUDP;
    hook2.hooknum = NF_INET_POST_ROUTING;
    hook2.pf = PF_INET;
    hook2.priority = NF_IP_PRI_FIRST;
    nf_register_net_hook(&init_net, &hook2);

    registerBlockPing();
    registerBlockTelnet();

    return 0;
}

void removeFilter(void) {
    printk(KERN_INFO "The filters are being removed.\n");
    nf_unregister_net_hook(&init_net, &hook1);

    // unregister another 4 hooks
    i = 0;
    while(i < 4) {
        nf_unregister_net_hook(&init_net, &hooks[i]);
        i++;
    }

    nf_unregister_net_hook(&init_net, &hook2);

    nf_unregister_net_hook(&init_net, &hook_ping);
    nf_unregister_net_hook(&init_net, &hook_telnet);
}

module_init(registerFilter);
module_exit(removeFilter);

MODULE_LICENSE("GPL");
