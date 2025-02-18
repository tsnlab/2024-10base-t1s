#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>

#include "arp.h"
#include "ethernet.h"
#include "icmp.h"
#include "ip.h"
#include "ipv4.h"
#include "udp.h"

uint16_t calculate_checksum(uint16_t* data, int length) {
    uint32_t sum = 0;

    while (length > 1) {
        sum += *data++;
        length -= 2;
    }

    if (length == 1) {
        sum += *(uint8_t*)data;
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return ~sum;
}

void create_udp_packet(unsigned char* smac, unsigned char* dmac, const char* src_ip_str, const char* dst_ip_str,
                       uint16_t src_port, uint16_t dst_port, uint16_t frame_length, uint8_t* packet_buffer) {
    if (frame_length < 64) {
        fprintf(stderr, "Error: Frame length must be bigger than 64 bytes.\n");
        exit(EXIT_FAILURE);
    }

    struct ethernet_header* eth = (struct ethernet_header*)packet_buffer;
    for (int i = 0; i < ETH_ALEN; i++) {
        eth->smac[i] = smac[i];
        eth->dmac[i] = dmac[i];
    }
    eth->type = ETH_TYPE_IPv4;

    struct ipv4_header* ip = (struct ipv4_header*)(packet_buffer + sizeof(struct ethernet_header));
    ip->version = 0x4;
    ip->hdr_len = 0x5;
    ip->dscp = 0;
    ip->ecn = 0;
    ip->len = frame_length - sizeof(struct ethernet_header);
    ip->id = 0;
    ip->flags = 0;
    ip->frag_offset = 0;
    ip->ttl = 64;
    ip->proto = IP_PROTO_UDP;
    ip->checksum = 0;
    ip->src = inet_addr(src_ip_str);
    ip->dst = inet_addr(dst_ip_str);

    struct udp_header* udp =
        (struct udp_header*)(packet_buffer + sizeof(struct ethernet_header) + sizeof(struct ipv4_header));
    udp->srcport = src_port;
    udp->dstport = dst_port;
    udp->length = frame_length - sizeof(struct ethernet_header) - sizeof(struct ipv4_header);
    udp->checksum = 0;

    size_t payload_offset = sizeof(struct ethernet_header) + sizeof(struct ipv4_header) + sizeof(struct udp_header);
    size_t payload_size = frame_length - payload_offset;

    memset(packet_buffer + payload_offset, 0, payload_size);
    ip->checksum = htons(calculate_checksum((uint16_t*)ip, sizeof(struct ipv4_header)));
}

void create_arp_request_frame(unsigned char* frame, unsigned char* smac, const char* src_ip, const char* dst_ip) {
    struct ethernet_header* eth = (struct ethernet_header*)frame;
    struct arp_header* arp = (struct arp_header*)(frame + ETH_HLEN);

    memset(eth->dmac, 0xFF, ETH_ALEN);
    for (int i = 0; i < ETH_ALEN; i++) {
        eth->smac[i] = smac[i];
    }
    eth->type = ETH_TYPE_ARP;

    arp->hw_type = 1;
    arp->proto_type = ETH_TYPE_IPv4;
    arp->hw_size = 6;
    arp->proto_size = 4;
    arp->opcode = 1;

    memcpy(arp->sender_hw, eth->smac, ETH_ALEN);

    for (int i = 0; i < 4; i++) {
        arp->sender_proto[3-i] = src_ip[i];
    }

    memset(arp->target_hw, 0, ETH_ALEN);

    for (int i = 0; i < 4; i++) {
        arp->target_proto[3-i] = dst_ip[i];
    }

    memset(frame + ETH_HLEN + ARP_HLEN, 0, 18);
}
