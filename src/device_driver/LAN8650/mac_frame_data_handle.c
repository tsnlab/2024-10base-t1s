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

    uint8_t txbuffer[MPW_MAX_BUFFER_SIZE] = {
        0,
    };
    uint8_t rxbuffer[MPW_MAX_BUFFER_SIZE] = {
        0,
    };
    uint16_t numberof_bytestosend = 0;

    //    printf(">>> %s - length: %d\n", __func__, length);

    /* Buffer Status Register Bits 15:8 – TXC[7:0] Transmit Credits Available */
    regval = read_register(RG_FRAME_BUFFER, FBW_BUFFER_WRITE_STATUS1_H);

    if ((regval & 0xFF) == 0) {
        /* There is no empty buffer */
        return -1;
    }

    fill_mpw_header(txbuffer, SPI_CMD_WRITE, TX_FRAME_FIFO_BASE);

    memcpy(&txbuffer[SPI_MPW_HEADER_SIZE], packet, length);

    numberof_bytestosend = SPI_MPW_HEADER_SIZE + length;

    spi_transfer(rxbuffer, txbuffer, numberof_bytestosend);

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

void fill_mpw_header(uint8_t *txbuffer, uint8_t cmd, uint16_t address);

int spi_mpw_read_metata(struct mpw_meta_data * p_meta) {
    struct mpw_cmd_header input;
    uint8_t txbuffer[sizeof(struct mpw_ctrl_cmd_meta)] = {0};
    uint8_t rxbuffer[sizeof(struct mpw_ctrl_cmd_meta)] = {0};
    uint16_t numberof_bytestosend = 0;
    uint32_t timestamp;
    uint16_t frame_length;

#if 0
    input.cmd = SPI_CMD_READ;
    input.address = METADATA_FIFO_BASE;

    fill_mpw_header(txbuffer, input.cmd, input.address);
#else
    fill_mpw_header(txbuffer, SPI_CMD_READ, METADATA_FIFO_BASE);
#endif

    numberof_bytestosend = SPI_READ_META_DATA_CMD_LENGTH; /* Command(1) + Target Address(2) + Timestamp(8) + Frame Length(2) */

    printf("%s\n", __func__);
    for(int id=0; id< SPI_READ_META_DATA_CMD_LENGTH; id++) {
        printf("txbuffer[%2d]: 0x%02x\n", id, txbuffer[id] & 0xff);
    }

    for(int id=0; id<SPI_READ_META_DATA_CMD_LENGTH; id++) {
        printf("rxbuffer[%2d]: 0x%02x\n", id, rxbuffer[id] & 0xff);
    }

    spi_transfer(rxbuffer, txbuffer, numberof_bytestosend);

    for(int id=0; id<SPI_READ_META_DATA_CMD_LENGTH; id++) {
        printf("rxbuffer[%2d]: 0x%02x\n", id, rxbuffer[id] & 0xff);
    }

    memcpy((uint8_t*)&timestamp, &rxbuffer[SPI_MPW_HEADER_SIZE], sizeof(uint32_t));
    p_meta->timestamp_h = ntohl(timestamp);
    memcpy((uint8_t*)&timestamp, &rxbuffer[SPI_MPW_HEADER_SIZE + sizeof(uint32_t)], sizeof(uint32_t));
    p_meta->timestamp_l = ntohl(timestamp);;
    memcpy((uint8_t*)&frame_length, &rxbuffer[SPI_MPW_HEADER_SIZE + 2 * sizeof(uint32_t)], sizeof(uint16_t));
    p_meta->frame_length = ntohs(frame_length);

    return 0;
}

int api_spi_receive_frame(uint8_t* packet, uint16_t* length) {
    uint8_t txbuffer[MPW_MAX_BUFFER_SIZE] = {
        0,
    };
    uint8_t rxbuffer[MPW_MAX_BUFFER_SIZE] = {
        0,
    };
    uint16_t numberof_bytestosend = 0;
    struct mpw_meta_data meta;

    spi_mpw_read_metata(&meta);

    if((meta.frame_length == 0) || (meta.frame_length > MPW_MAX_PAYLOAD_BYTE)) {
        /* There is no Ethernet frame data available for reading. */
        return -1;
    }

    fill_mpw_header(txbuffer, SPI_CMD_READ, RX_FRAME_FIFO_BASE);

    numberof_bytestosend = SPI_MPW_HEADER_SIZE + meta.frame_length; 

    spi_transfer(rxbuffer, txbuffer, numberof_bytestosend);

    memcpy(packet, (uint8_t *)&meta, MPW_RX_META_LENGTH);

    memcpy((uint8_t *)&packet[MPW_RX_META_LENGTH], (uint8_t *)&rxbuffer[SPI_MPW_HEADER_SIZE], meta.frame_length);

    *length = meta.frame_length;

    return 0;
}
