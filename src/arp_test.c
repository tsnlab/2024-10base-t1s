#include "arp_test.h"

#include "arch.h"
#include "spi.h"

// ARP table
struct arp_entry g_arp_table[MAX_ENTRIES];
int g_arp_table_count = 0;

// Function to add an entry to the ARP table
static int add_to_arp_table(unsigned char* ip, unsigned char* mac) {
    int ret = -ARP_E_REQUEST_FAILED;
    int i;

    for (i = 0; i < g_arp_table_count; i++) {
        if (memcmp(g_arp_table[i].ip, ip, 4) == 0) {
            /* IP already exists, update MAC */
            memcpy(g_arp_table[i].mac, mac, 6);
        }
    }
    if (g_arp_table_count < MAX_ENTRIES) {
        memcpy(g_arp_table[g_arp_table_count].ip, ip, 4);
        memcpy(g_arp_table[g_arp_table_count].mac, mac, 6);
        g_arp_table_count++;
        ret = ARP_E_SUCCESS;
    } else {
        printf("ARP table is full\n");
    }

    return ret;
}

// Function to print the ARP table
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

static int arp_request(uint8_t* arp_request_buffer, uint16_t length) {
    uint8_t txbuffer[HEADER_SIZE + MAX_PAYLOAD_BYTE] = {
        0,
    };
    uint8_t rxbuffer[MAX_PAYLOAD_BYTE + FOOTER_SIZE] = {
        0,
    };
    int i = 0;
    int j = 0;
    static union data_header data_transfer_header = {
        0,
    };

    // header setting
    memset(&data_transfer_header.tx_header_bits, 0, sizeof(data_transfer_header.tx_header_bits));

    data_transfer_header.tx_header_bits.dnc = DNC_COMMANDTYPE_DATA;
    data_transfer_header.tx_header_bits.seq = 1;                   // TODO: check this if there are multiiple chunks
    data_transfer_header.tx_header_bits.norx = 0;                  // No Receive
    data_transfer_header.tx_header_bits.dv = 1;                    // Data Valid
    data_transfer_header.tx_header_bits.sv = 1;                    // start chunk
    data_transfer_header.tx_header_bits.ev = 1;                    // end chunk (single chunk)
    data_transfer_header.tx_header_bits.ebo = PACKET_SIZE_ARP - 1; // TODO: check this if there are multiiple chunks
    data_transfer_header.tx_header_bits.p = ((get_parity(data_transfer_header.data_frame_head) == 0) ? 1 : 0);

    // copy header
    data_transfer_header.data_frame_head = htonl(data_transfer_header.data_frame_head);
    memcpy(txbuffer, &data_transfer_header.data_frame_head, HEADER_SIZE);

    // copy ARP data
    memcpy(&txbuffer[HEADER_SIZE], arp_request_buffer, length);

    // transfer
    if (SPI_E_SUCCESS != spi_transfer((uint8_t*)&rxbuffer[0], (uint8_t*)&txbuffer[0], sizeof(txbuffer))) {
        return -ARP_E_REQUEST_FAILED;
    }

    printf_debug("txbuffer when Requesting: \n");
    for (i = 0; i < 7; i++) {
        for (j = 0; j < 10; j++) {
            if (i * 10 + j >= MAX_PAYLOAD_BYTE + HEADER_SIZE) {
                printf_debug("...");
                break;
            }
            printf_debug("%02x ", txbuffer[i * 10 + j]);
        }
        printf_debug("\n");
    }
    printf_debug("\n");

    printf_debug("rxbuffer when Requesting: \n");
    for (i = 0; i < 7; i++) {
        for (j = 0; j < 10; j++) {
            if (i * 10 + j >= MAX_PAYLOAD_BYTE + FOOTER_SIZE) {
                printf_debug("...");
                break;
            }
            printf_debug("%02x ", rxbuffer[i * 10 + j]);
        }
        printf_debug("\n");
    }
    printf_debug("\n");

    return ARP_E_SUCCESS;
}

static int arp_reply(uint8_t* arp_reply_buffer, uint16_t* length) {
    uint8_t txbuffer[HEADER_SIZE + MAX_PAYLOAD_BYTE] = {
        0,
    };
    uint8_t rxbuffer[MAX_PAYLOAD_BYTE + FOOTER_SIZE] = {
        0,
    };
    static union data_header data_transfer_header = {
        0,
    };
    union data_footer data_transfer_rx_footer;
    int i = 0;
    int j = 0;

    // receive dummy header setting
    data_transfer_header.tx_header_bits.dnc = DNC_COMMANDTYPE_DATA;
    data_transfer_header.tx_header_bits.seq = 0;
    data_transfer_header.tx_header_bits.norx = 0;
    data_transfer_header.tx_header_bits.dv = 0; // receive mode
    data_transfer_header.tx_header_bits.sv = 0;
    data_transfer_header.tx_header_bits.ev = 0;
    data_transfer_header.tx_header_bits.ebo = 0;
    data_transfer_header.tx_header_bits.tsc = 0;
    data_transfer_header.tx_header_bits.p = 0;
    data_transfer_header.tx_header_bits.p = ((get_parity(data_transfer_header.data_frame_head) == 0) ? 1 : 0);

    // copy header
    data_transfer_header.data_frame_head = htonl(data_transfer_header.data_frame_head);
    memcpy(txbuffer, &data_transfer_header.data_frame_head, HEADER_SIZE);

    // receive buffer
    spi_transfer((uint8_t*)&rxbuffer[0], (uint8_t*)&txbuffer[0], sizeof(txbuffer));

    printf_debug("txbuffer when Receiving: \n");
    for (i = 0; i < 7; i++) {
        for (j = 0; j < 10; j++) {
            if (i * 10 + j >= MAX_PAYLOAD_BYTE + HEADER_SIZE) {
                printf_debug("...");
                break;
            }
            printf_debug("%02x ", txbuffer[i * 10 + j]);
        }
        printf_debug("\n");
    }
    printf_debug("\n");

    printf_debug("rxbuffer when Receiving: \n");
    for (i = 0; i < 7; i++) {
        for (j = 0; j < 10; j++) {
            if (i * 10 + j >= MAX_PAYLOAD_BYTE + FOOTER_SIZE) {
                printf_debug("...");
                break;
            }
            printf_debug("%02x ", rxbuffer[i * 10 + j]);
        }
        printf_debug("\n");
    }
    printf_debug("\n");

    // Footer check
    memcpy((uint8_t*)&data_transfer_rx_footer.data_frame_foot, &rxbuffer[MAX_PAYLOAD_BYTE], FOOTER_SIZE);
    data_transfer_rx_footer.data_frame_foot = ntohl(data_transfer_rx_footer.data_frame_foot);

    printf_debug("data_transfer_rx_footer: \n");
    printf_debug("exst: %u\n", data_transfer_rx_footer.rx_footer_bits.exst);
    printf_debug("hdrb: %u\n", data_transfer_rx_footer.rx_footer_bits.hdrb);
    printf_debug("sync: %u\n", data_transfer_rx_footer.rx_footer_bits.sync);
    printf_debug("dv: %u\n", data_transfer_rx_footer.rx_footer_bits.dv);
    printf_debug("Raw value: 0x%08x\n", data_transfer_rx_footer.data_frame_foot);

    // receive data validation
    if (data_transfer_rx_footer.rx_footer_bits.dv && !data_transfer_rx_footer.rx_footer_bits.exst) {

        uint16_t actual_length = data_transfer_rx_footer.rx_footer_bits.ebo + 1;
        memcpy(arp_reply_buffer, &rxbuffer[0], actual_length);
        *length = actual_length;

        // Ethernet packet check
        printf("Destination MAC: ");
        if (arp_reply_buffer[0] == 0xFF && arp_reply_buffer[1] == 0xFF && arp_reply_buffer[2] == 0xFF &&
            arp_reply_buffer[3] == 0xFF && arp_reply_buffer[4] == 0xFF && arp_reply_buffer[5] == 0xFF) {
            printf("Broadcast\n");
        } else {
            printf("%02x:%02x:%02x:%02x:%02x:%02x\n", // clang-format off
                    arp_reply_buffer[0], arp_reply_buffer[1], arp_reply_buffer[2],
                    arp_reply_buffer[3], arp_reply_buffer[4], arp_reply_buffer[5]);
        }
        printf("Source MAC: ");
        printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                arp_reply_buffer[6], arp_reply_buffer[7], arp_reply_buffer[8],
                arp_reply_buffer[9], arp_reply_buffer[10], arp_reply_buffer[11]);
        printf("EtherType: %02x%02x",
                arp_reply_buffer[12], arp_reply_buffer[13]); // clang-format on
        if (arp_reply_buffer[12] == 0x08 && arp_reply_buffer[13] == 0x06) {
            printf("(ARP)\n");
        } else {
            printf("(Unknown)\n");
        }

        return ARP_E_SUCCESS;
    }

    return -ARP_E_REPLY_FAILED;
}

int arp_test(int plca_mode) {
    int ret = -ARP_E_REQUEST_FAILED;
    uint16_t received_length = 0;
    unsigned char buffer[MAX_PAYLOAD_BYTE] = {
        0,
    };
    struct ethhdr* eth = (struct ethhdr*)buffer;
    struct arphdr_ipv4* arp = (struct arphdr_ipv4*)(buffer + sizeof(struct ethhdr));

    if (plca_mode == PLCA_MODE_COORDINATOR) {
        // Fill Ethernet header
        memset(eth->h_dest, 0xFF, ETH_ALEN);                         // Broadcast
        memcpy(eth->h_source, "\xd8\x3a\xdd\x44\xab\x0f", ETH_ALEN); // Replace with your MAC
        eth->h_proto = htons(ETH_P_ARP);                             // ARP Ethertype

        // Fill ARP header
        arp->arp.ar_hrd = htons(1);                                           // Ethernet
        arp->arp.ar_pro = htons(ETH_P_IP);                                    // IPv4
        arp->arp.ar_hln = ETH_ALEN;                                           // MAC size
        arp->arp.ar_pln = IP_LEN;                                             // IP size
        arp->arp.ar_op = htons(ARPOP_REQUEST);                                // ARP Request
        memcpy(arp->sender_mac, "\xde\xad\xbe\xef\xbe\xef", arp->arp.ar_hln); // Replace with your MAC
        inet_pton(AF_INET, "172.16.11.201", arp->sender_ip);                  // Replace with your IP
        memset(arp->target_mac, 0x00, arp->arp.ar_hln);                       // Unknown
        inet_pton(AF_INET, "172.16.11.203", arp->target_ip);                  // Replace with target IP

        // Send ARP request
        ret = arp_request(buffer, sizeof(buffer));
        if (ret != ARP_E_SUCCESS) {
            printf("ARP request failed, the error code is %d\n", ret);
        }
    } else if (plca_mode == PLCA_MODE_FOLLOWER) {
        // Receive ARP Reply
        ret = arp_reply(buffer, &received_length);
        if (ret != ARP_E_SUCCESS) {
            printf("ARP reply failed, the error code is %d\n", ret);
        }

        if (ntohs(eth->h_proto) == ETH_P_ARP) { // Check if ARP
            arp = (struct arphdr_ipv4*)(buffer + sizeof(struct ethhdr));
            if (ntohs(arp->arp.ar_op) == ARPOP_REPLY) { // ARP Reply
                printf("Received ARP reply from %d.%d.%d.%d\n", arp->sender_ip[0], arp->sender_ip[1], arp->sender_ip[2],
                       arp->sender_ip[3]);
                add_to_arp_table(arp->sender_ip, arp->sender_mac);
                print_arp_table();
                ret = ARP_E_SUCCESS;
            } else if (ntohs(arp->arp.ar_op) == ARPOP_REQUEST) { // ARP Request
                printf("Received ARP request from %d.%d.%d.%d\n", arp->sender_ip[0], arp->sender_ip[1],
                       arp->sender_ip[2], arp->sender_ip[3]);
                ret = ARP_E_SUCCESS;
            } else {
                printf("ARP reply is not ARPOP_REPLY nor ARPOP_REQUEST\n");
            }
        }

    } else {
        printf("Invalid PLCA mode\n");
    }

    return ret;
}
