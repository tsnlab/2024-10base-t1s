#ifndef ARP_TEST_H
#define ARP_TEST_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <sys/socket.h>

#define MAX_ENTRIES 100
#define PACKET_SIZE_ARP 42

/* TODO fix eth_hdr to ethhdr, arp_hdr -> arphdr + @
 * There is ethhdr defined in <linux/if_header.h>
 * Also there is arphdr in <linux/if_arp.h> but without the sender/receiver addresses. */

/* TODO Elaborate PACKET_SIZE_ARP
 * Length of Ethernet header is defined as ETH_HLEN, and Length of ARP header better be defined as sizeof(arphdr) +
 * 20(address field size in IPv4) */

// Ethernet header
struct eth_hdr {
    unsigned char dest_mac[6];
    unsigned char src_mac[6];
    unsigned short ethertype;
};

// ARP header
struct arp_hdr {
    unsigned short htype;
    unsigned short ptype;
    unsigned char hlen;
    unsigned char plen;
    unsigned short oper;
    unsigned char sender_mac[6];
    unsigned char sender_ip[4];
    unsigned char target_mac[6];
    unsigned char target_ip[4];
};

// ARP table entry
struct arp_entry {
    unsigned char ip[4];
    unsigned char mac[6];
};

typedef enum {
    ARP_E_SUCCESS = 0,
    ARP_E_REQUEST_FAILED,
    ARP_E_REPLY_FAILED,
    ARP_E_UNKNOWN_ERROR,
} ARP_ReturnType;

ARP_ReturnType ArpTest(void);

#endif /* ARP_TEST_H */
