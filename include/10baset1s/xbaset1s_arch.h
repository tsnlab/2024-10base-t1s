#pragma once

#include <stdint.h>

enum {
    RET_SUCCESS = 0,
    RET_FAIL,
};

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
