#pragma once

#include <stdint.h>

/**
 * Configures pigpio to use a particular sample rate timed by a specified peripheral.
 * Opens a SPI device
 * @return handle (>=0) if OK, otherwise ERR_BAD_SPI_CHANNEL, ERR_BAD_SPI_SPEED, ERR_BAD_FLAGS, ERR_NO_AUX_SPI, or
 * ERR_SPI_OPEN_FAILED
 */
int spi_open(void);

/**
 * Transfers count bytes of data from txBuf to the SPI device associated with the handle
 * @param handle. >=0, as returned by a call to spi_init
 * @param rxbuffer the received data bytes
 * @param txbuffer the data bytes to write
 * @param count the number of bytes to transfer
 * @return the number of bytes transferred if OK, otherwise ERR_BAD_HANDLE, ERR_BAD_SPI_COUNT, or ERR_SPI_XFER_FAILED.
 */
int spi_transfer(unsigned int handle, uint8_t* rxbuffer, uint8_t* txbuffer, uint16_t count);

/**
 * Closes the SPI device identified by the handle & Terminates the library
 * @param handle. >=0, as returned by a call to spi_init
 * @return 0 if OK, otherwise ERR_BAD_HANDLE.
 */
int spi_close(unsigned int handle);
