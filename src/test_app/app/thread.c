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

extern pthread_mutex_t spi_mutex;

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
        //        pthread_mutex_lock(&spi_mutex);
        if (api_spi_receive_frame((uint8_t*)rx.data, &bytes_rcv)) {
            //            pthread_mutex_unlock(&spi_mutex);
            rx_stats.rxErrors++;
            continue;
        }
        //        pthread_mutex_unlock(&spi_mutex);
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

#if 1
void print_mac(uint8_t* mac) {
    printf("%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void print_ip(uint8_t* ip_bytes) {
    printf("%d.%d.%d.%d", ip_bytes[0], ip_bytes[1], ip_bytes[2], ip_bytes[3]);
}

void process_packet(uint8_t* packet, int packet_len) {
    struct ethernet_header* eth_header = (struct ethernet_header*)packet;

    // 이더넷 타입이 ARP인지 확인 (0x0806)
    if (eth_header->type != 0x0806) {
        return;
    }

    struct arp_header* arp = (struct arp_header*)(packet + sizeof(struct ethernet_header));

    // ARP 응답 패킷인지 확인 (opcode == 2)
    if (arp->opcode != 2) {
        return;
    }

    printf("ARP Response Packet Detected:\n");
    printf("Hardware Type: 0x%04x\n", arp->hw_type);
    printf("Protocol Type: 0x%04x\n", arp->proto_type);
    printf("Hardware Size: %d\n", arp->hw_size);
    printf("Protocol Size: %d\n", arp->proto_size);
    printf("Opcode: %d\n", arp->opcode);

    printf("Sender MAC: ");
    print_mac(arp->sender_hw);
    printf("\n");

    printf("Sender IP: ");
    print_ip(arp->sender_proto);
    printf("\n");

    printf("Target MAC: ");
    print_mac(arp->target_hw);
    printf("\n");

    printf("Target IP: ");
    print_ip(arp->target_proto);
    printf("\n");
}

#endif

static void receiver_as_client(int sts_flag, int pkt_length) {

#if 0
    struct spi_rx_buffer rx;
    uint16_t bytes_rcv;
#endif

    printf(">>> %s\n", __func__);

    while (rx_thread_run) {
        ;
#if 0
        if (sts_flag == 0) {
            sleep(1);
        }
        memset(&rx, 0, sizeof(rx));
        bytes_rcv = 0;
        pthread_mutex_lock(&spi_mutex);
        if (api_spi_receive_frame((uint8_t*)rx.data, &bytes_rcv)) {
            pthread_mutex_unlock(&spi_mutex);
            rx_stats.rxErrors++;
            continue;
        }
        pthread_mutex_unlock(&spi_mutex);
        if (bytes_rcv > MAX_BUFFER_LENGTH) {
            rx_stats.rxErrors++;
            continue;
        }
        rx_stats.rxPackets++;
        rx_stats.rxBytes += bytes_rcv;
        rx.metadata.frame_length = bytes_rcv;

        if (sts_flag == 0) {
            process_packet((uint8_t*)rx.data, (int)bytes_rcv);
        }
#else
        sleep(1);
#endif
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

    // make ethernet frame
    memcpy(&(tx_eth->dmac), &(rx_eth->smac), 6);
    memcpy(&(tx_eth->smac), my_mac, 6);

    tx_len = ETH_HLEN;

    // do arp, udp echo, etc.
    switch (rx_eth->type) {
    case ETH_TYPE_ARP: // arp
        ;
        struct arp_header* rx_arp = (struct arp_header*)ETH_PAYLOAD(rx_frame);
        struct arp_header* tx_arp = (struct arp_header*)ETH_PAYLOAD(tx_frame);
        if (rx_arp->opcode != ARP_OPCODE_ARP_REQUEST) {
            return -1;
        }

        if (memcmp(rx_arp->target_proto, my_ipv4, IP_ADDR_LEN)) {
            return -1;
        }
        // make arp packet
        // tx_arp->hw_type = rx_arp->hw_type;
        // tx_arp->proto_type = rx_arp->proto_type;
        // tx_arp->hw_size = rx_arp->hw_size;
        // tx_arp->proto_size = rx_arp->proto_size;
        tx_arp->opcode = ARP_OPCODE_ARP_REPLY;
        memcpy(tx_arp->target_hw, rx_arp->sender_hw, HW_ADDR_LEN);
        memcpy(tx_arp->sender_hw, my_mac, HW_ADDR_LEN);
        uint8_t sender_proto[4];
        memcpy(sender_proto, rx_arp->sender_proto, IP_ADDR_LEN);
        memcpy(tx_arp->sender_proto, rx_arp->target_proto, IP_ADDR_LEN);
        memcpy(tx_arp->target_proto, sender_proto, IP_ADDR_LEN);

        tx_len += ARP_HLEN;
        break;
    case ETH_TYPE_IPv4: // ip
        ;
        struct ipv4_header* rx_ipv4 = (struct ipv4_header*)ETH_PAYLOAD(rx_frame);
        struct ipv4_header* tx_ipv4 = (struct ipv4_header*)ETH_PAYLOAD(tx_frame);

        uint32_t src;

        // Fill IPv4 header
        // memcpy(tx_ipv4, rx_ipv4, IPv4_HLEN(rx_ipv4));
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

            // Fill ICMP header and body
            // memcpy(tx_icmp, rx_icmp, icmp_len);
            tx_icmp->type = ICMP_TYPE_ECHO_REPLY;
            icmp_checksum(tx_icmp, icmp_len);
            tx_len += icmp_len;

        } else if (rx_ipv4->proto == IP_PROTO_UDP) {
            struct udp_header* rx_udp = (struct udp_header*)IPv4_PAYLOAD(rx_ipv4);
            if (rx_udp->dstport != 7) {
                return -1;
            }

            struct udp_header* tx_udp = (struct udp_header*)IPv4_PAYLOAD(tx_ipv4);

            // Fill UDP header
            // memcpy(tx_udp, rx_udp, rx_udp->length);
            uint16_t srcport;
            srcport = rx_udp->dstport;
            tx_udp->dstport = rx_udp->srcport;
            tx_udp->srcport = srcport;
            tx_udp->checksum = 0;
            tx_len += rx_udp->length; // UDP.length contains header length
        } else {
            return -1;
        }
        break;
    default:
        //        printf("Unknown type: %04x\n", rx_eth->type);
        return -1;
    }

    if (tx_len < 60) {
        tx_len = 60;
    }
    // tx_metadata->frame_length = tx_len;
    tx_metadata->frame_length = (uint16_t)pkt_length;
    // dump_buffer((unsigned char*)tx->data, tx_len);
    //    pthread_mutex_lock(&spi_mutex);
    status = api_spi_transmit_frame(tx->data, tx_metadata->frame_length);
    //    pthread_mutex_unlock(&spi_mutex);
    if (status) {
        tx_stats.txFiltered++;
    } else {
        tx_stats.txPackets++;
        tx_stats.txBytes += (tx_metadata->frame_length + 4 + 8);
    }
    return 0;
}

#if 0
void create_arp_request_frame(unsigned char* frame, unsigned char * smac, const char* src_ip, const char* dst_ip) {
    struct ethernet_header* eth = (struct ethernet_header*)frame;
    struct arp_header* arp = (struct arp_header*)(frame + ETH_HLEN);

    /* 이더넷 헤더 설정 */
    memset(eth->dmac, 0xFF, ETH_ALEN); /* 브로드캐스트 주소 */
    for (int i = 0; i < ETH_ALEN; i++) {
        eth->smac[i] = my_mac[i];
    }
    eth->type = 0x0806; /* ARP 프로토콜 */

    /* ARP 헤더 설정 */
    arp->hw_type = 1;         /* 이더넷 */
    arp->proto_type = 0x0800; /* IPv4 */
    arp->hw_size = 6;         /* MAC 주소 길이 */
    arp->proto_size = 4;      /* IP 주소 길이 */
    arp->opcode = 1;          /* ARP 요청 */

    /* 송신자 MAC 및 IP 주소 설정 */
    memcpy(arp->sender_hw, eth->smac, ETH_ALEN);
    inet_pton(AF_INET, src_ip, arp->sender_proto);

    /* 목적지 MAC 주소는 알 수 없으므로 0으로 설정 */
    memset(arp->target_hw, 0, ETH_ALEN);

    /* 목적지 IP 주소 설정 */
    inet_pton(AF_INET, dst_ip, arp->target_proto);

    /* 패딩 추가 (최소 프레임 크기 64바이트를 맞추기 위해) */
    memset(frame + ETH_HLEN + ARP_HLEN, 0, 18);
}
#endif

#if 0
void packet_handler(const u_char *packet) {
    struct ether_header *eth_header;
    struct ip *ip_header;
    struct tcphdr *tcp_header;

    // 이더넷 헤더 파싱
    eth_header = (struct ether_header *)packet;

    printf("Ethernet Header:\n");
    printf("Source MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
           eth_header->ether_shost[0], eth_header->ether_shost[1],
           eth_header->ether_shost[2], eth_header->ether_shost[3],
           eth_header->ether_shost[4], eth_header->ether_shost[5]);
    printf("Destination MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
           eth_header->ether_dhost[0], eth_header->ether_dhost[1],
           eth_header->ether_dhost[2], eth_header->ether_dhost[3],
           eth_header->ether_dhost[4], eth_header->ether_dhost[5]);

    // IP 패킷인 경우에만 처리
    if (ntohs(eth_header->ether_type) == ETHERTYPE_IP) {
        // IP 헤더 파싱
        ip_header = (struct ip *)(packet + sizeof(struct ether_header));

        printf("\nIP Header:\n");
        printf("Source IP: %s\n", inet_ntoa(ip_header->ip_src));
        printf("Destination IP: %s\n", inet_ntoa(ip_header->ip_dst));

        // TCP 패킷인 경우에만 처리
        if (ip_header->ip_p == IPPROTO_TCP) {
            // TCP 헤더 파싱
            tcp_header = (struct tcphdr *)(packet + sizeof(struct ether_header) + ip_header->ip_hl * 4);

            printf("\nTCP Header:\n");
            printf("Source Port: %d\n", ntohs(tcp_header->th_sport));
            printf("Destination Port: %d\n", ntohs(tcp_header->th_dport));
        }
    }

    printf("\n-----------------------------\n");
}
#endif

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
        process_packet((uint8_t*)rx.data, (int)bytes_rcv);
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
#if 0
        if (sts_flag == 0) {
            sleep(1);
        };
#endif
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

void calculate_stats() {
    unsigned long long usec = currTv - lastTv;

    cs.rxPackets = rx_stats.rxPackets;
    cs.rxBytes = rx_stats.rxBytes;
    cs.txPackets = tx_stats.txPackets;
    cs.txBytes = tx_stats.txBytes;
    cs.rxPps = ((rx_stats.rxPackets - os.rxPackets) * 1000000) / usec;
    cs.rxBps = ((rx_stats.rxBytes - os.rxBytes) * 8000000) / usec;
    cs.rxPBps = ((rx_stats.rxBytes - (cs.rxPps * 12) - os.rxBytes) * 8000000) / usec; /* remove FCS & Preamble */
    cs.txPps = ((tx_stats.txPackets - os.txPackets) * 1000000) / usec;
    cs.txBps = ((tx_stats.txBytes - os.txBytes) * 8000000) / usec;
    cs.txPBps = ((tx_stats.txBytes - (cs.txPps * 12) - os.txBytes) * 8000000) / usec; /* remove FCS & Preamble */
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
