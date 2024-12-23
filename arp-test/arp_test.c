#include <arp_test.h>
#include <hardware_dependent.h>
#include <spi.h>
// ARP table
struct arp_entry g_arpTable[MAX_ENTRIES];
uint8_t g_arpTableCount = 0u;

bool GetParity(uint32_t valueToCalculateParity) 
{
	valueToCalculateParity ^= valueToCalculateParity >> 1;
	valueToCalculateParity ^= valueToCalculateParity >> 2;
	valueToCalculateParity = ((valueToCalculateParity & 0x11111111U) * 0x11111111U);
	return ((valueToCalculateParity >> 28) & 1);
}

void ConvertEndianness(uint32_t valueToConvert, uint32_t *convertedValue)
{
  uint8_t position = 0;
  uint8_t variableSize = (uint8_t)(sizeof(valueToConvert));
  uint8_t tempVar = 0;
  uint8_t convertedBytes[(sizeof(valueToConvert))] = {0};

  bcopy((char *)&valueToConvert, convertedBytes, variableSize);      // cast and copy an uint32_t to a uint8_t array
  position = variableSize - (uint8_t)1;
  for (uint8_t byteIndex = 0; byteIndex < (variableSize/2); byteIndex++)  // swap bytes in this uint8_t array
  {       
      tempVar = (uint8_t)convertedBytes[byteIndex];
      convertedBytes[byteIndex] = convertedBytes[position];
      convertedBytes[position--] = tempVar;
  }
  bcopy(convertedBytes, (uint8_t *)convertedValue, variableSize);      // copy the swapped convertedBytes to an uint32_t
}

// Function to add an entry to the ARP table
ARP_ReturnType AddtoArpTable(unsigned char *ip, unsigned char *mac)
{
    ARP_ReturnType ret = ARP_E_ERROR;
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
void PrintArpTable(void) {
    uint8_t i;

    printf("\nARP Table:\n");
    for (i = 0u; i < g_arpTableCount; i++) {
        printf("IP: %d.%d.%d.%d, MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
        g_arpTable[i].ip[0], g_arpTable[i].ip[1], g_arpTable[i].ip[2], g_arpTable[i].ip[3],
        g_arpTable[i].mac[0], g_arpTable[i].mac[1], g_arpTable[i].mac[2],
        g_arpTable[i].mac[3], g_arpTable[i].mac[4], g_arpTable[i].mac[5]);
    }
}

ARP_ReturnType ARPRequest(uint8_t* arp_request_buffer, uint16_t length) {
    uint8_t txBuffer[100] = {0u, };
    uint8_t rxBuffer[100] = {0u, };
    uint8_t bufferIndex = 0u;
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
    SPI_Transfer((uint8_t *)&rxBuffer[0], (uint8_t *)&txBuffer[0], (uint16_t)(bufferIndex + length));
    
    return ARP_E_SUCCESS;
}

ARP_ReturnType ARPReply(uint8_t* arp_reply_buffer, uint16_t* length) {
    uint8_t txBuffer[100] = {0u, };
    uint8_t rxBuffer[100] = {0u, };
    static uDataHeaderFooter_t dataTransferHeader = {0u, };
    uDataHeaderFooter_t datatransferRxFooter;
    uint32_t bigEndianRxFooter = 0u;
    uint16_t expected_size = 68u; // ARP reply size
    uint16_t actual_length = 0u; // actual length of received data

    // receive dummy header setting
    dataTransferHeader.stVarTxHeadBits.DNC = DNC_COMMANDTYPE_DATA;
    dataTransferHeader.stVarTxHeadBits.NORX = 0;
    dataTransferHeader.stVarTxHeadBits.P = (!GetParity(dataTransferHeader.dataFrameHeadFoot));

    // copy header (4 bytes)
    for (int8_t headerByteCount = 3; headerByteCount >= 0; headerByteCount--) {
        txBuffer[headerByteCount] = dataTransferHeader.dataFrameHeaderBuffer[headerByteCount];
    }

    // receive try (ARP reply size)
    SPI_Transfer((uint8_t *)&rxBuffer[0], (uint8_t *)&txBuffer[0], expected_size);

    // Footer check
    memmove((uint8_t *)&datatransferRxFooter.dataFrameHeadFoot, 
            &rxBuffer[expected_size - HEADER_FOOTER_SIZE], 
            HEADER_FOOTER_SIZE);
    ConvertEndianness(datatransferRxFooter.dataFrameHeadFoot, &bigEndianRxFooter);
    datatransferRxFooter.dataFrameHeadFoot = bigEndianRxFooter;

    // receive data validation
    if (datatransferRxFooter.stVarRxFooterBits.DV && 
        !datatransferRxFooter.stVarRxFooterBits.EXST) {
        
        actual_length = datatransferRxFooter.stVarRxFooterBits.EBO + 1;
        memcpy(arp_reply_buffer, &rxBuffer[HEADER_FOOTER_SIZE], actual_length);
        *length = actual_length;
        return ARP_E_SUCCESS;
    }

    return ARP_E_ERROR;
}

ARP_ReturnType ArpTest(void) {
    ARP_ReturnType ret = ARP_E_ERROR;
    uint16_t received_length = 0u;

    unsigned char buffer[PACKET_SIZE_ARP] = {0u, }; // Ethernet (14) + ARP (28)
    struct eth_hdr *eth = (struct eth_hdr *) buffer;
    struct arp_hdr *arp = (struct arp_hdr *) (buffer + sizeof(struct eth_hdr));
    
    // Fill Ethernet header
    memset(eth->dest_mac, 0xFF, 6); // Broadcast
    memcpy(eth->src_mac, "\x20\x2b\x20\x61\x0f\x3f", 6); // Replace with your MAC
    eth->ethertype = htons(0x0806); // ARP Ethertype

    // Fill ARP header
    arp->htype = htons(1u); // Ethernet
    arp->ptype = htons(0x0800u); // IPv4
    arp->hlen = 6u; // MAC size
    arp->plen = 4u; // IP size
    arp->oper = htons(1u); // ARP Request
    memcpy(arp->sender_mac, "\x20\x2b\x20\x61\x0f\x3f", 6u); // Replace with your MAC
    memset(arp->target_mac, 0x00, 6u); // Unknown
  
    // Send ARP request
    if (ARPRequest(buffer, sizeof(buffer)) != ARP_E_SUCCESS) {
        printf("ARP request failed\n");
    }

    // Receive ARP Reply

    if (ARPReply(buffer, &received_length) != ARP_E_SUCCESS) {
        printf("ARP reply failed\n");
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
  
    return ret;
}
