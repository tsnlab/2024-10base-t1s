#ifndef XBASE_COMMON_H
#define XBASE_COMMON_H

#define BUFFER_ALIGNMENT (0x800)
#define MAX_PACKET_LENGTH (0x800)
#define MAX_PACKET_BURST (1)
#define MAX_BUFFER_LENGTH (MAX_PACKET_LENGTH * MAX_PACKET_BURST)
#define NUMBER_OF_BUFFER (64)
#define NUMBER_OF_POOL_BUFFER (NUMBER_OF_BUFFER + 1)
#define NUMBER_OF_RESERVED_BUFFER (4)
#define ENGINE_NUMBER_OF_BUFFER (NUMBER_OF_BUFFER / 2)
#define PACKET_ADDRESS_MASK (~(MAX_PACKET_LENGTH - 1))

#define EMPTY_ELEMENT (NULL)

#define NUMBER_OF_QUEUE NUMBER_OF_BUFFER

typedef char* BUF_POINTER;

typedef struct buffer_stack {
    BUF_POINTER elements[NUMBER_OF_POOL_BUFFER];
    int top;
    pthread_mutex_t mutex;
} buffer_stack_t;

typedef struct reserved_buffer_stack {
    BUF_POINTER elements[NUMBER_OF_RESERVED_BUFFER];
    int top;
    pthread_mutex_t mutex;
} reserved_buffer_stack_t;

typedef char* QueueElement;
typedef struct circular_queue {
    QueueElement elements[NUMBER_OF_QUEUE];
    int front;
    int rear;
    int count;
    pthread_mutex_t mutex;
} CircularQueue_t;

typedef struct stats {
    unsigned long long rxPackets;
    unsigned long long rxBytes;
    unsigned long long rxErrors;
    unsigned long long rxNoBuffer; /* BD is not available */
    unsigned long long rxPps;
    unsigned long long rxBps;
    unsigned long long txPackets;
    unsigned long long txBytes;
    unsigned long long txFiltered;
    unsigned long long txErrors;
    unsigned long long txPps;
    unsigned long long txBps;
} stats_t;

#define MAX_PACKET_LEN 1536

struct rx_metadata {
    uint16_t frame_length;
    uint16_t reserved;
} __attribute__((packed, scalar_storage_order("big-endian")));

struct tx_metadata {
    uint16_t frame_length;
    uint16_t reserved;
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
