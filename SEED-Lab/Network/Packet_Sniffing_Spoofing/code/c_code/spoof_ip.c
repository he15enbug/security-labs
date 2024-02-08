#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include "my_header.h"

#define SRC_IP      "10.9.0.6"
#define DEST_IP     "10.9.0.5"

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

int main() {
    memset(buffer, 0, 1024);

    struct ipheader* ip  = (struct ipheader *) buffer;
    // fill in the IP header
    ip->iph_ihl             = 5;
    ip->iph_ver             = 4;
    ip->iph_ttl             = 20;
    ip->iph_sourceip.s_addr = inet_addr(SRC_IP); // Source IP
    ip->iph_destip.s_addr   = inet_addr(DEST_IP);  // Dest IP
    ip->iph_protocol        = IPPROTO_TCP; // The value is 6.
    // mind the byte order!
    ip->iph_len             = htons(sizeof(struct ipheader));
    // ip->iph_chksum will be set by the system

    send_raw_ip_packet(ip);
}
