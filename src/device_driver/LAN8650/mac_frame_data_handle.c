#include "arch.h"
#include "arp_test.h"
#include "spi.h"

static int spi_transmit_frame(uint8_t* packet, uint16_t length, uint32_t sv, uint32_t ev) {
    uint8_t txbuffer[HEADER_SIZE + MAX_PAYLOAD_BYTE] = {
        0,
    };
    uint8_t rxbuffer[MAX_PAYLOAD_BYTE + FOOTER_SIZE] = {
        0,
    };
    static uint8_t seq_toggle = 1;
    union data_header data_transfer_header;

    data_transfer_header.data_frame_head = 0;
    uint32_t be_header;

    // header setting
    data_transfer_header.tx_header_bits.dnc = DNC_COMMANDTYPE_DATA;
    data_transfer_header.tx_header_bits.seq = seq_toggle; /* Data Block Sequence */
    seq_toggle = !seq_toggle;
    data_transfer_header.tx_header_bits.norx = 0;         /* No Receive */
    data_transfer_header.tx_header_bits.dv = 1;           /* Data Valid */
    data_transfer_header.tx_header_bits.sv = sv & 0x1;    /* start chunk */
    data_transfer_header.tx_header_bits.ev = ev & 0x1;    /* end chunk (single chunk) */
    data_transfer_header.tx_header_bits.ebo = length - 1; /* End Byte Offset */
    data_transfer_header.tx_header_bits.p = ((get_parity(data_transfer_header.data_frame_head) == 0) ? 1 : 0);

    // copy header
    be_header = htonl(data_transfer_header.data_frame_head);
    memcpy(txbuffer, &be_header, HEADER_SIZE);

    // copy ARP data
    memcpy(&txbuffer[HEADER_SIZE], packet, length);

    // transfer
    if (SPI_E_SUCCESS != spi_transfer(rxbuffer, txbuffer, sizeof(txbuffer))) {
        return -ARP_E_REQUEST_FAILED;
    }

#if 0
    // print buffer for debugging
    printf_debug("txbuffer when Requesting: \n");
    print_buffer(txbuffer, HEADER_SIZE + PACKET_SIZE_ARP);
    printf_debug("rxbuffer when Requesting: \n");
    print_buffer(rxbuffer, PACKET_SIZE_ARP + FOOTER_SIZE);
#endif

    return ARP_E_SUCCESS;
}

int api_spi_transmit_frame(uint8_t* packet, uint16_t length) {
    uint32_t start_valid = 1;
    uint32_t end_valid;
    uint16_t send_bytes;
    uint16_t acc_byte = 0;
    uint16_t remainder = length;
    int result;

    while (remainder > 0) {
        if (remainder <= MAX_PAYLOAD_BYTE) {
            end_valid = 1;
            send_bytes = remainder;

        } else {
            end_valid = 0;
            send_bytes = MAX_PAYLOAD_BYTE;
        }

        result = spi_transmit_frame((uint8_t*)&packet[acc_byte], send_bytes, start_valid, end_valid);
        if (result) {
            printf("%s - Fail to transmit frame(packet[%d], send_bytes: %d)\n", __func__, acc_byte, send_bytes);
            return -1;
        }
        acc_byte += send_bytes;
        remainder -= send_bytes;
        start_valid = 0;
    }

    return 0;
}

static void config_receive_dummy_header(uint32_t* header) {
    static uint8_t seq_toggle = 1;
    union data_header data_transfer_header;

    data_transfer_header.data_frame_head = 0;

    /* receive dummy header setting */
    data_transfer_header.tx_header_bits.dnc = DNC_COMMANDTYPE_DATA;
    data_transfer_header.tx_header_bits.seq = seq_toggle; /* Other values are all 0 since this is receive mode */
    seq_toggle = !seq_toggle;
    data_transfer_header.tx_header_bits.p = ((get_parity(data_transfer_header.data_frame_head) == 0) ? 1 : 0);

    *header = htonl(data_transfer_header.data_frame_head);
}

void debug_print(uint8_t* buffer, uint16_t length) {
    for (int i = 0; i < length; i++) {
        printf("%02x ", buffer[i]);
        if (i % 16 == 15) {
            printf("\n");
        }
    }
    printf("\n");
}

void debug_spi_transfer_result(uint8_t* buffer, char* name) {

    printf("\n%s when Receiving: \n", name);
    debug_print(buffer, HEADER_SIZE + PACKET_SIZE_ARP);
}

static int spi_receive_frame(uint8_t* rxbuffer) {
    uint8_t txbuffer[HEADER_SIZE + MAX_PAYLOAD_BYTE] = {
        0,
    };
    uint32_t be_header;
    int result;

    config_receive_dummy_header(&be_header);
    memcpy(txbuffer, &be_header, HEADER_SIZE);

    // receive buffer
    result = spi_transfer(rxbuffer, txbuffer, sizeof(txbuffer));

    debug_spi_transfer_result(txbuffer, "txbuffer");

    return result;
}

void print_footer(union data_footer* footer) {

    printf("Footer\n");
    printf("     exst: %u\n", footer->rx_footer_bits.exst);
    printf("     hdrb: %u\n", footer->rx_footer_bits.hdrb);
    printf("     sync: %u\n", footer->rx_footer_bits.sync);
    printf("      rba: %u\n", footer->rx_footer_bits.rba);
    printf("       vs: %u\n", footer->rx_footer_bits.vs);
    printf("       dv: %u\n", footer->rx_footer_bits.dv);
    printf("       sv: %u\n", footer->rx_footer_bits.sv);
    printf("      swo: %u\n", footer->rx_footer_bits.swo);
    printf("       fd: %u\n", footer->rx_footer_bits.fd);
    printf("       ev: %u\n", footer->rx_footer_bits.ev);
    printf("      ebo: %u\n", footer->rx_footer_bits.ebo);
    printf("     rtsa: %u\n", footer->rx_footer_bits.rtsa);
    printf("     rtps: %u\n", footer->rx_footer_bits.rtps);
    printf("      txc: %u\n", footer->rx_footer_bits.txc);
    printf("        p: %u\n", footer->rx_footer_bits.p);
    printf("Raw value: 0x%08x\n", footer->data_frame_foot);
}

int api_spi_receive_frame(uint8_t* packet, uint16_t* length) {
    uint8_t rxbuffer[MAX_PAYLOAD_BYTE + FOOTER_SIZE] = {
        0,
    };
    union data_footer data_transfer_rx_footer;
    int result;

    result = spi_receive_frame(rxbuffer);

    // print buffer for debugging
    debug_spi_transfer_result(rxbuffer, "rxbuffer");

    // Footer check
    memcpy((uint8_t*)&data_transfer_rx_footer.data_frame_foot, &rxbuffer[MAX_PAYLOAD_BYTE], FOOTER_SIZE);
    data_transfer_rx_footer.data_frame_foot = ntohl(data_transfer_rx_footer.data_frame_foot);

    print_footer(&data_transfer_rx_footer);

    // receive data validation
    if (data_transfer_rx_footer.rx_footer_bits.dv && !data_transfer_rx_footer.rx_footer_bits.exst) {

        uint16_t actual_length = data_transfer_rx_footer.rx_footer_bits.ebo + 1;
        memcpy(packet, rxbuffer, actual_length);
        *length = actual_length;

        // Ethernet packet check
        printf("Destination MAC: ");
        printf("%02x:%02x:%02x:%02x:%02x:%02x\n", // clang-format off
                packet[0], packet[1], packet[2],
                packet[3], packet[4], packet[5]);
        printf("Source MAC: ");
        printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                packet[6], packet[7], packet[8],
                packet[9], packet[10], packet[11]);
        printf("EtherType: %02x%02x\n",
                packet[12], packet[13]); // clang-format on

        return 0;
    }

    return -2;
}
