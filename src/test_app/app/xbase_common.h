#ifndef XBASE_COMMON_H
#define XBASE_COMMON_H

#define MAX_BUFFER_LENGTH 0x800
#define MAX_PACKET_LEN 1536

struct rx_metadata {
    uint32_t timestamp_h;
    uint32_t timestamp_l;
    uint16_t frame_length;
    uint16_t reserved;
} __attribute__((packed, scalar_storage_order("big-endian")));

struct tick_count {
        uint32_t tick:29;
        uint32_t priority:3;
} __attribute__((packed, scalar_storage_order("big-endian")));

struct tx_metadata {
        struct tick_count from;
        struct tick_count to;
        struct tick_count delay_from;
        struct tick_count delay_to;
        uint16_t frame_length;
        uint16_t timestamp_id;
        uint8_t fail_policy;
        uint8_t reserved0[3];
        uint32_t reserved1;
        uint32_t reserved2;
} __attribute__((packed, scalar_storage_order("big-endian")));

struct spi_rx_buffer {
    struct rx_metadata metadata;
    uint8_t data[MAX_PACKET_LEN];
};

struct spi_tx_buffer {
    struct tx_metadata metadata;
    uint8_t data[MAX_PACKET_LEN];
};

#define HW_ADDR_LEN 6
#define IP_ADDR_LEN 4

/******************************************************************************
 *                                                                            *
 *                            Function Prototypes                             *
 *                                                                            *
 ******************************************************************************/

#endif // XBASE_COMMON_H
