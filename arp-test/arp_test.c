#include <arp_test.h>
#include <hardware_dependent.h>
#include <spi.h>

// ARP table
struct arp_entry g_arpTable[MAX_ENTRIES];
uint8_t g_arpTableCount = 0u;

// Function to add an entry to the ARP table
static ARP_ReturnType AddtoArpTable(unsigned char *ip, unsigned char *mac)
{
    ARP_ReturnType ret = ARP_E_REQUEST_FAILED;
    uint8_t i;

    for(i = 0u; i < g_arpTableCount; i++) {
        if(memcmp(g_arpTable[i].ip, ip, 4u) == 0) {
            /* IP already exists, update MAC */
            memcpy(g_arpTable[i].mac, mac, 6u);
        }
    }
    if (g_arpTableCount < MAX_ENTRIES) {
        memcpy(g_arpTable[g_arpTableCount].ip, ip, 4u);
        memcpy(g_arpTable[g_arpTableCount].mac, mac, 6u);
        g_arpTableCount++;
        ret = ARP_E_SUCCESS;
    }
    else {
        printf("ARP table is full\n");
    }

    return ret;
}

// Function to print the ARP table
static void PrintArpTable(void) {
    uint8_t i;

    printf("\nARP Table:\n");
    for (i = 0u; i < g_arpTableCount; i++) {
        printf("IP: %d.%d.%d.%d, MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
        g_arpTable[i].ip[0], g_arpTable[i].ip[1], g_arpTable[i].ip[2], g_arpTable[i].ip[3],
        g_arpTable[i].mac[0], g_arpTable[i].mac[1], g_arpTable[i].mac[2],
        g_arpTable[i].mac[3], g_arpTable[i].mac[4], g_arpTable[i].mac[5]);
    }
}

static ARP_ReturnType ARPRequest(uint8_t* arp_request_buffer, uint16_t length) {
    uint8_t txBuffer[100] = {0u, };
    uint8_t rxBuffer[100] = {0u, };
    uint8_t bufferIndex = 0u;
    uint8_t i = 0u;
    uint8_t j = 0u;
    static uDataHeaderFooter_t dataTransferHeader = {0u, };
    
    // header setting
    dataTransferHeader.stVarTxHeadBits.DNC = DNC_COMMANDTYPE_DATA;
    dataTransferHeader.stVarTxHeadBits.SEQ = (uint8_t)(~dataTransferHeader.stVarTxHeadBits.SEQ);
    dataTransferHeader.stVarTxHeadBits.NORX = 0;
    dataTransferHeader.stVarTxHeadBits.DV = 1;
    dataTransferHeader.stVarTxHeadBits.SV = 1;  // start chunk
    dataTransferHeader.stVarTxHeadBits.EV = 1;  // end chunk (single chunk)
    dataTransferHeader.stVarTxHeadBits.EBO = length - 1;
    dataTransferHeader.stVarTxHeadBits.P = (!GetParity(dataTransferHeader.dataFrameHeadFoot));

    // copy header
    for (int8_t headerByteCount = 3; headerByteCount >= 0; headerByteCount--) {
        txBuffer[bufferIndex++] = dataTransferHeader.dataFrameHeaderBuffer[headerByteCount];
    }

    // copy ARP data
    memcpy(&txBuffer[bufferIndex], arp_request_buffer, length);
    
    // transfer
    if(SPI_E_SUCCESS != SPI_Transfer((uint8_t *)&rxBuffer[0], (uint8_t *)&txBuffer[0], (uint16_t)(bufferIndex + length))) {
        return ARP_E_REQUEST_FAILED;
    }
    
    printf("txBuffer when Requesting: \n");
    for (i = 0u; i < 7u; i++) {
        for (j = 0u; j < 10u; j++) {
            printf("%02x ", txBuffer[i * 10 + j]);
        }
        printf("\n");
    }
    printf("\n");

    printf("rxBuffer when Requesting: \n");
    for (i = 0u; i < 7u; i++) {
        for (j = 0u; j < 10u; j++) {
            printf("%02x ", rxBuffer[i * 10 + j]);
        }
        printf("\n");
    }
    printf("\n");

    return ARP_E_SUCCESS;
}
*/

static ARP_ReturnType ARPReply(uint8_t* arp_reply_buffer, uint16_t* length) {
    uint8_t txBuffer[100] = {0u, };
    uint8_t rxBuffer[100] = {0u, };
    static uDataHeaderFooter_t dataTransferHeader = {0u, };
    uDataHeaderFooter_t datatransferRxFooter;
    uint32_t bigEndianRxFooter = 0u;
    uint16_t expected_size = 32; // test packet (20bytes) + header/footer (4+4bytes) + justincase
    uint8_t i = 0u;
    uint8_t j = 0u;

    // receive dummy header setting
    dataTransferFooter.stVarTxHeadBits.DNC = DNC_COMMANDTYPE_DATA;
    dataTransferFooter.stVarTxHeadBits.NORX = 0;
    dataTransferFooter.stVarTxHeadBits.DV = 0;  // receive mode
    dataTransferFooter.stVarTxHeadBits.P = (!GetParity(dataTransferFooter.dataFrameHeadFoot));

    // copy footer (4 bytes)
    for (int8_t footerByteCount = 3; footerByteCount >= 0; footerByteCount--) {
        txBuffer[footerByteCount] = dataTransferFooter.dataFrameHeaderBuffer[footerByteCount];
    }

    // receive buffer
    SPI_Transfer((uint8_t *)&rxBuffer[0], (uint8_t *)&txBuffer[0], expected_size);

    printf("txBuffer when Receiving: \n");
    for (i = 0u; i < 7u; i++) {
        for (j = 0u; j < 10u; j++) {
            printf("%02x ", txBuffer[i * 10 + j]);
        }
        printf("\n");
    }
    printf("\n");

    printf("rxBuffer when Receiving: \n");
    for (i = 0u; i < 7u; i++) {
        for (j = 0u; j < 10u; j++) {
            printf("%02x ", rxBuffer[i * 10 + j]);
        }
        printf("\n");
    }
    printf("\n");

    // Footer check
    memmove((uint8_t *)&datatransferRxFooter.dataFrameHeadFoot, 
            &rxBuffer[expected_size - HEADER_FOOTER_SIZE], 
            HEADER_FOOTER_SIZE);
    ConvertEndianness(datatransferRxFooter.dataFrameHeadFoot, &bigEndianRxFooter);
    datatransferRxFooter.dataFrameHeadFoot = bigEndianRxFooter;

    // receive data validation
    if (datatransferRxFooter.stVarRxFooterBits.DV && 
        !datatransferRxFooter.stVarRxFooterBits.EXST) {
        
        uint16_t actual_length = datatransferRxFooter.stVarRxFooterBits.EBO + 1;
        memcpy(arp_reply_buffer, &rxBuffer[HEADER_FOOTER_SIZE], actual_length);
        *length = actual_length;

        // Ethernet packet check
        printf("Received Ethernet packet:\n");
        printf("Destination MAC: ");
        for(i = 0; i < 6; i++) {
            printf("%02x:", arp_reply_buffer[i]);
        }
        printf("\nSource MAC: ");
        for(i = 6; i < 12; i++) {
            printf("%02x:", arp_reply_buffer[i]);
        }
        printf("\nEtherType: %02x%02x\n", arp_reply_buffer[12], arp_reply_buffer[13]);
        
        return ARP_E_SUCCESS;
    }

    return ARP_E_REPLY_FAILED;
}

/* Original code
ARP_ReturnType ARPReply(uint8_t* arp_reply_buffer, uint16_t* length) {
    uint8_t txBuffer[MAX_PAYLOAD_BYTE + HEADER_FOOTER_SIZE] = {0u, };
    uint8_t rxBuffer[MAX_PAYLOAD_BYTE + HEADER_FOOTER_SIZE] = {0u, };
    static uDataHeaderFooter_t dataTransferFooter = {0u, };
    uDataHeaderFooter_t datatransferRxFooter;
    uint32_t bigEndianRxFooter = 0u;
    uint16_t expected_size = MAX_PAYLOAD_BYTE + HEADER_FOOTER_SIZE; // ARP reply size
    uint16_t actual_length = 0u; // actual length of received data
    uint8_t i = 0u;
    uint8_t j = 0u;
    // receive dummy header setting
    dataTransferFooter.stVarTxHeadBits.DNC = DNC_COMMANDTYPE_DATA;
    dataTransferFooter.stVarTxHeadBits.NORX = 0;
    dataTransferFooter.stVarTxHeadBits.P = (!GetParity(dataTransferFooter.dataFrameHeadFoot));

    // copy header (4 bytes)
    for (int8_t footerByteCount = 3; footerByteCount >= 0; footerByteCount--) {
        txBuffer[footerByteCount] = dataTransferFooter.dataFrameHeaderBuffer[footerByteCount];
    }

    // receive try (ARP reply size)
    SPI_Transfer((uint8_t *)&rxBuffer[0], (uint8_t *)&txBuffer[0], expected_size);

    printf("txBuffer when Receiving: \n");
    for (i = 0u; i < 7u; i++) {
        for (j = 0u; j < 10u; j++) {
            printf("%02x ", txBuffer[i * 10 + j]);
        }
        printf("\n");
    }
    printf("\n");

    printf("rxBuffer when Receiving: \n");
    for (i = 0u; i < 7u; i++) {
        for (j = 0u; j < 10u; j++) {
            printf("%02x ", rxBuffer[i * 10 + j]);
        }
        printf("\n");
    }
    printf("\n");

    // Footer check
    memmove((uint8_t *)&datatransferRxFooter.dataFrameHeadFoot, 
            &rxBuffer[expected_size - HEADER_FOOTER_SIZE], 
            HEADER_FOOTER_SIZE);
    ConvertEndianness(datatransferRxFooter.dataFrameHeadFoot, &bigEndianRxFooter);
    datatransferRxFooter.dataFrameHeadFoot = bigEndianRxFooter;

    // receive data validation
    if (datatransferRxFooter.stVarRxFooterBits.DV && 
        !datatransferRxFooterBits.EXST) {
        
        actual_length = datatransferRxFooter.stVarRxFooterBits.EBO + 1;
        memcpy(arp_reply_buffer, &rxBuffer[HEADER_FOOTER_SIZE], actual_length);
        *length = actual_length;
        return ARP_E_SUCCESS;
    }

    return ARP_E_REPLY_FAILED;
}
*/

ARP_ReturnType ArpTest(void) {
    ARP_ReturnType ret = ARP_E_REQUEST_FAILED;
    uint16_t received_length = 0u;

    unsigned char buffer[PACKET_SIZE_ARP] = {0u, }; // Ethernet (14) + ARP (28)
    struct eth_hdr *eth = (struct eth_hdr *) buffer;
    struct arp_hdr *arp = (struct arp_hdr *) (buffer + sizeof(struct eth_hdr));
    
    // Fill Ethernet header
    memset(eth->dest_mac, 0xFF, 6); // Broadcast
    memcpy(eth->src_mac, "\xd8\x3a\xdd\x44\xab\x0f", 6); // Replace with your MAC
    eth->ethertype = htons(0x0806); // ARP Ethertype

    // Fill ARP header
    arp->htype = htons(1u); // Ethernet
    arp->ptype = htons(0x0800u); // IPv4
    arp->hlen = 6u; // MAC size
    arp->plen = 4u; // IP size
    arp->oper = htons(1u); // ARP Request
    memcpy(arp->sender_mac, "\xd8\x3a\xdd\x44\xab\x0f", 6u); // Replace with your MAC
    inet_pton(AF_INET, "172.16.11.201", arp->sender_ip); // Replace with your IP
    memset(arp->target_mac, 0x00, 6u); // Unknown
    inet_pton(AF_INET, "172.16.11.203", arp->target_ip); // Replace with target IP

    // Send ARP request
    ret = ARPRequest(buffer, sizeof(buffer));
    if (ret != ARP_E_SUCCESS) {
        printf("ARP request failed, the error code is %d\n", ret);
    }

/*    // Receive ARP Reply
    ret = ARPReply(buffer, &received_length);
    if (ret != ARP_E_SUCCESS) {
        printf("ARP reply failed, the error code is %d\n", ret);
    }
        eth = (struct eth_hdr *)buffer;
        if (ntohs(eth->ethertype) == 0x0806) { // Check if ARP
            arp = (struct arp_hdr *) (buffer + sizeof(struct eth_hdr));
            if (ntohs(arp->oper) == 2) { // ARP Reply
                printf("Received ARP reply from %d.%d.%d.%d\n",
                        arp->sender_ip[0], arp->sender_ip[1], arp->sender_ip[2], arp->sender_ip[3]);
                AddtoArpTable(arp->sender_ip, arp->sender_mac);
                PrintArpTable();

                ret = ARP_E_SUCCESS;
            }
    }
    //Debug code
    ret = ARPReply(buffer, &received_length);
    if (ret != ARP_E_SUCCESS) {
        printf("Second ARP reply failed, the error code is %d\n", ret);
    }
*/
    return ret;
}
