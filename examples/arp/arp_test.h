#pragma once

#include <stdint.h>

#include <net/if_arp.h>

#define HW_ADDR_LEN 6

#define IP_LEN (4)
#define PACKET_SIZE_ARP 60 // (sizeof(struct ethhdr) + sizeof(struct arphdr_ipv4))

#define MAX_PACKET_SIZE 2048

#define ETH_ALEN 6
#define ETH_HLEN 14
#define ARP_HLEN 28

#define FOLLOWER_MAC "d8:3a:95:30:23:41"
#define COORDINATOR_MAC "d8:3a:95:30:23:40"

#define FOLLOWER_IP4 "192.168.10.11"
#define COORDINATOR_IP4 "192.168.10.21"

#define NODE_COUNT 2
#define FOLLOWER_NODE_ID 1

#define BYTE_WIDTH 8
#define BYTE_MASK 0xFF

#define MAC_BROAD_CAST_ADDR_BYTE 0xFF

enum ethernet_type {
    ETH_TYPE_ARP = 0x0806,
    ETH_TYPE_IP_V4 = 0x0800,
    ETH_TYPE_IP_V6 = 0x86DD,
    ETH_TYPE_VLAN = 0x8100,
    ETH_TYPE_PTP_V2 = 0x88F7,
};

enum arp_opcode {
    ARP_OPCODE_ARP_REQUEST = 1,
    ARP_OPCODE_ARP_REPLY = 2,
    ARP_OPCODE_RARP_REQUEST = 3,
    ARP_OPCODE_RARP_REPLY = 4,
    ARP_OPCODE_DRARP_REQUEST = 5,
    ARP_OPCODE_DRARP_REPLY = 6,
    ARP_OPCODE_DRARP_ERROR = 7,
    ARP_OPCODE_INARP_REQUEST = 8,
    ARP_OPCODE_INARP_REPLY = 9,
};

#define MAX_ENTRIES (100)

struct arphdr_ipv4 {
    struct arphdr arp;
    uint8_t sender_mac[ETH_ALEN];
    uint8_t sender_ip[IP_LEN];
    uint8_t target_mac[ETH_ALEN];
    uint8_t target_ip[IP_LEN];
};

struct ethhdr_l {
    unsigned char h_dest[ETH_ALEN];
    unsigned char h_source[ETH_ALEN];
    unsigned short h_proto;
};

struct arphdr_l {
    unsigned short ar_hrd;
    unsigned short ar_pro;
    unsigned char ar_hln;
    unsigned char ar_pln;
    unsigned short ar_op;
    unsigned char ar_sha[ETH_ALEN];
    unsigned char ar_sip[4];
    unsigned char ar_tha[ETH_ALEN];
    unsigned char ar_tip[4];
};

struct arp_entry {
    uint8_t ip[IP_LEN];
    uint8_t mac[ETH_ALEN];
};
