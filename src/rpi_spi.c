#include <pigpio.h>
#include <stdint.h>
#include <stdio.h>

#include <10baset1s/rpi_spi.h>
#include <10baset1s/xbaset1s_arch.h>

#define PI_SPI_COMM_SPEED 15000000 /* 15MHz speed */
#define PI_SPI_CHANNEL 0           /* Channel 0 */
#define PI_SPI_FLAGS 0             /* No special setup */

int gpio_config(unsigned int micros, unsigned int peripheral, unsigned int source) {

    int ret;

    /* int gpioCfgClock(unsigned cfgMicros, unsigned cfgPeripheral, unsigned cfgSource) */
    /**
     * Configures pigpio to use a particular sample rate timed by a specified peripheral.
     * This function is only effective if called before gpioInitialise.
     *     cfgMicros: 1, 2, 4, 5, 8, 10
     * cfgPeripheral: 0 (PWM), 1 (PCM)
     * cfgSource: deprecated, value is ignored
     */
    ret = gpioCfgClock(micros, peripheral, source);
    printf("%s - %dth line - %d\n", __func__, __LINE__, ret);

    /* int gpioInitialise(void) */
    /**
     * Initialises the library.
     * Returns the pigpio version number if OK, otherwise PI_INIT_FAILED.
     * gpioInitialise must be called before using the other library functions with the following exceptions:
     */
    if ((ret = gpioInitialise()) < 0) {
        printf("%s - %dth line - %d\n", __func__, __LINE__, ret);
        return ERR_INIT_FAILED;
    }

    printf("%s - %dth line - the pigpio version number: %d\n", __func__, __LINE__, ret);
    return RET_SUCCESS;
}

int spi_open(void) {

    int handle;

    if (gpio_config(1, 1, 1) != RET_SUCCESS) {
        return ERR_INIT_FAILED;
    }

    /* Opens a SPI device */
    /** int spiOpen(unsigned spiChan, unsigned baud, unsigned spiFlags)
     * This function returns a handle for the SPI device on the channel. Data will be transferred at baud bits per
     * second. The flags may be used to modify the default behaviour of 4-wire operation, mode 0, active low chip
     * select. The Pi has two SPI peripherals: main and auxiliary. The main SPI has two chip selects (channels), the
     * auxiliary has three. The auxiliary SPI is available on all models but the A and B. The GPIO used are given in the
     * following table.
     *          MISO MOSI SCLK CE0   CE1   CE2
     * Main SPI    9   10   11   8     7    -
     * Aux  SPI   19   20   21  18    17    16
     * spiChan: 0-1 (0-2 for the auxiliary SPI)
     * baud: 32K-125M (values above 30M are unlikely to work)
     * spiFlags: see below
     * Returns a handle (>=0) if OK, otherwise PI_BAD_SPI_CHANNEL,
     * PI_BAD_SPI_SPEED, PI_BAD_FLAGS, PI_NO_AUX_SPI, or PI_SPI_OPEN_FAILED.
     */
    handle = spiOpen(PI_SPI_CHANNEL, PI_SPI_COMM_SPEED, PI_SPI_FLAGS);
    if (handle < 0) {
        printf("%s - %dth line - %d\n", __func__, __LINE__, handle);
    }
    return handle;
}

int spi_transfer(unsigned int handle, uint8_t* rxbuffer, uint8_t* txbuffer, uint16_t count) {
    int num_bytes_tr;

    /* Transfers bytes with a SPI device */
    /**
     * int spiXfer(unsigned handle, char *txBuf, char *rxBuf, unsigned count)
     * This function transfers count bytes of data from txBuf to the SPI device associated with the handle.
     * Simultaneously count bytes of data are read from the device and placed in rxBuf.
     * handle: >=0, as returned by a call to spiOpen
     * txBuf: the data bytes to write
     * rxBuf: the received data bytes
     * count: the number of bytes to transfer
     * Returns the number of bytes transferred if OK, otherwise PI_BAD_HANDLE, PI_BAD_SPI_COUNT, or PI_SPI_XFER_FAILED.
     */
    num_bytes_tr = spiXfer(handle, (char*)txbuffer, (char*)rxbuffer, count);
    if (num_bytes_tr <= 0) {
        printf("%s - %dth line - %d\n", __func__, __LINE__, num_bytes_tr);
    }
    return num_bytes_tr;
}

int spi_close(unsigned int handle) {
    int ret;

    /* Closes a SPI device */
    /* int spiClose(unsigned handle)
     * This functions closes the SPI device identified by the handle.
     * handle: >=0, as returned by a call to spiOpen
     * Returns 0 if OK, otherwise PI_BAD_HANDLE.
     */

    ret = spiClose(handle);
    if (ret != 0) {
        printf("%s - %dth line - %d\n", __func__, __LINE__, ret);
    }

    /* Stop library */
    /**
     * void gpioTerminate(void)
     * Terminates the library.
     * Returns nothing.
     * Call before program exit.
     * This function resets the used DMA channels, releases memory, and terminates any running threads.
     * Example
     * gpioTerminate();
     */

    gpioTerminate();

    return ret;
}
