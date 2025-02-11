#include "thread.h"

#include <libcom.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <sys/time.h>

#include "arp.h"
#include "create_ethernet_packet.h"
#include "ethernet.h"
#include "icmp.h"
#include "ip.h"
#include "ipv4.h"
#include "udp.h"
#include "xbase-t1s.h"
#include "xbase_common.h"

#define FCS_LENGTH 4
#define PREAMBLE_LENGTH 8

int rx_thread_run = 1;
int tx_thread_run = 1;
int stats_thread_run = 1;

unsigned char my_mac[HW_ADDR_LEN];
unsigned char my_ipv4[IP_ADDR_LEN];
unsigned char dst_ipv4[IP_ADDR_LEN];

stats_t rx_stats;
stats_t tx_stats;

stats_t cs; /* total stats */
stats_t os; /* total stats */

unsigned long long currTv;
unsigned long long lastTv;

time_t start_time;
time_t stopTime;

char* counter_name[MAX_COUNTERS] = {
    "rxPackets", "rxBytes",    "rxErrors", "rxNoBuffer", "rxPps", "rxbps",      "rxPure-bps", "txPackets",
    "txBytes",   "txFiltered", "txErrors", "txPps",      "txbps", "txPure-bps", NULL,
};

int printCounters[] = {
    COUNTERS_RXPACKETS,
    COUNTERS_RXBYTES,
    COUNTERS_RXPPS,
    COUNTERS_RXBPS,
    COUNTERS_RXPBPS,

    COUNTERS_TXPACKETS,
    COUNTERS_TXBYTES,
    COUNTERS_TXPPS,
    COUNTERS_TXBPS,
    COUNTERS_TXPBPS,

    0xffff,
};

static void packet_parse(struct spi_rx_buffer* rx);
static int process_send_packet(struct spi_rx_buffer* rx, int pkt_length);
int api_spi_transmit_frame(uint8_t* packet, uint16_t length);
int api_spi_receive_frame(uint8_t* packet, uint16_t* length);

void initialize_statistics(stats_t* p_stats) {

    memset(p_stats, 0, sizeof(stats_t));
}

static void receiver_as_server(int sts_flag, int pkt_length) {

    struct spi_rx_buffer rx;
    uint16_t bytes_rcv;
    int status;

    printf(">>> %s\n", __func__);

    while (rx_thread_run) {
        if (sts_flag == 0) {
            sleep(1);
        }
        memset(&rx, 0, sizeof(rx));

        bytes_rcv = 0;
        if (api_spi_receive_frame((uint8_t*)rx.data, &bytes_rcv)) {
            rx_stats.rxErrors++;
            continue;
        }
        if (bytes_rcv > MAX_BUFFER_LENGTH) {
            rx_stats.rxErrors++;
            continue;
        }
        rx_stats.rxPackets++;
        rx_stats.rxBytes += (bytes_rcv + 8); /* Preamble */
        rx.metadata.frame_length = bytes_rcv;

        if (sts_flag == 0) {
            dump_buffer((unsigned char*)rx.data, bytes_rcv);
        }

        status = process_send_packet((struct spi_rx_buffer*)&rx, pkt_length);
        if (status == -1) {
            tx_stats.txFiltered++;
        }
    }
    printf("<<< %s\n", __func__);
}

static void receiver_as_client(int sts_flag, int pkt_length) {

    printf(">>> %s\n", __func__);

    while (rx_thread_run) {
        ;
        sleep(1);
    }
    printf("<<< %s\n", __func__);
}

void* receiver_thread(void* arg) {

    rx_thread_arg_t* p_arg = (rx_thread_arg_t*)arg;

    printf(">>> %s(mode: %d)\n", __func__, p_arg->mode);

    initialize_statistics(&rx_stats);

    switch (p_arg->mode) {
    case RUN_MODE_CLIENT:
        receiver_as_client(p_arg->sts_flag, p_arg->pkt_length);
        break;
    case RUN_MODE_SERVER:
        receiver_as_server(p_arg->sts_flag, p_arg->pkt_length);
        break;
    default:
        printf("%s - Unknown mode(%d)\n", __func__, p_arg->mode);
        break;
    }

    printf("<<< %s\n", __func__);

    return NULL;
}

static void test_receiver(int sts_flag, int pkt_length) {

    struct spi_rx_buffer rx;
    uint16_t bytes_rcv;

    printf(">>> %s\n", __func__);

    while (rx_thread_run) {
        if (sts_flag == 0) {
            sleep(1);
        }
        memset(&rx, 0, sizeof(rx));
        bytes_rcv = 0;
        if (api_spi_receive_frame((uint8_t*)rx.data, &bytes_rcv)) {
            rx_stats.rxErrors++;
            continue;
        }
        if (bytes_rcv > MAX_BUFFER_LENGTH) {
            rx_stats.rxErrors++;
            continue;
        }
        rx_stats.rxPackets++;
        rx_stats.rxBytes += bytes_rcv;
        rx.metadata.frame_length = bytes_rcv;

        if (sts_flag == 0) {
            packet_parse((struct spi_rx_buffer*)&rx);
        }
    }
    printf("<<< %s\n", __func__);
}

void* test_receiver_thread(void* arg) {

    rx_thread_arg_t* p_arg = (rx_thread_arg_t*)arg;

    printf(">>> %s(mode: %d)\n", __func__, p_arg->mode);

    initialize_statistics(&rx_stats);

    switch (p_arg->mode) {
    case TEST_MODE_RECEIVER:
        test_receiver(p_arg->sts_flag, p_arg->pkt_length);
        break;
    default:
        printf("%s - Unknown mode(%d)\n", __func__, p_arg->mode);
        break;
    }

    printf("<<< %s\n", __func__);

    return NULL;
}

static void packet_parse(struct spi_rx_buffer* rx) {
    uint8_t* rx_frame = (uint8_t*)&rx->data;
    struct ethernet_header* rx_eth = (struct ethernet_header*)rx_frame;
    unsigned char ipv4_addr[4];
    int i;

    mac_address(rx_eth->dmac, "      Dst. MAC: ");
    mac_address(rx_eth->smac, "      Src. MAC: ");

    switch (rx_eth->type) {
    case ETH_TYPE_ARP:
        struct arp_header* rx_arp = (struct arp_header*)ETH_PAYLOAD(rx_frame);

        printf(" ETHERNET TYPE: %s\n", "ARP");
        printf("       HW Type: %04x\n", rx_arp->hw_type);
        printf("   Proto. Type: %04x\n", rx_arp->proto_type);
        printf("       HW Size: %d\n", rx_arp->hw_size);
        printf("   Proto. Size: %d\n", rx_arp->hw_size);
        switch (rx_arp->opcode) {
        case ARP_OPCODE_ARP_REQUEST:
            printf("        Opcode: ARP_REQUEST(%d)\n", rx_arp->opcode);
            break;
        case ARP_OPCODE_ARP_REPLY:
            printf("        Opcode: ARP_REPLY(%d)\n", rx_arp->opcode);
            break;
        default:
            printf("Unknown Opcode: %d\n", rx_arp->opcode);
            return;
        }

        mac_address(rx_arp->sender_hw, "     Sender HW: ");
        ipv4_address(rx_arp->sender_proto, " Sender Proto.: ");
        mac_address(rx_arp->target_hw, "     Target HW: ");
        ipv4_address(rx_arp->target_proto, " Target Proto.: ");
        break;

    case ETH_TYPE_IPv4:
        struct ipv4_header* rx_ipv4 = (struct ipv4_header*)ETH_PAYLOAD(rx_frame);

        printf(" ETHERNET TYPE: %s\n", "IPv4");

        printf("       Version: %d\n", rx_ipv4->version);
        printf("     HDR. LEN.: %d\n", rx_ipv4->hdr_len);
        printf("          DSCP: %d\n", rx_ipv4->dscp);
        printf("           ECN: %d\n", rx_ipv4->ecn);
        printf("        LENGTH: %d\n", rx_ipv4->len);
        printf("            ID: %d\n", rx_ipv4->id);
        printf("         Flags: %d\n", rx_ipv4->flags);
        printf("   Flag Offset: %d\n", rx_ipv4->frag_offset);
        printf("           TTL: %d\n", rx_ipv4->ttl);
        printf("      Checksum: %d\n", rx_ipv4->checksum);

        for (i = 0; i < 4; i++) {
            ipv4_addr[3 - i] = (unsigned char)((rx_ipv4->src >> (i * 8)) & 0xff);
        }
        ipv4_address(ipv4_addr, "  Src. Address: ");

        for (i = 0; i < 4; i++) {
            ipv4_addr[3 - i] = (unsigned char)((rx_ipv4->dst >> (i * 8)) & 0xff);
        }
        ipv4_address(ipv4_addr, "  Dst. Address: ");

        if (rx_ipv4->proto == IP_PROTO_ICMP) {
            struct icmp_header* rx_icmp = (struct icmp_header*)IPv4_PAYLOAD(rx_ipv4);
            printf("      IP PROTO: %s\n", "ICMP");

            switch (rx_icmp->type) {
            case ICMP_TYPE_ECHO_REQUEST:
                printf("     ICMP Type: %s\n", "ECHO REQUEST");
                break;
            case ICMP_TYPE_ECHO_REPLY:
                printf("     ICMP Type: %s\n", "ECHO REPLY");
                break;
            default:
                printf("     ICMP Type: Unknown(%d)\n", rx_icmp->type);
                return;
            }
            printf(" ICMP Checksum: 0x%04x\n", rx_icmp->checksum);

        } else if (rx_ipv4->proto == IP_PROTO_UDP) {
            struct udp_header* rx_udp = (struct udp_header*)IPv4_PAYLOAD(rx_ipv4);
            printf("      IP PROTO: %s\n", "UDP");

            printf("     Src. Port: %d\n", rx_udp->srcport);
            printf("     Dst. Port: %d\n", rx_udp->dstport);
            printf("        Length: %d\n", rx_udp->length);
            printf("     Dst. Port: %d\n", rx_udp->dstport);
            printf("      Checksum: 0x%04x\n", rx_udp->checksum);
        } else {
            printf("      IP PROTO: Unknown(%d)\n", rx_ipv4->proto);
        }
        break;
    default:
        printf(" ETHERNET TYPE: Unknown(0x%04x)\n", rx_eth->type);
    }
}

static int process_send_packet(struct spi_rx_buffer* rx, int pkt_length) {
    uint8_t* buffer = (uint8_t*)rx;
    int tx_len;
    struct spi_tx_buffer* tx =
        (struct spi_tx_buffer*)(buffer + sizeof(struct rx_metadata) - sizeof(struct tx_metadata));
    struct tx_metadata* tx_metadata = &tx->metadata;
    uint8_t* rx_frame = (uint8_t*)&rx->data;
    uint8_t* tx_frame = (uint8_t*)&tx->data;
    struct ethernet_header* rx_eth = (struct ethernet_header*)rx_frame;
    struct ethernet_header* tx_eth = (struct ethernet_header*)tx_frame;
    int status;

    tx_metadata->reserved = 0;

    memcpy(&(tx_eth->dmac), &(rx_eth->smac), 6);
    memcpy(&(tx_eth->smac), my_mac, 6);

    tx_len = ETH_HLEN;

    switch (rx_eth->type) {
    case ETH_TYPE_ARP:;
        struct arp_header* rx_arp = (struct arp_header*)ETH_PAYLOAD(rx_frame);
        struct arp_header* tx_arp = (struct arp_header*)ETH_PAYLOAD(tx_frame);
        if (rx_arp->opcode != ARP_OPCODE_ARP_REQUEST) {
            return -1;
        }

        if (memcmp(rx_arp->target_proto, my_ipv4, IP_ADDR_LEN)) {
            return -1;
        }
        tx_arp->opcode = ARP_OPCODE_ARP_REPLY;
        memcpy(tx_arp->target_hw, rx_arp->sender_hw, HW_ADDR_LEN);
        memcpy(tx_arp->sender_hw, my_mac, HW_ADDR_LEN);
        uint8_t sender_proto[4];
        memcpy(sender_proto, rx_arp->sender_proto, IP_ADDR_LEN);
        memcpy(tx_arp->sender_proto, rx_arp->target_proto, IP_ADDR_LEN);
        memcpy(tx_arp->target_proto, sender_proto, IP_ADDR_LEN);

        tx_len += ARP_HLEN;
        break;
    case ETH_TYPE_IPv4:;
        struct ipv4_header* rx_ipv4 = (struct ipv4_header*)ETH_PAYLOAD(rx_frame);
        struct ipv4_header* tx_ipv4 = (struct ipv4_header*)ETH_PAYLOAD(tx_frame);

        uint32_t src;

        src = rx_ipv4->dst;
        tx_ipv4->dst = rx_ipv4->src;
        tx_ipv4->src = src;
        tx_len += IPv4_HLEN(rx_ipv4);

        if (rx_ipv4->proto == IP_PROTO_ICMP) {
            struct icmp_header* rx_icmp = (struct icmp_header*)IPv4_PAYLOAD(rx_ipv4);

            if (rx_icmp->type != ICMP_TYPE_ECHO_REQUEST) {
                return -1;
            }

            struct icmp_header* tx_icmp = (struct icmp_header*)IPv4_PAYLOAD(tx_ipv4);
            unsigned long icmp_len = IPv4_BODY_LEN(rx_ipv4);

            tx_icmp->type = ICMP_TYPE_ECHO_REPLY;
            icmp_checksum(tx_icmp, icmp_len);
            tx_len += icmp_len;

        } else if (rx_ipv4->proto == IP_PROTO_UDP) {
            struct udp_header* rx_udp = (struct udp_header*)IPv4_PAYLOAD(rx_ipv4);
            if (rx_udp->dstport != 7) {
                return -1;
            }

            struct udp_header* tx_udp = (struct udp_header*)IPv4_PAYLOAD(tx_ipv4);

            uint16_t srcport;
            srcport = rx_udp->dstport;
            tx_udp->dstport = rx_udp->srcport;
            tx_udp->srcport = srcport;
            tx_udp->checksum = 0;
            tx_len += rx_udp->length;
        } else {
            return -1;
        }
        break;
    default:
        return -1;
    }

    if (tx_len < 60) {
        tx_len = 60;
    }
    tx_metadata->frame_length = tx_len;
    // tx_metadata->frame_length = (uint16_t)pkt_length;
    status = api_spi_transmit_frame(tx->data, tx_metadata->frame_length);
    if (status) {
        tx_stats.txFiltered++;
    } else {
        tx_stats.txPackets++;
        tx_stats.txBytes += (tx_metadata->frame_length + 4 + 8);
    }
    return 0;
}

static inline void receive_task_as_client(int sts_flag) {

    struct spi_rx_buffer rx;
    uint16_t bytes_rcv;

    memset(&rx, 0, sizeof(rx));
    bytes_rcv = 0;
    if (api_spi_receive_frame((uint8_t*)rx.data, &bytes_rcv)) {
        rx_stats.rxErrors++;
        return;
    }
    if (bytes_rcv > MAX_BUFFER_LENGTH) {
        rx_stats.rxErrors++;
        return;
    }
    rx_stats.rxPackets++;
    rx_stats.rxBytes += (bytes_rcv + 8);
    rx.metadata.frame_length = bytes_rcv;

    if (sts_flag == 0) {
        packet_parse((struct spi_rx_buffer*)&rx);
    }
}

static void sender_as_client(int sts_flag, int pkt_length) {
    struct spi_tx_buffer tx;
    char src_ip[16];
    char dst_ip[16];
    int status;

    memset(src_ip, 0, sizeof(src_ip));
    memset(dst_ip, 0, sizeof(dst_ip));
    sprintf(src_ip, "%d.%d.%d.%d", my_ipv4[0], my_ipv4[1], my_ipv4[2], my_ipv4[3]);
    sprintf(dst_ip, "%d.%d.%d.%d", dst_ipv4[0], dst_ipv4[1], dst_ipv4[2], dst_ipv4[3]);

    create_arp_request_frame((unsigned char*)tx.data, my_mac, (const char*)src_ip, (const char*)dst_ip);

    dump_buffer((unsigned char*)tx.data, pkt_length);
    printf("\n");

    tx.metadata.frame_length = pkt_length;

    while (tx_thread_run) {
        status = api_spi_transmit_frame(tx.data, tx.metadata.frame_length);
        if (status) {
            tx_stats.txErrors++;
        } else {
            tx_stats.txPackets++;
            tx_stats.txBytes += (tx.metadata.frame_length + 4 + 8);
        }
        if (sts_flag == 0) {
            sleep(1);
        }

        receive_task_as_client(sts_flag);
    }
}

static void sender_as_server(int sts_flag, int pkt_length) {

    while (tx_thread_run) {
        sleep(1);
    }
}

void* sender_thread(void* arg) {
    tx_thread_arg_t* p_arg = (tx_thread_arg_t*)arg;

    printf(">>> %s(mode: %d)\n", __func__, p_arg->mode);

    initialize_statistics(&tx_stats);

    switch (p_arg->mode) {
    case RUN_MODE_CLIENT:
        sender_as_client(p_arg->sts_flag, p_arg->pkt_length);
        break;
    case RUN_MODE_SERVER:
        sender_as_server(p_arg->sts_flag, p_arg->pkt_length);
        break;
    default:
        printf("%s - Unknown mode(%d)\n", __func__, p_arg->mode);
        break;
    }

    printf("<<< %s\n", __func__);

    return NULL;
}

static void test_transmitter(int sts_flag, int pkt_length, uint64_t dmac) {
    struct spi_tx_buffer tx;
    unsigned char dst_mac[HW_ADDR_LEN];
    char src_ip[16];
    char dst_ip[16];
    int status;
    int i;

    memset(src_ip, 0, sizeof(src_ip));
    memset(dst_ip, 0, sizeof(dst_ip));
    sprintf(src_ip, "%d.%d.%d.%d", my_ipv4[0], my_ipv4[1], my_ipv4[2], my_ipv4[3]);
    sprintf(dst_ip, "%d.%d.%d.%d", dst_ipv4[0], dst_ipv4[1], dst_ipv4[2], dst_ipv4[3]);

    for (i = 0; i < HW_ADDR_LEN; i++) {
        dst_mac[i] = (unsigned char)((dmac >> (i * 8)) & 0xff);
    }

    create_udp_packet(my_mac, dst_mac, (const char*)src_ip, (const char*)dst_ip, 1234, 7, (uint16_t)pkt_length,
                      (unsigned char*)tx.data);

    dump_buffer((unsigned char*)tx.data, pkt_length);
    printf("\n");

    tx.metadata.frame_length = pkt_length;

    while (tx_thread_run) {
        status = api_spi_transmit_frame(tx.data, tx.metadata.frame_length);
        if (status) {
            tx_stats.txErrors++;
        } else {
            tx_stats.txPackets++;
            tx_stats.txBytes += (tx.metadata.frame_length + FCS_LENGTH + PREAMBLE_LENGTH);
        }
        if (sts_flag == 0) {
            sleep(1);
        }
    }
}

static inline void receive_task_as_tranceiver(int sts_flag) {

    struct spi_rx_buffer rx;
    uint16_t bytes_rcv;

    memset(&rx, 0, sizeof(rx));
    bytes_rcv = 0;
    if (api_spi_receive_frame((uint8_t*)rx.data, &bytes_rcv)) {
        rx_stats.rxErrors++;
        return;
    }
    if (bytes_rcv > MAX_BUFFER_LENGTH) {
        rx_stats.rxErrors++;
        return;
    }
    rx_stats.rxPackets++;
    rx_stats.rxBytes += (bytes_rcv + 8);
    rx.metadata.frame_length = bytes_rcv;

    if (sts_flag == 0) {
        packet_parse((struct spi_rx_buffer*)&rx);
    }
}

static void test_tranceiver(int sts_flag, int pkt_length, uint64_t dmac) {
    struct spi_tx_buffer tx;
    unsigned char dst_mac[HW_ADDR_LEN];
    char src_ip[16];
    char dst_ip[16];
    int status;
    int i;

    memset(src_ip, 0, sizeof(src_ip));
    memset(dst_ip, 0, sizeof(dst_ip));
    sprintf(src_ip, "%d.%d.%d.%d", my_ipv4[0], my_ipv4[1], my_ipv4[2], my_ipv4[3]);
    sprintf(dst_ip, "%d.%d.%d.%d", dst_ipv4[0], dst_ipv4[1], dst_ipv4[2], dst_ipv4[3]);

    for (i = 0; i < HW_ADDR_LEN; i++) {
        dst_mac[i] = (unsigned char)((dmac >> (i * 8)) & 0xff);
    }

    create_udp_packet(my_mac, dst_mac, (const char*)src_ip, (const char*)dst_ip, 1234, 7, (uint16_t)pkt_length,
                      (unsigned char*)tx.data);

    dump_buffer((unsigned char*)tx.data, pkt_length);
    printf("\n");

    tx.metadata.frame_length = pkt_length;

    while (tx_thread_run) {
        status = api_spi_transmit_frame(tx.data, tx.metadata.frame_length);
        if (status) {
            tx_stats.txErrors++;
        } else {
            tx_stats.txPackets++;
            tx_stats.txBytes += (tx.metadata.frame_length + FCS_LENGTH + PREAMBLE_LENGTH);
        }
        if (sts_flag == 0) {
            sleep(1);
        }

        receive_task_as_client(sts_flag);
    }
}

void* transmitter_thread(void* arg) {
    tx_thread_arg_t* p_arg = (tx_thread_arg_t*)arg;

    printf(">>> %s(mode: %d)\n", __func__, p_arg->mode);

    initialize_statistics(&tx_stats);

    switch (p_arg->mode) {
    case TEST_MODE_TRANSMITTER:
        test_transmitter(p_arg->sts_flag, p_arg->pkt_length, p_arg->dmac);
        break;
    case TEST_MODE_TRANCEIVER:
        test_tranceiver(p_arg->sts_flag, p_arg->pkt_length, p_arg->dmac);
        break;
    default:
        printf("%s - Unknown mode(%d)\n", __func__, p_arg->mode);
        break;
    }

    printf("<<< %s\n", __func__);

    return NULL;
}

void calculate_stats() {
    unsigned long long usec = currTv - lastTv;

    cs.rxPackets = rx_stats.rxPackets;
    cs.rxBytes = rx_stats.rxBytes;
    cs.txPackets = tx_stats.txPackets;
    cs.txBytes = tx_stats.txBytes;
    cs.rxPps = ((rx_stats.rxPackets - os.rxPackets) * 1000000) / usec;
    cs.rxBps = ((rx_stats.rxBytes - os.rxBytes) * 8000000) / usec;
    cs.rxPBps = ((rx_stats.rxBytes - (cs.rxPps * (FCS_LENGTH + PREAMBLE_LENGTH)) - os.rxBytes) * 8000000) /
                usec; /* remove FCS & Preamble */
    cs.txPps = ((tx_stats.txPackets - os.txPackets) * 1000000) / usec;
    cs.txBps = ((tx_stats.txBytes - os.txBytes) * 8000000) / usec;
    cs.txPBps = ((tx_stats.txBytes - (cs.txPps * (FCS_LENGTH + PREAMBLE_LENGTH)) - os.txBytes) * 8000000) /
                usec; /* remove FCS & Preamble */
    memcpy(&os, &cs, sizeof(stats_t));
}

void print_counter() {

    printf("%20s", counter_name[COUNTERS_RXPACKETS]);
    printf("%16llu\n", rx_stats.rxPackets);
    printf("%20s", counter_name[COUNTERS_RXBYTES]);
    printf("%16llu\n", rx_stats.rxBytes);
    printf("%20s", counter_name[COUNTERS_RXERRORS]);
    printf("%16llu\n", rx_stats.rxErrors);
    printf("%20s", counter_name[COUNTERS_RXNOBUF]);
    printf("%16llu\n", rx_stats.rxNoBuffer);
    printf("%20s", counter_name[COUNTERS_RXPPS]);
    printf("%16llu\n", cs.rxPps);
    printf("%20s", counter_name[COUNTERS_RXBPS]);
    printf("%16llu\n", cs.rxBps);
    printf("%20s", counter_name[COUNTERS_RXPBPS]);
    printf("%16llu\n", cs.rxPBps);
    printf("%20s", counter_name[COUNTERS_TXPACKETS]);
    printf("%16llu\n", tx_stats.txPackets);
    printf("%20s", counter_name[COUNTERS_TXBYTES]);
    printf("%16llu\n", tx_stats.txBytes);
    printf("%20s", counter_name[COUNTERS_TXFILTERED]);
    printf("%16llu\n", tx_stats.txFiltered);
    printf("%20s", counter_name[COUNTERS_TXERRORS]);
    printf("%16llu\n", tx_stats.txErrors);
    printf("%20s", counter_name[COUNTERS_TXPPS]);
    printf("%16llu\n", cs.txPps);
    printf("%20s", counter_name[COUNTERS_TXBPS]);
    printf("%16llu\n", cs.txBps);
    printf("%20s", counter_name[COUNTERS_TXPBPS]);
    printf("%16llu\n", cs.txPBps);
}

void CalExecTimeInfo(int seed, execTime_t* info) {

    int day = 0;
    int hour = 0;
    int min = 0;
    int sec = 0;
    int tmp;

    day = seed / DAY;
    tmp = seed % DAY;
    if (tmp) {
        seed = tmp;
        hour = seed / HOUR;
        tmp = seed % HOUR;
        if (tmp) {
            seed = tmp;
            min = seed / MIN;
            sec = seed % MIN;
        }
    }

    info->day = day;
    info->hour = hour;
    info->min = min;
    info->sec = sec;
}

void print_stats() {

    int totalRunTime;
    time_t cur_time;
    struct timeval tv;
    execTime_t runTimeInfo;

    int systemRet = system("clear");
    if (systemRet == -1) {
    }

    gettimeofday(&tv, NULL);
    currTv = tv.tv_sec * 1000000 + tv.tv_usec;

    calculate_stats();

    time(&cur_time);
    totalRunTime = cur_time - start_time;
    CalExecTimeInfo(totalRunTime, &runTimeInfo);

    print_counter();
    printf("%20s %u sec (%d-%d:%d:%d)\n", "Running", (unsigned int)totalRunTime, runTimeInfo.day, runTimeInfo.hour,
           runTimeInfo.min, runTimeInfo.sec);

    lastTv = currTv;
}

void* stats_thread(void* arg) {

    stats_thread_arg_t* p_arg = (stats_thread_arg_t*)arg;
    struct timeval previousTime, currentTime;
    double elapsedTime;
    struct timeval tv;

    printf(">>> %s(mode: %d)\n", __func__, p_arg->mode);

    time(&start_time);
    gettimeofday(&tv, NULL);
    lastTv = tv.tv_sec * 1000000 + tv.tv_usec;

    gettimeofday(&previousTime, NULL);

    while (stats_thread_run) {
        gettimeofday(&currentTime, NULL);
        elapsedTime =
            (currentTime.tv_sec - previousTime.tv_sec) + (currentTime.tv_usec - previousTime.tv_usec) / 1000000.0;

        if (elapsedTime >= 1.0) {
            print_stats();
            memcpy(&previousTime, &currentTime, sizeof(struct timeval));
        }
    }

    print_stats();
    printf("<<< %s\n", __func__);

    return NULL;
}
