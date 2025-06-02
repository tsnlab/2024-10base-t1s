#include "spi.h"

#include "arch.h"
#include "arp_test.h"

#define SPI_COMM_SPEED (15000000) // 15MHz speed
// #define SPI_COMM_SPEED (32000) // 32KHz speed
#define SPI_CHANNEL (0)           // Channel 0
#define SPI_FLAGS (0)             // No special setup

static int spihandle;

static int spi_init(void) {

    gpioCfgClock(1, 1, 1); // Settup sample rate to be 1 MHz.
/*
    int gpioCfgClock(unsigned cfgMicros, unsigned cfgPeripheral, unsigned cfgSource)
Configures pigpio to use a particular sample rate timed by a specified peripheral.

This function is only effective if called before gpioInitialise.

    cfgMicros: 1, 2, 4, 5, 8, 10
cfgPeripheral: 0 (PWM), 1 (PCM)
    cfgSource: deprecated, value is ignored


The timings are provided by the specified peripheral (PWM or PCM).

The default setting is 5 microseconds using the PCM peripheral.

The approximate CPU percentage used for each sample rate is:

sample  cpu
 rate    %

  1     25
  2     16
  4     11
  5     10
  8     15
 10     14
*/
    if (gpioInitialise() < 0) {
        return -SPI_E_ISR_INIT_ERROR;
    }

    spihandle = spiOpen(SPI_CHANNEL, SPI_COMM_SPEED, SPI_FLAGS);
/*
int spiOpen(unsigned spiChan, unsigned baud, unsigned spiFlags)
This function returns a handle for the SPI device on the channel. Data will be transferred at baud bits per second. The flags may be used to modify the default behaviour of 4-wire operation, mode 0, active low chip select.

The Pi has two SPI peripherals: main and auxiliary.

The main SPI has two chip selects (channels), the auxiliary has three.

The auxiliary SPI is available on all models but the A and B.

The GPIO used are given in the following table.

MISO	MOSI	SCLK	CE0	CE1	CE2
Main SPI	9	10	11	8	7	-
Aux SPI	19	20	21	18	17	16


 spiChan: 0-1 (0-2 for the auxiliary SPI)
    baud: 32K-125M (values above 30M are unlikely to work)
spiFlags: see below


Returns a handle (>=0) if OK, otherwise PI_BAD_SPI_CHANNEL, PI_BAD_SPI_SPEED, PI_BAD_FLAGS, PI_NO_AUX_SPI, or PI_SPI_OPEN_FAILED.

spiFlags consists of the least significant 22 bits.

21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
 b  b  b  b  b  b  R  T  n  n  n  n  W  A u2 u1 u0 p2 p1 p0  m  m


mm defines the SPI mode.

Warning: modes 1 and 3 do not appear to work on the auxiliary SPI.

Mode POL PHA
 0    0   0
 1    0   1
 2    1   0
 3    1   1


px is 0 if CEx is active low (default) and 1 for active high.

ux is 0 if the CEx GPIO is reserved for SPI (default) and 1 otherwise.

A is 0 for the main SPI, 1 for the auxiliary SPI.

W is 0 if the device is not 3-wire, 1 if the device is 3-wire. Main SPI only.

nnnn defines the number of bytes (0-15) to write before switching the MOSI line to MISO to read data. This field is ignored if W is not set. Main SPI only.

T is 1 if the least significant bit is transmitted on MOSI first, the default (0) shifts the most significant bit out first. Auxiliary SPI only.

R is 1 if the least significant bit is received on MISO first, the default (0) receives the most significant bit first. Auxiliary SPI only.

bbbbbb defines the word size in bits (0-32). The default (0) sets 8 bits per word. Auxiliary SPI only.

The spiRead, spiWrite, and spiXfer functions transfer data packed into 1, 2, or 4 bytes according to the word size in bits.

For bits 1-8 there will be one byte per word.
For bits 9-16 there will be two bytes per word.
For bits 17-32 there will be four bytes per word.

Multi-byte transfers are made in least significant byte first order.

E.g. to transfer 32 11-bit words buf should contain 64 bytes and count should be 64.

E.g. to transfer the 14 bit value 0x1ABC send the bytes 0xBC followed by 0x1A.

The other bits in flags should be set to zero.
*/
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
