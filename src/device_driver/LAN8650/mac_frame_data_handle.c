#include "arch.h"
#include "arp_test.h"
#include "register.h"
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
    uint32_t regval;

    printf(">>> %s - length: %d\n", __func__, length);

    /* Buffer Status Register Bits 15:8 – TXC[7:0] Transmit Credits Available */
    regval = read_register(MMS0, OA_BUFSTS);
    regval = (regval >> 8) & 0xFF;

    if (length > (regval * 64)) {
        /* There are not enough Transmit Creditsi */
        printf("<<< %s - length: %d, Transmit Credits: %d\n", __func__, length, regval);
        return -1;
    }

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

    printf("<<< %s - length: %d\n", __func__, length);

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

    //    debug_spi_transfer_result(txbuffer, "txbuffer");

    return result;
}

void print_footer(union data_footer* footer) {

    printf("Footer\n");
#if 0
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
#endif
    printf("Raw value: 0x%08x\n", footer->data_frame_foot);
}

int api_spi_receive_frame(uint8_t* packet, uint16_t* length) {
    uint8_t rxbuffer[MAX_PAYLOAD_BYTE + FOOTER_SIZE] = {
        0,
    };
    union data_footer footer;
    int result;
    uint32_t regval;
    uint32_t acc_bytes = 0;
    uint8_t stop_flag = 0;
    uint16_t actual_length;

    while (stop_flag == 0) {

        /* Buffer Status Register Bits 7:0 – RBA[7:0] Receive Blocks Available */
        regval = read_register(MMS0, OA_BUFSTS);
        if ((regval & 0xFF) == 0) {
            stop_flag = 1;
            /* There is no Ethernet frame data available for reading. */
            return -1;
        }

        result = spi_receive_frame(rxbuffer);

        if (result != SPI_E_SUCCESS) {
            stop_flag = 1;
            return -1;
        }

        // Footer check
        // memset(&footer, 0, sizeof(footer));
        memcpy((uint8_t*)&footer.data_frame_foot, &rxbuffer[MAX_PAYLOAD_BYTE], FOOTER_SIZE);
        footer.data_frame_foot = ntohl(footer.data_frame_foot);

        print_footer(&footer);

        /* Frame Drop */
        if (footer.rx_footer_bits.fd) {
            stop_flag = 1;
            return -1;
        }

        /* Check if data is valid */
        /* There is no start of an Ethernet frame in the frame that came in at the time when the frame was started to be
         * received.*/
        if ((!footer.rx_footer_bits.dv) || ((acc_bytes == 0) && (!footer.rx_footer_bits.sv))) {
            continue;
        }

        if (footer.rx_footer_bits.sv) {
            acc_bytes = 0;
            if (footer.rx_footer_bits.ev) {
                /* Ethernet Frame Start + Ethernet Frame End*/
                actual_length = (footer.rx_footer_bits.ebo + 1) - footer.rx_footer_bits.swo * 4;
                memcpy(&packet[acc_bytes], &rxbuffer[footer.rx_footer_bits.swo * 4], actual_length);
                acc_bytes += actual_length;
                *length = acc_bytes;
                stop_flag = 1;
                return 0;

            } else {
                /* Ethernet Frame Start + Not Ethernet Frame End*/
                actual_length = MAX_PAYLOAD_BYTE - footer.rx_footer_bits.swo * 4;
                memcpy(&packet[acc_bytes], &rxbuffer[footer.rx_footer_bits.swo * 4], actual_length);
                acc_bytes += actual_length;
            }
        } else {
            if (footer.rx_footer_bits.ev) {
                /* Not Ethernet Frame Start + Ethernet Frame End*/
                actual_length = footer.rx_footer_bits.ebo + 1;
                memcpy(&packet[acc_bytes], &rxbuffer[0], actual_length);
                acc_bytes += actual_length;
                *length = acc_bytes;
                stop_flag = 1;
                return 0;
            } else {
                /* Not Ethernet Frame Start + Not Ethernet Frame End*/
                actual_length = MAX_PAYLOAD_BYTE;
                memcpy(&packet[acc_bytes], &rxbuffer[0], actual_length);
                acc_bytes += actual_length;
            }
        }
    }

    // print buffer for debugging
    // debug_spi_transfer_result(rxbuffer, "rxbuffer");

#if 0
    // receive data validation
    if (footer.rx_footer_bits.dv && !footer.rx_footer_bits.exst) {

        uint16_t actual_length = footer.rx_footer_bits.ebo + 1;
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
#endif

    return -1;
}
