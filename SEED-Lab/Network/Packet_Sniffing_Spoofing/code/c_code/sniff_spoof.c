#include <pcap.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>

#include "my_header.h"

unsigned short in_cksum (unsigned short *buf, int length) {
    unsigned short *w = buf;
    int nleft = length;
    int sum = 0;
    unsigned short temp=0;

    /*
    * The algorithm uses a 32 bit accumulator (sum), adds
    * sequential 16 bit words to it, and at the end, folds back all
    * the carry bits from the top 16 bits into the lower 16 bits.
    */
    while (nleft > 1)  {
        sum += *w++;
        nleft -= 2;
    }

    /* treat the odd byte at the end, if any */
    if (nleft == 1) {
        *(u_char *)(&temp) = *(u_char *)w ;
        sum += temp;
    }

    /* add back carry outs from top 16 bits to low 16 bits */
    sum = (sum >> 16) + (sum & 0xffff);  // add hi 16 to low 16
    sum += (sum >> 16);                  // add carry
    return (unsigned short)(~sum);
}

void send_raw_ip_packet(struct ipheader *ip) {
    struct sockaddr_in dest_info;
    int enable = 1;

    // Step 1: create a raw network socket
    int sd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if(sd < 0) {
        perror("socket() error");
        exit(-1);
    }

    // Step 2: Set socket option
    setsockopt(sd, IPPROTO_IP, IP_HDRINCL, &enable, sizeof(enable));

    // Step 3: Provide needed information about destination
    dest_info.sin_family = AF_INET;
    dest_info.sin_addr   = ip->iph_destip;

    // Step 4: Send the packet out
    if(sendto(sd, ip, ntohs(ip->iph_len), 0, (struct sockaddr *)&dest_info, sizeof(dest_info)) < 0) {
        perror("sendto() error");
        exit(-1);
    }
    close(sd);
}

char buffer[1024];

void spoof_icmp_reply(struct ipheader* ip, struct icmpheader* icmp) {
    // fill in the ICMP header
    icmp->icmp_type   = 0; // 8: request, 0: reply
    icmp->icmp_chksum = in_cksum((unsigned short *) icmp, sizeof(struct icmpheader));

    // fill in the IP header
    struct in_addr tmp = ip->iph_destip;
    ip->iph_destip     = ip->iph_sourceip;
    ip->iph_sourceip   = tmp;

    send_raw_ip_packet(ip);

    printf("Reply From %s\n", inet_ntoa(ip->iph_sourceip));
    printf("Reply   To %s\n", inet_ntoa(ip->iph_destip));
}

void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
    printf("*****Got a packet*****\n");
    struct ethheader* eth = (struct ethheader*) packet;
    if(ntohs(eth->ether_type) == 0x0800) { // 0x0800 is IP type
        struct ipheader* ip_rcv= (struct ipheader*) (packet + sizeof(struct ethheader));
        printf("From: %s\n", inet_ntoa(ip_rcv->iph_sourceip));
        printf("  To: %s\n", inet_ntoa(ip_rcv->iph_destip));
        // determine protocol used
        switch(ip_rcv->iph_protocol) {
            case IPPROTO_TCP:
                printf("Protocol: TCP\n");
                return;
            case IPPROTO_UDP:
                printf("Protocol: UDP\n");
                return;
            case IPPROTO_ICMP:
                printf("Protocol: ICMP\n");
                struct icmpheader* icmp_rcv= (struct icmpheader*) (packet + sizeof(struct ethheader) + sizeof(struct ipheader));
                if(icmp_rcv->icmp_type==8) spoof_icmp_reply(ip_rcv, icmp_rcv);
                return;
            default:
                printf("Protocol: others\n");
                return;
        }
    }
}

int main() {
    pcap_t *handle; // this handle is used to manage the packet capture session
    char errbuf[PCAP_ERRBUF_SIZE]; // hold error messages
    struct bpf_program fp; // hold compiled BPF (Berkeley Packet Filter) program. BPF is a filtering mechanism used to specify which packets should be captured
    //char filter_exp[] = "icmp"; // hold filter expression
    //char filter_exp[] = "icmp and host 10.9.0.5 and host 10.9.0.6";
    //char filter_exp[] = "tcp dst portrange 10-100";
    char filter_exp[] = "";
    bpf_u_int32 net; // unsigned 32-bit integer, store the network address associated with the network interface used for packet capture

    /* Step 1: Open live pcap session on NIC with name "br-06753fb4dff8"
     * 1: whether to put the interface into promiscuous mode
     * 1000: the timeout value in milliseconds for capturing packets
     */
    handle = pcap_open_live("br-537c81d8b33b", BUFSIZ, 1, 1000, errbuf);

    /* Step 2: Compile filter_exp into BPF psuedo-code
     * &fp: will be filled in with the compiled filter program
     * 0: typically set to 0 for backward compatibility and is reserved for future use, can be safely ignored for now
     * net: the network mask of the network on which packets are being captured
     */
    pcap_compile(handle, &fp, filter_exp, 0, net);
    if(pcap_setfilter(handle, &fp) != 0) {
        pcap_perror(handle, "Error:");
        exit(EXIT_FAILURE);
    }

    /* Step 3: Capture packets
     * -1: `pcap_loop()` should loop indefinitely
     * NULL: a pointer to user data that can be passed to the callback function, in this case, no data is passed
     */
    pcap_loop(handle, -1, got_packet, NULL);

    pcap_close(handle); // Close the handle
    return 0;
}
