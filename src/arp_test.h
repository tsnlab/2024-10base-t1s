#ifndef ARP_TEST_H
#define ARP_TEST_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <sys/socket.h>

#include "spi.h"

#define MAX_ENTRIES (100)

#define MAC_ADDR "\xd8\x3a\xdd\x44\xab\x0f" // TODO: Change to your MAC address
#define IP_ADDR "172.16.11.201"             // TODO: Change to your IP address
#define TARGET_IP_ADDR "172.16.11.203"      // TODO: Change to target IP address

#define IPV4 // This project regards IPv4 for now
#ifdef IPV4

#define IP_LEN (4)
#define PACKET_SIZE_ARP (sizeof(struct ethhdr) + sizeof(struct arphdr_ipv4))

struct arphdr_ipv4 {
    struct arphdr arp;
    uint8_t sender_mac[ETH_ALEN];
    uint8_t sender_ip[IP_LEN];
    uint8_t target_mac[ETH_ALEN];
    uint8_t target_ip[IP_LEN];
};

// ARP table entry regarding IPv4
struct arp_entry {
    uint8_t ip[IP_LEN];
    uint8_t mac[ETH_ALEN];
};

#endif /* IPV4 */

enum {
    ARP_E_SUCCESS = 0,
    ARP_E_REQUEST_FAILED,
    ARP_E_REPLY_FAILED,
    ARP_E_UNKNOWN_ERROR,
};

int arp_test(int plca_mode);

#endif /* ARP_TEST_H */
