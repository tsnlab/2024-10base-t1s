#pragma once

#include <stdint.h>

enum {
    RET_SUCCESS = 0,
    RET_FAIL,
};

#define FRAME_TIMESTAMP_ENABLE 1

#ifdef FRAME_TIMESTAMP_ENABLE

enum {
    TIMESTAMP_32BITS = 0,
    TIMESTAMP_64BITS,
};

/* Transmit Timestamp Capture Register */
enum {
    TTSC_NO = 0,
    TTSC_A,
    TTSC_B,
    TTSC_C,
};

#define TRANSMIT_TIMESTAMP_CAPTURE_MASK 0x3

#define TRANSMIT_TIMESTAMP_CAPTURE_AVAILABLE_MASK_A 0x100
#define TRANSMIT_TIMESTAMP_CAPTURE_AVAILABLE_MASK_B 0x200
#define TRANSMIT_TIMESTAMP_CAPTURE_AVAILABLE_MASK_C 0x400

#define TIMESTAMP_CAPTURE_AVAILABLE_CHECK_COUNT 10

#define NANOSECONDS_MASK 0x3FFFFFFF

#define FRAME_TIMESTAMP_SELECT TIMESTAMP_64BITS

#if 1 /* 64-BIT TIMESTAMPS */

struct timestamp_format {
    uint32_t seconds;
    union {
        uint32_t nanoseconds;
        struct {
            uint32_t nanoseconds : 30;
            uint32_t rsvd : 2;
        } nano;
    };
};

#else /* 32-BIT TIMESTAMPS */

#define NANOSECONDS_WIDTH 30
#define SECONDS_MASK 0x3

struct timestamp_format {
    uint32_t nanoseconds : 30;
    uint32_t seconds : 2;
};

#endif

#endif

/* Error Codes */
#define ERR_NOT_ENOUGH_CREDITS (-100) /* not enough Transmit Credits */
#define ERR_SPI_TRANSMIT_FAIL (-101)  /* Fail to spi_transmit_frame */
#define ERR_NO_RECEIVED_FRAME (-102)  /* no Ethernet frame data available for reading */
#define ERR_SPI_RECEIVE_FAIL (-103)   /* Fail to spi_receive_frame */
#define ERR_RECEIVE_FRAME_DROP (-104) /* Receive Frame Drop */

#define ERR_INIT_FAILED (-1)      /* gpioInitialise failed */
#define ERR_BAD_HANDLE (-25)      /* unknown handle */
#define ERR_SPI_OPEN_FAILED (-73) /* can't open SPI device */
#define ERR_BAD_FLAGS (-77)       /* bad i2c/spi/ser open flags */
#define ERR_BAD_SPI_SPEED (-78)   /* bad SPI speed */
#define ERR_BAD_SPI_COUNT (-84)   /* bad SPI count */
#define ERR_SPI_XFER_FAILED (-89) /* spi xfer/read/write failed  */
#define ERR_NO_AUX_SPI (-91)      /* no auxiliary SPI on Pi A or B */

#define ERR_UNKNOWN_PARAMETER (-200) /* Unknown parameter */
#define ERR_NOT_AVAILAVLE (-201)     /* Not available */
