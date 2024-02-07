#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

/* Ethernet header */
struct ethheader {
    u_char  ether_dhost[6]; /* destination host address */
    u_char  ether_shost[6]; /* source host address */
    u_short ether_type;     /* protocol type (IP, ARP, RARP, etc) */
};

/* IP Header */
struct ipheader {
    unsigned char      iph_ihl:4, //IP header length
                        iph_ver:4; //IP version
    unsigned char      iph_tos; //Type of service
    unsigned short int iph_len; //IP Packet length (data + header)
    unsigned short int iph_ident; //Identification
    unsigned short int iph_flag:3, //Fragmentation flags
                        iph_offset:13; //Flags offset
    unsigned char      iph_ttl; //Time to Live
    unsigned char      iph_protocol; //Protocol type
    unsigned short int iph_chksum; //IP datagram checksum
    struct  in_addr    iph_sourceip; //Source IP address
    struct  in_addr    iph_destip;   //Destination IP address
};

/* This function will be invoked by pcap for each captured packet, we can process each packet inside the function
*/
void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
    printf("Got a packet\n");
    struct ethheader* eth = (struct ethheader*) packet;
    if(ntohs(eth->ether_type) == 0x0800) { // 0x0800 is IP type
        struct ipheader* ip= (struct ipheader*) (packet + sizeof(struct ethheader));
        printf("From: %s\n", inet_ntoa(ip->iph_sourceip));
        printf("  To: %s\n", inet_ntoa(ip->iph_destip));
        // determine protocol used
        switch(ip->iph_protocol) {
            case IPPROTO_TCP:
                printf("Protocol: TCP\n");
                return;
            case IPPROTO_UDP:
                printf("Protocol: UDP\n");
                return;
            case IPPROTO_ICMP:
                printf("Protocol: ICMP\n");
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
    char filter_exp[] = "icmp"; // hold filter expression
    bpf_u_int32 net; // unsigned 32-bit integer, store the network address associated with the network interface used for packet capture

	/* Step 1: Open live pcap session on NIC with name "br-06753fb4dff8"
	 * 1: whether to put the interface into promiscuous mode
	 * 1000: the timeout value in milliseconds for capturing packets
	 */
    handle = pcap_open_live("br-06753fb4dff8", BUFSIZ, 1, 1000, errbuf);

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
