#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <10baset1s/lan865x.h>
#include <10baset1s/rpi_spi.h>
#include <10baset1s/xbaset1s_arch.h>
#include <arpa/inet.h>

#include "lan865x-micron.h"

uint8_t get_parity(uint32_t header);

static int spi_transmit_segment(unsigned int handle, uint8_t* packet, uint16_t length, uint32_t sv, uint32_t ev) {
    uint8_t tx_buf[HEADER_SIZE + MAX_PAYLOAD_BYTE] = {
        0,
    };
    uint8_t rx_buf[MAX_PAYLOAD_BYTE + FOOTER_SIZE] = {
        0,
    };
    static uint8_t seq_toggle = 1;
    union data_header tx_header;

    tx_header.data_frame = 0;
    uint32_t be_header;

    /* header setting */
    tx_header.data_bits.dnc = DNC_FLAG_DATA;
    tx_header.data_bits.seq = seq_toggle; /* Data Block Sequence */
    seq_toggle = !seq_toggle;
    tx_header.data_bits.norx = 0;         /* No Receive */
    tx_header.data_bits.dv = 1;           /* Data Valid */
    tx_header.data_bits.sv = sv & 0x1;    /* start chunk */
    tx_header.data_bits.ev = ev & 0x1;    /* end chunk (single chunk) */
    tx_header.data_bits.ebo = length - 1; /* End Byte Offset */
    tx_header.data_bits.p = ((get_parity(tx_header.data_frame) == 0) ? 1 : 0);

    be_header = htonl(tx_header.data_frame);
    memcpy(tx_buf, &be_header, HEADER_SIZE);

    memcpy(&tx_buf[HEADER_SIZE], packet, length);

    if (spi_transfer(handle, rx_buf, tx_buf, sizeof(tx_buf)) <= 0) {
        return -RET_FAIL;
    }

    return RET_SUCCESS;
}

int spi_transmit_frame(unsigned int handle, uint8_t* packet, uint16_t length) {
    uint32_t start_valid = 1;
    uint32_t end_valid;
    uint16_t send_bytes;
    uint16_t acc_byte = 0;
    uint16_t remainder = length;
    int result;
    uint32_t regval;

    /* Buffer Status Register Bits 15:8 – TXC[7:0] Transmit Credits Available */
    regval = read_register(handle, MMS0, MMS0_OA_BUFSTS);
    regval = (regval >> MMS0_OA_BUFSTS_TXC_SHIFT) & MMS0_OA_BUFSTS_TXC_MASK;

    if (length > (regval * MAX_PAYLOAD_BYTE)) {
        /* There are not enough Transmit Credits */
        /* printf("<<< %s - length: %d, Transmit Credits: %d\n", __func__, length, regval); */
        return ERR_NOT_ENOUGH_CREDITS;
    }

    while (remainder > 0) {
        if (remainder <= MAX_PAYLOAD_BYTE) {
            end_valid = 1;
            send_bytes = remainder;

        } else {
            end_valid = 0;
            send_bytes = MAX_PAYLOAD_BYTE;
        }

        result = spi_transmit_segment(handle, (uint8_t*)&packet[acc_byte], send_bytes, start_valid, end_valid);
        if (result) {
            /* printf("%s - Fail to transmit frame(packet[%d], send_bytes: %d)\n", __func__, acc_byte, send_bytes); */
            return ERR_SPI_TRANSMIT_FAIL;
        }
        acc_byte += send_bytes;
        remainder -= send_bytes;
        start_valid = 0;
    }

    return RET_SUCCESS;
}

#ifdef FRAME_TIMESTAMP_ENABLE
static int spi_transmit_segment_with_timestamp(unsigned int handle, uint8_t* packet, uint16_t length, uint32_t sv,
                                               uint32_t ev, uint32_t tsc) {
    uint8_t tx_buf[HEADER_SIZE + MAX_PAYLOAD_BYTE] = {
        0,
    };
    uint8_t rx_buf[MAX_PAYLOAD_BYTE + FOOTER_SIZE] = {
        0,
    };
    static uint8_t seq_toggle = 1;
    union data_header tx_header;

    tx_header.data_frame = 0;
    uint32_t be_header;

    /* header setting */
    tx_header.data_bits.dnc = DNC_FLAG_DATA;
    tx_header.data_bits.seq = seq_toggle; /* Data Block Sequence */
    seq_toggle = !seq_toggle;
    tx_header.data_bits.norx = 0;         /* No Receive */
    tx_header.data_bits.dv = 1;           /* Data Valid */
    tx_header.data_bits.sv = sv & 0x1;    /* start chunk */
    tx_header.data_bits.ev = ev & 0x1;    /* end chunk (single chunk) */
    tx_header.data_bits.tsc = tsc;        /* Time Stamp Capture */
    tx_header.data_bits.ebo = length - 1; /* End Byte Offset */
    tx_header.data_bits.p = ((get_parity(tx_header.data_frame) == 0) ? 1 : 0);

    be_header = htonl(tx_header.data_frame);
    memcpy(tx_buf, &be_header, HEADER_SIZE);

    memcpy(&tx_buf[HEADER_SIZE], packet, length);

    if (spi_transfer(handle, rx_buf, tx_buf, sizeof(tx_buf)) <= 0) {
        return -RET_FAIL;
    }

    return RET_SUCCESS;
}

int spi_transmit_frame_with_timestamp(unsigned int handle, uint8_t* packet, uint16_t length, int reg) {
    uint32_t start_valid = 1;
    uint32_t end_valid;
    uint16_t send_bytes;
    uint16_t acc_byte = 0;
    uint16_t remainder = length;
    int result;
    uint32_t regval;

    /* Buffer Status Register Bits 15:8 – TXC[7:0] Transmit Credits Available */
    regval = read_register(handle, MMS0, MMS0_OA_BUFSTS);
    regval = (regval >> MMS0_OA_BUFSTS_TXC_SHIFT) & MMS0_OA_BUFSTS_TXC_MASK;

    if (length > (regval * MAX_PAYLOAD_BYTE)) {
        /* There are not enough Transmit Credits */
        /* printf("<<< %s - length: %d, Transmit Credits: %d\n", __func__, length, regval); */
        return ERR_NOT_ENOUGH_CREDITS;
    }

    while (remainder > 0) {
        if (remainder <= MAX_PAYLOAD_BYTE) {
            end_valid = 1;
            send_bytes = remainder;

        } else {
            end_valid = 0;
            send_bytes = MAX_PAYLOAD_BYTE;
        }

        result = spi_transmit_segment_with_timestamp(handle, (uint8_t*)&packet[acc_byte], send_bytes, start_valid,
                                                     end_valid, (uint32_t)(reg & TRANSMIT_TIMESTAMP_CAPTURE_MASK));
        if (result) {
            /* printf("%s - Fail to transmit frame(packet[%d], send_bytes: %d)\n", __func__, acc_byte, send_bytes); */
            return ERR_SPI_TRANSMIT_FAIL;
        }
        acc_byte += send_bytes;
        remainder -= send_bytes;
        start_valid = 0;
    }

    return RET_SUCCESS;
}
#endif

static void config_receive_dummy_header(uint32_t* header) {
    static uint8_t seq_toggle = 1;
    union data_header tx_header;

    tx_header.data_frame = 0;

    /* receive dummy header setting */
    tx_header.data_bits.dnc = DNC_FLAG_DATA;
    tx_header.data_bits.seq = seq_toggle; /* Other values are all 0 since this is receive mode */
    seq_toggle = !seq_toggle;
    tx_header.data_bits.p = ((get_parity(tx_header.data_frame) == 0) ? 1 : 0);

    *header = htonl(tx_header.data_frame);
}

static int spi_receive_segment(unsigned int handle, uint8_t* rx_buf) {
    uint8_t tx_buf[HEADER_SIZE + MAX_PAYLOAD_BYTE] = {
        0,
    };
    uint32_t be_header;

    config_receive_dummy_header(&be_header);
    memcpy(tx_buf, &be_header, HEADER_SIZE);

    if (spi_transfer(handle, rx_buf, tx_buf, sizeof(tx_buf)) <= 0) {
        return -RET_FAIL;
    }

    return RET_SUCCESS;
}

int spi_receive_frame(unsigned int handle, uint8_t* packet, uint16_t* length) {
    uint8_t rx_buf[MAX_PAYLOAD_BYTE + FOOTER_SIZE] = {
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
        regval = read_register(handle, MMS0, MMS0_OA_BUFSTS);
        if ((regval & MMS0_OA_BUFSTS_RBA_MASK) == 0) {
            stop_flag = 1;
            /* There is no Ethernet frame data available for reading. */
            return ERR_NO_RECEIVED_FRAME;
        }

        result = spi_receive_segment(handle, rx_buf);

        if (result != RET_SUCCESS) {
            stop_flag = 1;
            return ERR_SPI_RECEIVE_FAIL;
        }

        /* Footer check */
        memcpy((uint8_t*)&footer.data_frame, &rx_buf[MAX_PAYLOAD_BYTE], FOOTER_SIZE);
        footer.data_frame = ntohl(footer.data_frame);

        /* Frame Drop */
        if (footer.footer_bits.fd) {
            stop_flag = 1;
            return ERR_RECEIVE_FRAME_DROP;
        }

        /* Check if data is valid */
        /* There is no start of an Ethernet frame in the frame that came in at the time when the frame was started to be
         * received.*/
        if ((!footer.footer_bits.dv) || ((acc_bytes == 0) && (!footer.footer_bits.sv))) {
            continue;
        }

        if (footer.footer_bits.sv) {
            acc_bytes = 0;
            if (footer.footer_bits.ev) {
                /* Ethernet Frame Start + Ethernet Frame End*/
                actual_length =
                    (footer.footer_bits.ebo + END_BYTE_OFFSET) - footer.footer_bits.swo * START_WORD_OFFSET_UNIT;
                memcpy(&packet[acc_bytes], &rx_buf[footer.footer_bits.swo * START_WORD_OFFSET_UNIT], actual_length);
                acc_bytes += actual_length;
                *length = acc_bytes;
                stop_flag = 1;
                return RET_SUCCESS;
            }
            /* Ethernet Frame Start + Not Ethernet Frame End*/
            actual_length = MAX_PAYLOAD_BYTE - footer.footer_bits.swo * START_WORD_OFFSET_UNIT;
            memcpy(&packet[acc_bytes], &rx_buf[footer.footer_bits.swo * START_WORD_OFFSET_UNIT], actual_length);
            acc_bytes += actual_length;
        } else {
            if (footer.footer_bits.ev) {
                /* Not Ethernet Frame Start + Ethernet Frame End*/
                actual_length = footer.footer_bits.ebo + END_BYTE_OFFSET;
                memcpy(&packet[acc_bytes], &rx_buf[0], actual_length);
                acc_bytes += actual_length;
                *length = acc_bytes;
                stop_flag = 1;
                return RET_SUCCESS;
            }
            /* Not Ethernet Frame Start + Not Ethernet Frame End*/
            actual_length = MAX_PAYLOAD_BYTE;
            memcpy(&packet[acc_bytes], &rx_buf[0], actual_length);
            acc_bytes += actual_length;
        }
    }

    return -RET_FAIL;
}

#ifdef FRAME_TIMESTAMP_ENABLE

int spi_receive_frame_with_timestamp(unsigned int handle, uint8_t* packet, uint16_t* length,
                                     struct timestamp_format* timestamp) {
    uint8_t rx_buf[MAX_PAYLOAD_BYTE + FOOTER_SIZE] = {
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
        regval = read_register(handle, MMS0, MMS0_OA_BUFSTS);
        if ((regval & MMS0_OA_BUFSTS_RBA_MASK) == 0) {
            stop_flag = 1;
            /* There is no Ethernet frame data available for reading. */
            return ERR_NO_RECEIVED_FRAME;
        }

        result = spi_receive_segment(handle, rx_buf);

        if (result != RET_SUCCESS) {
            stop_flag = 1;
            return ERR_SPI_RECEIVE_FAIL;
        }

        /* Footer check */
        memcpy((uint8_t*)&footer.data_frame, &rx_buf[MAX_PAYLOAD_BYTE], FOOTER_SIZE);
        footer.data_frame = ntohl(footer.data_frame);

        printf("footer.data_frame: 0x%08x\n", footer.data_frame);

        for(uint32_t id=0; id<MAX_PAYLOAD_BYTE; id++) {
            if((id %16) == 0) {
                printf("\n");
            }
            printf("0x%02x ", rx_buf[id]);
        }
        printf("\n");

        /* Frame Drop */
        if (footer.footer_bits.fd) {
            stop_flag = 1;
            return ERR_RECEIVE_FRAME_DROP;
        }

        /* Check if data is valid */
        /* There is no start of an Ethernet frame in the frame that came in at the time when the frame was started to be
         * received.*/
        if ((!footer.footer_bits.dv) || ((acc_bytes == 0) && (!footer.footer_bits.sv))) {
            continue;
        }

        if (footer.footer_bits.sv) {
            /* Receive Timestamp Added */
            if (footer.footer_bits.rtsa) {
                memcpy(timestamp, &rx_buf[0], sizeof(struct timestamp_format));
                timestamp->seconds = ntohl(timestamp->seconds);
                timestamp->nanoseconds = ntohl(timestamp->nanoseconds);
            }
            acc_bytes = 0;
            if (footer.footer_bits.rtsa) {
                if (footer.footer_bits.ev) {
                    /* Ethernet Frame Start + Ethernet Frame End*/
                    actual_length = (footer.footer_bits.ebo + END_BYTE_OFFSET) -
                                    footer.footer_bits.swo * START_WORD_OFFSET_UNIT - sizeof(struct timestamp_format);
                    memcpy(&packet[acc_bytes],
                           &rx_buf[footer.footer_bits.swo * START_WORD_OFFSET_UNIT + sizeof(struct timestamp_format)],
                           actual_length);
                    acc_bytes += actual_length;
                    *length = acc_bytes;
                    stop_flag = 1;
                    return RET_SUCCESS;
                }
                /* Ethernet Frame Start + Not Ethernet Frame End*/
                actual_length =
                    MAX_PAYLOAD_BYTE - footer.footer_bits.swo * START_WORD_OFFSET_UNIT - sizeof(struct timestamp_format);
                printf("actual_length: %d\n", actual_length);
                memcpy(&packet[acc_bytes],
                       &rx_buf[footer.footer_bits.swo * START_WORD_OFFSET_UNIT + sizeof(struct timestamp_format)],
                       actual_length);

                for(uint32_t id=0; id<actual_length; id++) {
                    if((id %16) == 0) {
                        printf("\n");
                    }
                    printf("0x%02x ", packet[acc_bytes+id]);
                }
                printf("\n");

                acc_bytes += actual_length;
            } else {
                if (footer.footer_bits.ev) {
                    /* Ethernet Frame Start + Ethernet Frame End*/
                    actual_length =
                        (footer.footer_bits.ebo + END_BYTE_OFFSET) - footer.footer_bits.swo * START_WORD_OFFSET_UNIT;
                    memcpy(&packet[acc_bytes], &rx_buf[footer.footer_bits.swo * START_WORD_OFFSET_UNIT], actual_length);
                    acc_bytes += actual_length;
                    *length = acc_bytes;
                    stop_flag = 1;
                    return RET_SUCCESS;
                }
                /* Ethernet Frame Start + Not Ethernet Frame End*/
                actual_length = MAX_PAYLOAD_BYTE - footer.footer_bits.swo * START_WORD_OFFSET_UNIT;
                memcpy(&packet[acc_bytes], &rx_buf[footer.footer_bits.swo * START_WORD_OFFSET_UNIT], actual_length);
                acc_bytes += actual_length;
            }
        } else {
            if (footer.footer_bits.ev) {
                /* Not Ethernet Frame Start + Ethernet Frame End*/
                actual_length = footer.footer_bits.ebo + END_BYTE_OFFSET;
                printf("actual_length: %d\n", actual_length);
                memcpy(&packet[acc_bytes], &rx_buf[0], actual_length);

                for(uint32_t id=0; id<actual_length; id++) {
                    if((id %16) == 0) {
                        printf("\n");
                    }
                    printf("0x%02x ", packet[acc_bytes+id]);
                }
                printf("\n");

                acc_bytes += actual_length;
                *length = acc_bytes;
                stop_flag = 1;
                return RET_SUCCESS;
            }
            /* Not Ethernet Frame Start + Not Ethernet Frame End*/
            actual_length = MAX_PAYLOAD_BYTE;
            memcpy(&packet[acc_bytes], &rx_buf[0], actual_length);
            acc_bytes += actual_length;
        }
    }

    return -RET_FAIL;
}

#endif
