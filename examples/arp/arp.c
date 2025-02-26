#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <10baset1s/lan865x.h>
#include <10baset1s/rpi_spi.h>
#include <10baset1s/xbaset1s_arch.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <net/if_arp.h>
#include <netinet/in.h>

#include "arp_test.h"

int spi_handle;

unsigned char follower_ip[IP_LEN];
unsigned char coordinator_ip[IP_LEN];

unsigned char my_mac[HW_ADDR_LEN];

struct arp_entry g_arp_table[MAX_ENTRIES];
int g_arp_table_count = 0;

uint64_t mac_to_int64(const char* mac_address) {
    uint64_t result = 0;
    unsigned int values[HW_ADDR_LEN];

    if (sscanf(mac_address, "%x:%x:%x:%x:%x:%x", &values[5], &values[4], &values[3], &values[2], &values[1],
               &values[0]) == HW_ADDR_LEN) {
        for (int i = 0; i < HW_ADDR_LEN; i++) {
            result = (result << BYTE_WIDTH) | (values[i] & BYTE_MASK);
        }
    }

    return result;
}

uint16_t make_arp_packet(uint8_t* packet) {
    struct ethhdr* eth = (struct ethhdr*)packet;
    struct arphdr_ipv4* arp = (struct arphdr_ipv4*)(eth + 1);

    /* Fill Ethernet header */
    memset(eth->h_dest, MAC_BROAD_CAST_ADDR_BYTE, ETH_ALEN);
    memcpy(eth->h_source, my_mac, ETH_ALEN);
    eth->h_proto = htons(ETH_P_ARP);

    /* Fill ARP header */
    arp->arp.ar_hrd = htons(1);            /* Ethernet */
    arp->arp.ar_pro = htons(ETH_P_IP);     /* IPv4 */
    arp->arp.ar_hln = ETH_ALEN;            /* MAC size */
    arp->arp.ar_pln = IP_LEN;              /* IP size */
    arp->arp.ar_op = htons(ARPOP_REQUEST); /* ARP Request */
    memcpy(arp->sender_mac, my_mac, ETH_ALEN);
    inet_pton(AF_INET, COORDINATOR_IP4, arp->sender_ip);
    memset(arp->target_mac, 0x00, ETH_ALEN); /* Unknown */
    inet_pton(AF_INET, FOLLOWER_IP4, arp->target_ip);

    return PACKET_SIZE_ARP;
}

void create_arp_reply(unsigned char* request_packet, unsigned char* reply_packet, unsigned char* my_mac,
                      unsigned char* my_ip) {
    struct ethhdr_l* eth_req = (struct ethhdr_l*)request_packet;
    struct arphdr_l* arp_req = (struct arphdr_l*)(request_packet + ETH_HLEN);

    struct ethhdr_l* eth_reply = (struct ethhdr_l*)reply_packet;
    struct arphdr_l* arp_reply = (struct arphdr_l*)(reply_packet + ETH_HLEN);

    /* 이더넷 헤더 설정 */
    memcpy(eth_reply->h_dest, eth_req->h_source, ETH_ALEN);
    memcpy(eth_reply->h_source, my_mac, ETH_ALEN);
    eth_reply->h_proto = htons(ETH_TYPE_ARP); /* ARP 프로토콜 */

    /* ARP 헤더 설정 */
    arp_reply->ar_hrd = htons(1);                   /* 이더넷 */
    arp_reply->ar_pro = htons(ETH_TYPE_IP_V4);      /* IPv4 */
    arp_reply->ar_hln = ETH_ALEN;                   /* MAC 주소 길이 */
    arp_reply->ar_pln = IP_LEN;                     /* IP 주소 길이 */
    arp_reply->ar_op = htons(ARP_OPCODE_ARP_REPLY); /* ARP 응답 */

    memcpy(arp_reply->ar_sha, my_mac, ETH_ALEN);
    memcpy(arp_reply->ar_sip, my_ip, IP_LEN);
    memcpy(arp_reply->ar_tha, arp_req->ar_sha, ETH_ALEN);
    memcpy(arp_reply->ar_tip, arp_req->ar_sip, IP_LEN);
}

static int add_to_arp_table(unsigned char* ip, unsigned char* mac) {
    int ret = -RET_FAIL;
    int i;

    for (i = 0; i < g_arp_table_count; i++) {
        if (memcmp(g_arp_table[i].ip, ip, IP_LEN) == 0) {
            /* IP already exists, update MAC */
            memcpy(g_arp_table[i].mac, mac, ETH_ALEN);
            return RET_SUCCESS;
        }
    }
    if (g_arp_table_count < MAX_ENTRIES) {
        memcpy(g_arp_table[g_arp_table_count].ip, ip, IP_LEN);
        memcpy(g_arp_table[g_arp_table_count].mac, mac, ETH_ALEN);
        g_arp_table_count++;
        ret = RET_SUCCESS;
    } else {
        printf("ARP table is full\n");
    }

    return ret;
}

static void print_arp_table(void) {
    int i;

    printf("\nARP Table:\n");
    for (i = 0; i < g_arp_table_count; i++) { // clang-format off
        printf("IP: %d.%d.%d.%d, MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
               g_arp_table[i].ip[0], g_arp_table[i].ip[1], g_arp_table[i].ip[2], g_arp_table[i].ip[3],
               g_arp_table[i].mac[0], g_arp_table[i].mac[1], g_arp_table[i].mac[2], g_arp_table[i].mac[3],
               g_arp_table[i].mac[4], g_arp_table[i].mac[5]);
    } // clang-format on
}

static int check_arp_packet(uint8_t* packet) {
    struct ethhdr* eth = (struct ethhdr*)packet;
    struct arphdr_ipv4* arp = (struct arphdr_ipv4*)(eth + 1);

    if (ntohs(eth->h_proto) != ETH_P_ARP) {
        printf("Packet type is unknown. The packet type is 0x%04x\n", ntohs(eth->h_proto));
        return -RET_FAIL;
    }

    /* Check operation type */
    uint16_t op_type = ntohs(arp->arp.ar_op);
    if (op_type != ARPOP_REPLY && op_type != ARPOP_REQUEST) {
        printf("op type is unknown. op type is 0x%04x\n", op_type);
        return -RET_FAIL;
    }

    /* Handle ARP reply */
    if (op_type == ARPOP_REPLY) {
        printf("Received ARP reply from %d.%d.%d.%d\n", arp->sender_ip[0], arp->sender_ip[1], arp->sender_ip[2],
               arp->sender_ip[3]);
        add_to_arp_table(arp->sender_ip, arp->sender_mac);
        print_arp_table();
    } else { /* ARPOP_REQUEST */
        printf("Received ARP request from %d.%d.%d.%d\n", arp->sender_ip[0], arp->sender_ip[1], arp->sender_ip[2],
               arp->sender_ip[3]);
    }

    return RET_SUCCESS;
}

int fill_my_mac_address(int handle) {
    int i;
    uint64_t mac;

    mac = get_mac_address_on_chip((unsigned int)handle);
    if (mac == 0) {
        printf("[%s]MAC address(%ld) has not been set yet. Check your settings first !\n", __func__, mac);
        return -RET_FAIL;
    }

    for (i = 0; i < HW_ADDR_LEN; i++) {
        my_mac[i] = (unsigned char)((mac >> (i * BYTE_WIDTH)) & BYTE_MASK);
    }

    printf("MY MAC: \n    ");
    for (i = 0; i < HW_ADDR_LEN - 1; i++) {
        printf("%02x:", my_mac[i]);
    }
    printf("%02x\n", my_mac[i]);

    return RET_SUCCESS;
}

int config_lan865x_initialize(uint64_t mac, int node_id, int node_cnt) {

    set_mac_address_to_chip((unsigned int)spi_handle, mac);
    set_node_config((unsigned int)spi_handle, node_id, node_cnt);

    init_lan865x((unsigned int)spi_handle);

    if (fill_my_mac_address(spi_handle)) {
        spi_close(spi_handle);
        return -RET_FAIL;
    }

    return RET_SUCCESS;
}

int do_as_coordinator() {
    uint64_t mac;
    int ret;
    uint16_t length;
    uint16_t received_length;
#ifdef FRAME_TIMESTAMP_ENABLE
    struct timestamp_format timestamp;
    int timestamp_id = TTSC_A;
#endif
    unsigned char buffer[MAX_PACKET_SIZE] = {
        0,
    };
    unsigned char reply_packet[MAX_PACKET_SIZE] = {
        0,
    };

    mac = mac_to_int64(COORDINATOR_MAC);
    if (config_lan865x_initialize(mac, 0, NODE_COUNT)) {
        spi_close(spi_handle);
        return -1;
    }

    length = make_arp_packet(buffer);

#ifdef FRAME_TIMESTAMP_ENABLE
    ret = spi_transmit_frame_with_timestamp((unsigned int)spi_handle, (uint8_t*)buffer, length, timestamp_id);
#else
    ret = spi_transmit_frame((unsigned int)spi_handle, (uint8_t*)buffer, length);
#endif
    while (ret != RET_SUCCESS) {
        printf("Fail to send ARP request, the error code is %d\n", ret);
        sleep(1);
        ret = spi_transmit_frame((unsigned int)spi_handle, (uint8_t*)buffer, length);
    }
#ifdef FRAME_TIMESTAMP_ENABLE
    if (get_timestamp((unsigned int)spi_handle, timestamp_id, &timestamp)) {
        printf("Fail to get timestamp(%d)\n", timestamp_id);
    } else {
        print_timestamp_info(timestamp);
    }
#endif
    printf("Success to send ARP request\n");

#ifdef FRAME_TIMESTAMP_ENABLE
    ret = spi_receive_frame_with_timestamp((unsigned int)spi_handle, reply_packet, &received_length, &timestamp);
#else
    ret = spi_receive_frame((unsigned int)spi_handle, reply_packet, &received_length);
#endif
    while (ret != RET_SUCCESS) {
        printf("Fail to receive ARP reply, the error code is %d\n", ret);
        sleep(1);
#ifdef FRAME_TIMESTAMP_ENABLE
        ret = spi_receive_frame_with_timestamp((unsigned int)spi_handle, reply_packet, &received_length, &timestamp);
#else
        ret = spi_receive_frame((unsigned int)spi_handle, reply_packet, &received_length);
#endif
    }

    printf("Success to receive ARP reply\n");
    ret = check_arp_packet(reply_packet);

#ifdef FRAME_TIMESTAMP_ENABLE
    print_timestamp_info(timestamp);
#endif
    return RET_SUCCESS;
}

int do_as_follower() {
    uint64_t mac;
    int ret;
    uint16_t received_length;
    uint16_t length;
#ifdef FRAME_TIMESTAMP_ENABLE
    struct timestamp_format timestamp;
    int timestamp_id = TTSC_A;
#endif
    unsigned char buffer[MAX_PACKET_SIZE] = {
        0,
    };
    unsigned char reply_packet[MAX_PACKET_SIZE] = {
        0,
    };

    mac = mac_to_int64(FOLLOWER_MAC);
    if (config_lan865x_initialize(mac, FOLLOWER_NODE_ID, NODE_COUNT)) {
        spi_close(spi_handle);
        return -1;
    }

#ifdef FRAME_TIMESTAMP_ENABLE
    ret = spi_receive_frame_with_timestamp((unsigned int)spi_handle, reply_packet, &received_length, &timestamp);
#else
    ret = spi_receive_frame((unsigned int)spi_handle, buffer, &received_length);
#endif
    while (ret != RET_SUCCESS) {
        printf("Fail to receive ARP request, the error code is %d\n", ret);
        sleep(1);
#ifdef FRAME_TIMESTAMP_ENABLE
        ret = spi_receive_frame_with_timestamp((unsigned int)spi_handle, reply_packet, &received_length, &timestamp);
#else
        ret = spi_receive_frame((unsigned int)spi_handle, buffer, &received_length);
#endif
    }

#ifdef FRAME_TIMESTAMP_ENABLE
    print_timestamp_info(timestamp);
#endif
    printf("Success to recieve ARP request");
    ret = check_arp_packet(buffer);
    if (ret == RET_SUCCESS) {
        create_arp_reply(buffer, reply_packet, my_mac, follower_ip);
        length = PACKET_SIZE_ARP;
#ifdef FRAME_TIMESTAMP_ENABLE
        ret = spi_transmit_frame_with_timestamp((unsigned int)spi_handle, reply_packet, length, timestamp_id);
#else
        ret = spi_transmit_frame((unsigned int)spi_handle, reply_packet, length);
#endif
        while (ret != RET_SUCCESS) {
            printf("Fail to send ARP reply, the error code is %d\n", ret);
            sleep(1);
#ifdef FRAME_TIMESTAMP_ENABLE
            ret = spi_transmit_frame_with_timestamp((unsigned int)spi_handle, reply_packet, length, timestamp_id);
#else
            ret = spi_transmit_frame((unsigned int)spi_handle, reply_packet, length);
#endif
        }
#ifdef FRAME_TIMESTAMP_ENABLE
        print_timestamp_info(timestamp);
#endif
        printf("Success to send ARP reply\n");
    }

    return RET_SUCCESS;
}

int main(int argc, char* argv[]) {
    int opt = 0;

    spi_handle = spi_open();
    if (spi_handle < 0) {
        printf("Fail to open SPI, error code:%d\n", spi_handle);
        return -RET_FAIL;
    }

    sscanf(FOLLOWER_IP4, "%d.%d.%d.%d", &follower_ip[0], &follower_ip[1], &follower_ip[2], &follower_ip[3]);
    sscanf(COORDINATOR_IP4, "%d.%d.%d.%d", &coordinator_ip[0], &coordinator_ip[1], &coordinator_ip[2],
           &coordinator_ip[3]);

    while ((opt = getopt(argc, argv, "cfh")) != -1) {
        switch (opt) {
        case 'c':
            printf("Coordinator mode\n");
            do_as_coordinator();
            break;
        case 'f':
            printf("Follower mode\n");
            do_as_follower();
            break;
        case 'h':
            printf(
                "Help mode\n Usage: %s -c|-f|-h\n -c   : Coordinator mode\n -f   : Follower mode\n -h   : Help mode\n",
                argv[0]);
            break;
        default: /* '?' */
            printf("Invalid argument: %s\n Usage: %s -c|-f|-h\n", argv[opt], argv[0]);
        }
    }

    if (optind < argc || argc == 1) {
        printf("Invalid argument: %s\n Usage: %s -c|-f|-h\n", argv[optind], argv[0]);
    }

    spi_close((unsigned int)spi_handle);

    return 0;
}
