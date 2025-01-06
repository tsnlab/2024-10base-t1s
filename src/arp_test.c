#include "arp_test.h"

#include "arch.h"
#include "spi.h"

// ARP table
struct arp_entry g_arp_table[MAX_ENTRIES];
uint8_t g_arp_table_count = 0u;

// Function to add an entry to the ARP table
static int add_to_arp_table(unsigned char* ip, unsigned char* mac) {
    int ret = -ARP_E_REQUEST_FAILED;
    uint8_t i;

    for (i = 0u; i < g_arp_table_count; i++) {
        if (memcmp(g_arp_table[i].ip, ip, 4u) == 0) {
            /* IP already exists, update MAC */
            memcpy(g_arp_table[i].mac, mac, 6u);
        }
    }
    if (g_arp_table_count < MAX_ENTRIES) {
        memcpy(g_arp_table[g_arp_table_count].ip, ip, 4u);
        memcpy(g_arp_table[g_arp_table_count].mac, mac, 6u);
        g_arp_table_count++;
        ret = ARP_E_SUCCESS;
    } else {
        printf("ARP table is full\n");
    }

    return ret;
}

// Function to print the ARP table
static void print_arp_table(void) {
    uint8_t i;

    printf("\nARP Table:\n");
    for (i = 0u; i < g_arp_table_count; i++) {
        printf("IP: %d.%d.%d.%d, MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", g_arp_table[i].ip[0], g_arp_table[i].ip[1],
               g_arp_table[i].ip[2], g_arp_table[i].ip[3], g_arp_table[i].mac[0], g_arp_table[i].mac[1],
               g_arp_table[i].mac[2], g_arp_table[i].mac[3], g_arp_table[i].mac[4], g_arp_table[i].mac[5]);
    }
}

static int arp_request(uint8_t* arp_request_buffer, uint16_t length) {
    uint8_t txbuffer[MAX_PAYLOAD_BYTE + HEADER_FOOTER_SIZE] = {
        0u,
    };
    uint8_t rxbuffer[MAX_PAYLOAD_BYTE + HEADER_FOOTER_SIZE] = {
        0u,
    };
    uint8_t i = 0u;
    uint8_t j = 0u;
    static union u_data_header_footer data_transfer_header = {
        0u,
    };

    // header setting
    data_transfer_header.st_tx_header_bits.DNC = DNC_COMMANDTYPE_DATA;
    data_transfer_header.st_tx_header_bits.SEQ = 0;  // TODO: check this if there are multiiple chunks
    data_transfer_header.st_tx_header_bits.NORX = 0; // No Receive
    data_transfer_header.st_tx_header_bits.DV = 1;   // Data Valid
    data_transfer_header.st_tx_header_bits.SV = 1;   // start chunk
    data_transfer_header.st_tx_header_bits.EV = 1;   // end chunk (single chunk)
    data_transfer_header.st_tx_header_bits.EBO = 63; // TODO: check this if there are multiiple chunks
    data_transfer_header.st_tx_header_bits.P = ((get_parity(data_transfer_header.data_frame_head_foot) == 0) ? 1 : 0);

    // copy header
    data_transfer_header.data_frame_head_foot = htonl(data_transfer_header.data_frame_head_foot);
    memcpy(txbuffer, &data_transfer_header.data_frame_head_foot, HEADER_FOOTER_SIZE);

    // copy ARP data
    memcpy(&txbuffer[HEADER_FOOTER_SIZE], arp_request_buffer, length);

    // transfer
    if (SPI_E_SUCCESS !=
        spi_transfer((uint8_t*)&rxbuffer[0], (uint8_t*)&txbuffer[0], (uint16_t)(HEADER_FOOTER_SIZE + length))) {
        return -ARP_E_REQUEST_FAILED;
    }

    printf("txbuffer when Requesting: \n");
    for (i = 0u; i < 7u; i++) {
        for (j = 0u; j < 10u; j++) {
            if (i * 10 + j >= MAX_PAYLOAD_BYTE + HEADER_FOOTER_SIZE) {
                printf("...");
                break;
            }
            printf("%02x ", txbuffer[i * 10 + j]);
        }
        printf("\n");
    }
    printf("\n");

    printf("rxbuffer when Requesting: \n");
    for (i = 0u; i < 7u; i++) {
        for (j = 0u; j < 10u; j++) {
            if (i * 10 + j >= MAX_PAYLOAD_BYTE + HEADER_FOOTER_SIZE) {
                printf("...");
                break;
            }
            printf("%02x ", rxbuffer[i * 10 + j]);
        }
        printf("\n");
    }
    printf("\n");

    return ARP_E_SUCCESS;
}

static int arp_reply(uint8_t* arp_reply_buffer, uint16_t* length) {
    uint8_t txbuffer[MAX_PAYLOAD_BYTE + HEADER_FOOTER_SIZE] = {
        0u,
    };
    uint8_t rxbuffer[MAX_PAYLOAD_BYTE + HEADER_FOOTER_SIZE] = {
        0u,
    };
    static union u_data_header_footer data_transfer_footer = {
        0u,
    };
    union u_data_header_footer data_transfer_rx_footer;
    uint32_t bigendian_rx_footer = 0u;
    uint16_t expected_size = 32; // test packet (20bytes) + header/footer (4+4bytes) + justincase
    uint8_t i = 0u;
    uint8_t j = 0u;

    // receive dummy header setting
    data_transfer_footer.st_tx_header_bits.DNC = DNC_COMMANDTYPE_DATA;
    data_transfer_footer.st_tx_header_bits.NORX = 0;
    data_transfer_footer.st_tx_header_bits.DV = 0; // receive mode
    data_transfer_footer.st_tx_header_bits.P = ((get_parity(data_transfer_footer.data_frame_head_foot) == 0) ? 1 : 0);

    // copy footer (4 bytes)
    bigendian_rx_footer = htonl(data_transfer_footer.data_frame_head_foot);
    memcpy(txbuffer, &bigendian_rx_footer, HEADER_FOOTER_SIZE);

    // receive buffer
    spi_transfer((uint8_t*)&rxbuffer[0], (uint8_t*)&txbuffer[0], expected_size);

    printf("txbuffer when Receiving: \n");
    for (i = 0u; i < 7u; i++) {
        for (j = 0u; j < 10u; j++) {
            if (i * 10 + j >= MAX_PAYLOAD_BYTE + HEADER_FOOTER_SIZE) {
                printf("...");
                break;
            }
            printf("%02x ", txbuffer[i * 10 + j]);
        }
        printf("\n");
    }
    printf("\n");

    printf("rxbuffer when Receiving: \n");
    for (i = 0u; i < 7u; i++) {
        for (j = 0u; j < 10u; j++) {
            if (i * 10 + j >= MAX_PAYLOAD_BYTE + HEADER_FOOTER_SIZE) {
                printf("...");
                break;
            }
            printf("%02x ", rxbuffer[i * 10 + j]);
        }
        printf("\n");
    }
    printf("\n");

    // Footer check
    memmove((uint8_t*)&data_transfer_rx_footer.data_frame_head_foot, &rxbuffer[expected_size - HEADER_FOOTER_SIZE],
            HEADER_FOOTER_SIZE);
    bigendian_rx_footer = ntohl(data_transfer_rx_footer.data_frame_head_foot);
    data_transfer_rx_footer.data_frame_head_foot = bigendian_rx_footer;

    printf_debug("data_transfer_rx_footer: \n");
    printf_debug("EXST: %u\n", data_transfer_rx_footer.st_rx_footer_bits.EXST);
    printf_debug("HDRB: %u\n", data_transfer_rx_footer.st_rx_footer_bits.HDRB);
    printf_debug("SYNC: %u\n", data_transfer_rx_footer.st_rx_footer_bits.SYNC);
    printf_debug("DV: %u\n", data_transfer_rx_footer.st_rx_footer_bits.DV);
    printf_debug("Raw value: 0x%08x\n", data_transfer_rx_footer.data_frame_head_foot);

    // receive data validation
    if (data_transfer_rx_footer.st_rx_footer_bits.DV && !data_transfer_rx_footer.st_rx_footer_bits.EXST) {

        uint16_t actual_length = data_transfer_rx_footer.st_rx_footer_bits.EBO + 1;
        memcpy(arp_reply_buffer, &rxbuffer[HEADER_FOOTER_SIZE], actual_length);
        *length = actual_length;

        // Ethernet packet check
        printf("Received Ethernet packet:\n");
        printf("Destination MAC: ");
        for (i = 0; i < 6; i++) {
            printf("%02x:", arp_reply_buffer[i]);
        }
        printf("\nSource MAC: ");
        for (i = 6; i < 12; i++) {
            printf("%02x:", arp_reply_buffer[i]);
        }
        printf("\nEtherType: %02x%02x\n", arp_reply_buffer[12], arp_reply_buffer[13]);

        return ARP_E_SUCCESS;
    }

    return -ARP_E_REPLY_FAILED;
}

int arp_test(int plca_mode) {
    int ret = -ARP_E_REQUEST_FAILED;
    uint16_t received_length = 0u;

    unsigned char buffer[PACKET_SIZE_ARP] = {
        0u,
    }; // Ethernet (14) + ARP (28)
    struct ethhdr* eth = (struct ethhdr*)buffer;
    struct arphdr_ipv4* arp = (struct arphdr_ipv4*)(buffer + sizeof(struct ethhdr));

    // Fill Ethernet header
    memset(eth->h_dest, 0xFF, 6);                         // Broadcast
    memcpy(eth->h_source, "\xd8\x3a\xdd\x44\xab\x0f", 6); // Replace with your MAC
    eth->h_proto = htons(0x0806);                         // ARP Ethertype

    // Fill ARP header
    arp->arp.ar_hrd = htons(1u);                                          // Ethernet
    arp->arp.ar_pro = htons(0x0800u);                                     // IPv4
    arp->arp.ar_hln = 6u;                                                 // MAC size
    arp->arp.ar_pln = 4u;                                                 // IP size
    arp->arp.ar_op = htons(1u);                                           // ARP Request
    memcpy(arp->sender_mac, "\xd8\x3a\xdd\x44\xab\x0f", arp->arp.ar_hln); // Replace with your MAC
    inet_pton(AF_INET, "172.16.11.201", arp->sender_ip);                  // Replace with your IP
    memset(arp->target_mac, 0x00, arp->arp.ar_hln);                       // Unknown
    inet_pton(AF_INET, "172.16.11.203", arp->target_ip);                  // Replace with target IP

    if (plca_mode == PLCA_MODE_COORDINATOR) {
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

        if (ntohs(eth->h_proto) == 0x0806) { // Check if ARP
            arp = (struct arphdr_ipv4*)(buffer + sizeof(struct ethhdr));
            if (ntohs(arp->arp.ar_op) == 2) { // ARP Reply
                printf("Received ARP reply from %d.%d.%d.%d\n", arp->sender_ip[0], arp->sender_ip[1], arp->sender_ip[2],
                       arp->sender_ip[3]);
                add_to_arp_table(arp->sender_ip, arp->sender_mac);
                print_arp_table();
                ret = ARP_E_SUCCESS;
            }
        }

    } else {
        printf("Invalid PLCA mode\n");
    }

    return ret;
}
