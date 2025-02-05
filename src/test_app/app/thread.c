#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include "arp.h"
#include "ethernet.h"
#include "icmp.h"
#include "ip.h"
#include "ipv4.h"
#include "udp.h"
#include "xbase-t1s.h"
#include "xbase_common.h"

int rx_thread_run = 1;
int tx_thread_run = 1;

static CircularQueue_t g_queue;
static CircularQueue_t* queue = NULL;

stats_t rx_stats;
stats_t tx_stats;

void initialize_queue(CircularQueue_t* p_queue) {
    queue = p_queue;

    p_queue->front = 0;
    p_queue->rear = -1;
    p_queue->count = 0;
    pthread_mutex_init(&p_queue->mutex, NULL);
}

static int isQueueEmpty() {
    return (queue->count == 0);
}

static int isQueueFull() {
    return (queue->count == NUMBER_OF_QUEUE);
}

int getQueueCount() {
    return queue->count;
}

static void xbuffer_enqueue(QueueElement element) {
    pthread_mutex_lock(&queue->mutex);

    if (isQueueFull()) {
        // debug_printf("Queue is full. Cannot xbuffer_enqueue.\n");
        pthread_mutex_unlock(&queue->mutex);
        return;
    }

    queue->rear = (queue->rear + 1) % NUMBER_OF_QUEUE;
    queue->elements[queue->rear] = element;
    queue->count++;

    pthread_mutex_unlock(&queue->mutex);
}

static QueueElement xbuffer_dequeue() {
    pthread_mutex_lock(&queue->mutex);

    if (isQueueEmpty()) {
        // debug_printf("Queue is empty. Cannot xbuffer_dequeue.\n");
        pthread_mutex_unlock(&queue->mutex);
        return EMPTY_ELEMENT;
    }

    QueueElement dequeuedElement = queue->elements[queue->front];
    queue->front = (queue->front + 1) % NUMBER_OF_QUEUE;
    queue->count--;

    pthread_mutex_unlock(&queue->mutex);

    return dequeuedElement;
}

void initialize_statistics(stats_t* p_stats) {

    memset(p_stats, 0, sizeof(stats_t));
}

static void receiver_as_client() {

    BUF_POINTER buffer;
    int bytes_rcv;

    printf(">>> %s\n", __func__);

    while (rx_thread_run) {
        buffer = buffer_pool_alloc();
        if (buffer == NULL) {
            // printf("FAILURE: Could not buffer_pool_alloc.\n");
            rx_stats.rxNoBuffer++;
            continue;
        }

        bytes_rcv = 0;
#if 0
        spi_receive_frame
        if (xdma_api_read_to_buffer_with_fd(devname, fd, buffer, size, &bytes_rcv)) {
            if (buffer_pool_free(buffer)) {
                // debug_printf("FAILURE: Could not buffer_pool_free.\n");
            }
            rx_stats.rxErrors++;
            continue;
        }
        if (bytes_rcv > MAX_BUFFER_LENGTH) {
            if (buffer_pool_free(buffer)) {
                // debug_printf("FAILURE: Could not buffer_pool_free.\n");
            }
            rx_stats.rxErrors++;
            continue;
        }
        rx_stats.rxPackets++;
        rx_stats.rxBytes += bytes_rcv;

        xbuffer_enqueue((QueueElement)buffer);
#endif
    }
    printf("<<< %s\n", __func__);
}

void receiver_as_server() {

    printf(">>> %s\n", __func__);

    while (rx_thread_run) {

#if 0
        if (xdma_api_read_to_buffer_with_fd(devname, fd, buffer, size, &bytes_rcv)) {
            continue;
        }

        if (size != bytes_rcv) {
            // debug_printf("FAILURE: size(%ld) and bytes_rcv(%ld) are different.\n", size, bytes_rcv);
            rx_stats.rxErrors++;
            continue;
        }

        if (memcmp((const void*)data, (const void*)buffer, size)) {
            // debug_printf("FAILURE: data(%p) and buffer(%p) are different.\n", data, buffer);
            rx_stats.rxErrors++;
            continue;
        }

        rx_stats.rxPackets++;
        rx_stats.rxBytes += bytes_rcv;
#endif
    }
    printf("<<< %s\n", __func__);
}

void* receiver_thread(void* arg) {

    rx_thread_arg_t* p_arg = (rx_thread_arg_t*)arg;

    printf(">>> %s(mode: %d)\n", __func__, p_arg->mode);

    initialize_queue(&g_queue);
    initialize_statistics(&rx_stats);

    switch (p_arg->mode) {
    case RUN_MODE_CLIENT:
        receiver_as_client();
        break;
    case RUN_MODE_SERVER:
        receiver_as_server();
        break;
    default:
        printf("%s - Unknown mode(%d)\n", __func__, p_arg->mode);
        break;
    }

    pthread_mutex_destroy(&g_queue.mutex);

    printf("<<< %s\n", __func__);

    return NULL;
}

static unsigned char my_mac[HW_ADDR_LEN];
static unsigned char my_ipv4[IP_ADDR_LEN];

int api_spi_transmit_frame(uint8_t* packet, uint16_t length);

static int process_send_packet(struct spi_rx_buffer* rx) {
    uint8_t* buffer = (uint8_t*)rx;
    int tx_len;
    struct spi_tx_buffer* tx =
        (struct spi_tx_buffer*)(buffer + sizeof(struct rx_metadata) - sizeof(struct tx_metadata));
    struct tx_metadata* tx_metadata = &tx->metadata;
    uint8_t* rx_frame = (uint8_t*)&rx->data;
    uint8_t* tx_frame = (uint8_t*)&tx->data;
    struct ethernet_header* rx_eth = (struct ethernet_header*)rx_frame;
    struct ethernet_header* tx_eth = (struct ethernet_header*)tx_frame;

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

            struct udp_header* tx_udp = IPv4_PAYLOAD(tx_ipv4);

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
        printf("Unknown type: %04x\n", rx_eth->type);
        return -1;
    }

    tx_metadata->frame_length = tx_len;
    api_spi_transmit_frame(tx->data, tx_metadata->frame_length);
    return 0;
}

#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>

#define ETH_ALEN 6
#define ETH_HLEN 14
#define ARP_HLEN 28

struct ethhdr {
    unsigned char h_dest[ETH_ALEN];
    unsigned char h_source[ETH_ALEN];
    unsigned short h_proto;
};

struct arphdr {
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

void create_arp_request_frame(unsigned char* frame, const char* src_ip, const char* dst_ip) {
    struct ethhdr* eth = (struct ethhdr*)frame;
    struct arphdr* arp = (struct arphdr*)(frame + ETH_HLEN);

    /* 이더넷 헤더 설정 */
    memset(eth->h_dest, 0xFF, ETH_ALEN); /* 브로드캐스트 주소 */
    for (int i = 0; i < ETH_ALEN; i++) {
        eth->h_source[i] = my_mac[i];
    }
#if 0
    sscanf(src_mac, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &eth->h_source[0], &eth->h_source[1], &eth->h_source[2],
           &eth->h_source[3], &eth->h_source[4], &eth->h_source[5]);
#endif
    eth->h_proto = htons(0x0806); /* ARP 프로토콜 */

    /* ARP 헤더 설정 */
    arp->ar_hrd = htons(1);      /* 이더넷 */
    arp->ar_pro = htons(0x0800); /* IPv4 */
    arp->ar_hln = 6;             /* MAC 주소 길이 */
    arp->ar_pln = 4;             /* IP 주소 길이 */
    arp->ar_op = htons(1);       /* ARP 요청 */

    /* 송신자 MAC 및 IP 주소 설정 */
    memcpy(arp->ar_sha, eth->h_source, ETH_ALEN);
    inet_pton(AF_INET, src_ip, arp->ar_sip);

    /* 목적지 MAC 주소는 알 수 없으므로 0으로 설정 */
    memset(arp->ar_tha, 0, ETH_ALEN);

    /* 목적지 IP 주소 설정 */
    inet_pton(AF_INET, dst_ip, arp->ar_tip);

    /* 패딩 추가 (최소 프레임 크기 64바이트를 맞추기 위해) */
    memset(frame + ETH_HLEN + ARP_HLEN, 0, 18);
}

void dump_buffer(unsigned char* buffer, int len) {

    for (int idx = 0; idx < len; idx++) {
        if ((idx % 16) == 0) {
            printf("\n  ");
        }
        printf("0x%02x ", buffer[idx] & 0xFF);
    }
    printf("\n");
}

static void sender_as_client() {
    struct spi_tx_buffer tx;
    const char* dst_ip = "192.168.10.11"; // 예시 목적지 IP 주소
    char src_ip[100];

    memset(src_ip, 0, sizeof(src_ip));
    sprintf(src_ip, "%d.%d.%d.%d", my_ipv4[0], my_ipv4[1], my_ipv4[2], my_ipv4[3]);

    create_arp_request_frame((unsigned char*)tx.data, (const char*)src_ip, dst_ip);

    dump_buffer((unsigned char*)tx.data, 64);
    printf("\n");
    printf("\n");
    printf("\n");

    tx.metadata.frame_length = 64;

    while (tx_thread_run) {
        api_spi_transmit_frame(tx.data, tx.metadata.frame_length);

        sleep(1);
    }
}

static void sender_as_server() {

    QueueElement buffer = NULL;
    int status;

    while (tx_thread_run) {
        buffer = NULL;
        buffer = xbuffer_dequeue();
        if (buffer == NULL) {
            continue;
        }

        status = process_send_packet((struct spi_rx_buffer*)buffer);
        if (status == -1) {
            tx_stats.txFiltered++;
        }

        buffer_pool_free((BUF_POINTER)buffer);
    }
}

void* sender_thread(void* arg) {

    int i;

    tx_thread_arg_t* p_arg = (tx_thread_arg_t*)arg;

    printf(">>> %s(mode: %d)\n", __func__, p_arg->mode);

    for (i = 0; i < HW_ADDR_LEN; i++) {
        my_mac[HW_ADDR_LEN - 1 - i] = (unsigned char)((p_arg->mac >> (i * 8)) & 0xff);
    }
    for (i = 0; i < IP_ADDR_LEN; i++) {
        my_ipv4[IP_ADDR_LEN - 1 - i] = (unsigned char)((p_arg->ipv4 >> (i * 8)) & 0xff);
    }

    initialize_statistics(&tx_stats);

    switch (p_arg->mode) {
    case RUN_MODE_CLIENT:
        sender_as_client();
        break;
    case RUN_MODE_SERVER:
        sender_as_server();
        break;
    default:
        printf("%s - Unknown mode(%d)\n", __func__, p_arg->mode);
        break;
    }

    printf("<<< %s\n", __func__);

    return NULL;
}
