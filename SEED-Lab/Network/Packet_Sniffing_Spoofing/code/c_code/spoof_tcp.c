#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include "my_header.h"

#define SRC_IP      "10.9.0.6"
#define DEST_IP     "10.9.0.5"
#define SRC_PORT    5789
#define DEST_PORT   9875
#define SEQ_NUM     123456789
#define TCP_DATA    "Hello World!"

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
unsigned short calculate_tcp_checksum(struct ipheader *ip) {
    struct tcpheader *tcp = (struct tcpheader *)((u_char *)ip + sizeof(struct ipheader));

    int tcp_len = ntohs(ip->iph_len) - sizeof(struct ipheader);

    /* pseudo tcp header for the checksum computation */
    struct pseudo_tcp p_tcp;
    memset(&p_tcp, 0x0, sizeof(struct pseudo_tcp));

    p_tcp.saddr  = ip->iph_sourceip.s_addr;
    p_tcp.daddr  = ip->iph_destip.s_addr;
    p_tcp.mbz    = 0;
    p_tcp.ptcl   = IPPROTO_TCP;
    p_tcp.tcpl   = htons(tcp_len);
    memcpy(&p_tcp.tcp, tcp, tcp_len);

    return  (unsigned short) in_cksum((unsigned short *)&p_tcp, tcp_len + 12);
}

char buffer[1024];

int main() {
    memset(buffer, 0, 1024);

    struct ipheader*  ip  = (struct ipheader *) buffer;
    struct tcpheader* tcp = (struct tcpheader *) (buffer + sizeof(struct ipheader));

    // fill in the TCP data
    char *data = buffer + sizeof(struct ipheader) + sizeof(struct tcpheader);
    const char *msg = TCP_DATA;
    int data_len = strlen(msg);
    strncpy(data, msg, data_len);

    // fill in the TCP header
    // mind the byte order!
    tcp->tcp_sport = htons(SRC_PORT);
    tcp->tcp_dport = htons(DEST_PORT);
    tcp->tcp_seq   = htonl(SEQ_NUM);
    tcp->tcp_offx2 = 0x50;
    tcp->tcp_flags = 0x00;
    tcp->tcp_win   = htons(20000);

    // fill in the IP header
    ip->iph_ihl             = 5;
    ip->iph_ver             = 4;
    ip->iph_ttl             = 20;
    ip->iph_sourceip.s_addr = inet_addr(SRC_IP); // Source IP
    ip->iph_destip.s_addr   = inet_addr(DEST_IP);  // Dest IP
    ip->iph_protocol        = IPPROTO_TCP; // The value is 6.
    // mind the byte order!
    ip->iph_len             = htons(sizeof(struct ipheader) + sizeof(struct tcpheader) + data_len);

    // ip->iph_chksum will be set by the system
    tcp->tcp_sum = calculate_tcp_checksum(ip);

    send_raw_ip_packet(ip);
}
