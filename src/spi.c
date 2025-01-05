#include "spi.h"

#include "arch.h"
#include "arp_test.h"

#define DIO0_GPIO_PIN ((uint32_t)23)             //!< GPIO pin 23
#define MACPHY_INTERRUPT_GPIO_PIN ((uint32_t)25) //!< GPIO pin 25
#define INTERRUPT_TIMEOUT ((uint32_t)250)        //!< 250ms timeout
#define NO_INTERRUPT_TIMEOUT ((uint32_t)-1)      //!< No timeout wait any amount of time until next interrupt
#define SPI_COMM_SPEED ((uint32_t)15000000)      // 15MHz speed
#define SPI_CHANNEL ((uint32_t)0)                // Channel 0
#define SPI_FLAGS ((uint32_t)0)                  // No special setup

static int spihandle;

int spi_init(void) {

    gpioCfgClock(1, 1, 1); // Settup sample rate to be 1 MHz.
    if (gpioInitialise() < 0) {
        return SPI_E_ISR_INIT_ERROR;
    }

    spihandle = spiOpen(SPI_CHANNEL, SPI_COMM_SPEED, SPI_FLAGS);
    if (spihandle >= 0) {
        return SPI_E_SUCCESS;
    } else {
        return SPI_E_INIT_ERROR;
    }
}

int spi_transfer(uint8_t* rxbuffer, uint8_t* txbuffer, uint16_t length) {
    int numtransfered = -1;

    numtransfered = spiXfer(spihandle, (char*)txbuffer, (char*)rxbuffer, length);
    if (numtransfered > 0) {
        return SPI_E_SUCCESS;
    } else {
        return SPI_E_UNKNOWN_ERROR;
    }
}

int spi_cleanup(void) {
    int ret = SPI_E_UNKNOWN_ERROR;

    if (spihandle >= 0) {
        spiClose(spihandle);
        ret = SPI_E_SUCCESS;
    }

    gpioTerminate();
    return ret;
}

bool get_parity(uint32_t valueToCalculateParity) {
    valueToCalculateParity ^= valueToCalculateParity >> 1u;
    valueToCalculateParity ^= valueToCalculateParity >> 2u;
    valueToCalculateParity = ((valueToCalculateParity & 0x11111111U) * 0x11111111U);
    return ((valueToCalculateParity >> 28u) & 1u);
}

void convert_endianness(uint32_t valueToConvert, uint32_t* convertedValue) {
    uint8_t position = 0u;
    uint8_t variablesize = (uint8_t)(sizeof(valueToConvert));
    uint8_t tempvar = 0u;
    uint8_t convertedbytes[(sizeof(valueToConvert))] = {0u};

    bcopy((char*)&valueToConvert, convertedbytes, variablesize); // cast and copy an uint32_t to a uint8_t array
    position = variablesize - (uint8_t)1u;
    for (uint8_t byteIndex = 0u; byteIndex < (variablesize / 2u); byteIndex++) // swap bytes in this uint8_t array
    {
        tempvar = (uint8_t)convertedbytes[byteIndex];
        convertedbytes[byteIndex] = convertedbytes[position];
        convertedbytes[position--] = tempvar;
    }
    bcopy(convertedbytes, (uint8_t*)convertedValue, variablesize); // copy the swapped convertedbytes to an uint32_t
}
