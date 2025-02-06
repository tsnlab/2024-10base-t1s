#include "spi.h"

#include "arch.h"
#include "arp_test.h"

#define SPI_COMM_SPEED (15000000) // 15MHz speed
#define SPI_CHANNEL (0)           // Channel 0
#define SPI_FLAGS (0)             // No special setup

static int spihandle;

static int spi_init(void) {

    gpioCfgClock(1, 1, 1); // Settup sample rate to be 1 MHz.
    if (gpioInitialise() < 0) {
        return -SPI_E_ISR_INIT_ERROR;
    }

    spihandle = spiOpen(SPI_CHANNEL, SPI_COMM_SPEED, SPI_FLAGS);
    if (spihandle >= 0) {
        return SPI_E_SUCCESS;
    }

    return -SPI_E_INIT_ERROR;
}

int is_pi_gpio_initialised() {

    if (gpioInitialise() < 0) {
        return -SPI_E_ISR_INIT_ERROR;
    }

    return SPI_E_SUCCESS;
}

int api_pi_gpio_config() {

    /* Settup sample rate to be 1 MHz. */
    gpioCfgClock(1, 1, 1);

    return is_pi_gpio_initialised();
}

int api_is_pi_gpio_initialised() {

    return is_pi_gpio_initialised();
}

int api_pi_spi_open() {
    int spi_handle;

    spi_handle = spiOpen(SPI_CHANNEL, SPI_COMM_SPEED, SPI_FLAGS);
    if (spi_handle >= 0) {
        return spi_handle;
    }

    return -SPI_E_INIT_ERROR;
}

int api_spi_init(void) {
    return spi_init();
}

int spi_transfer(uint8_t* rxbuffer, uint8_t* txbuffer, uint16_t length) {
    int numtransfered = -1;

    numtransfered = spiXfer(spihandle, (char*)txbuffer, (char*)rxbuffer, length);
    if (numtransfered > 0) {
        return SPI_E_SUCCESS;
    }

    return -SPI_E_UNKNOWN_ERROR;
}

int spi_cleanup(void) {
    int ret = -SPI_E_UNKNOWN_ERROR;

    if (spihandle >= 0) {
        spiClose(spihandle);
        ret = SPI_E_SUCCESS;
    }

    gpioTerminate();
    return ret;
}

int api_pi_spi_close(int spi_handle) {

    if (spi_handle >= 0) {
        spiClose(spi_handle);
        return SPI_E_SUCCESS;
    }

    return -SPI_E_INIT_ERROR;
}

void api_pi_gpio_terminate() {

    gpioTerminate();
}

int api_spi_cleanup(void) {
    return spi_cleanup();
}

uint8_t get_parity(uint32_t valueToCalculateParity) {
    valueToCalculateParity ^= valueToCalculateParity >> 1;
    valueToCalculateParity ^= valueToCalculateParity >> 2;
    valueToCalculateParity = ((valueToCalculateParity & 0x11111111) * 0x11111111);
    return ((valueToCalculateParity >> 28) & 1);
}

void print_buffer(uint8_t* buffer, uint16_t length) {
    for (int i = 0; i < length; i++) {
        printf_debug("%02x ", buffer[i]);
        if (i % 16 == 15) {
            printf_debug("\n");
        }
    }
    printf_debug("\n");
}
