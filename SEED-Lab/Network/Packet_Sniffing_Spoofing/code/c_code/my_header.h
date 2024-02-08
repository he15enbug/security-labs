/* Ethernet header */
struct ethheader {
    u_char  ether_dhost[6];  // Destination host address
    u_char  ether_shost[6];  // Source host address
    u_short ether_type;      // IP? ARP? RARP? etc
};

/* IP Header */
struct ipheader {
    unsigned char      iph_ihl:4,     // IP header length
                       iph_ver:4;     // IP version
    unsigned char      iph_tos;       // Type of service
    unsigned short int iph_len;       // IP Packet length (data + header)
    unsigned short int iph_ident;     // Identification
    unsigned short int iph_flag:3,    // Fragmentation flags
                       iph_offset:13; // Flags offset
    unsigned char      iph_ttl;       // Time to Live
    unsigned char      iph_protocol;  // Protocol type
    unsigned short int iph_chksum;    // IP datagram checksum
    struct  in_addr    iph_sourceip;  // Source IP address
    struct  in_addr    iph_destip;    // Destination IP address
};


/* UDP Header */
struct udpheader {
    u_int16_t udp_sport;  // Source port
    u_int16_t udp_dport;  // Destination port
    u_int16_t udp_ulen;   // UDP length
    u_int16_t udp_sum;    // UDP checksum
};

/* ICMP Header  */
struct icmpheader {
    unsigned char icmp_type;        // ICMP message type
    unsigned char icmp_code;        // Error code
    unsigned short int icmp_chksum; // Checksum for ICMP Header and data
    unsigned short int icmp_id;     // Used for identifying request
    unsigned short int icmp_seq;    // Sequence number
};

/* TCP Header */
struct tcpheader {
    u_short tcp_sport;
    u_short tcp_dport;
    u_int32_t tcp_seq;
    u_int32_t tcp_ack;
    u_char tcp_offx2;   // Offset (4 bits) + reserved (3 bits) + NS flag (1 bit)
#define TH_OFF(th)  (((th)->tcp_offx2 & 0xf0) >> 4)
    u_char tcp_flags;   // TCP flags
#define TH_FIN  0x01
#define TH_SYN  0x02
#define TH_RST  0x04
#define TH_PUSH 0x08
#define TH_ACK  0x10
#define TH_URG  0x20
#define TH_ECE  0x40
#define TH_CWR  0x80
#define TH_FLAGS    (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
    u_short tcp_win; // Window size
    u_short tcp_sum; // Checksum
    u_short tcp_urp; // Urgent pointer
};

/* Psuedo TCP header */
struct pseudo_tcp {
    unsigned saddr, daddr;
    unsigned char mbz;
    unsigned char ptcl;
    unsigned short tcpl;
    struct tcpheader tcp;
    char payload[1500];
};