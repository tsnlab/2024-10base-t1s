#include <arp_test.h>
#include <hardware_dependent.h>
#include <spi.h>

#define DIO0_GPIO_PIN                (uint32_t)23       //!< GPIO pin 23
#define MACPHY_INTERRUPT_GPIO_PIN    (uint32_t)25       //!< GPIO pin 25
#define INTERRUPT_TIMEOUT            (uint32_t)250      //!< 250ms timeout 
#define NO_INTERRUPT_TIMEOUT         (uint32_t)-1       //!< No timeout wait any amount of time until next interrupt
#define SPI_COMM_SPEED               (uint32_t)15000000 // 15MHz speed
#define SPI_CHANNEL                  (uint32_t)0        // Channel 0
#define SPI_FLAGS                    (uint32_t)0        // No special setup

static int spiHandle;

//Configures to send and recieve SPI data to the NCN26010. Return true on success
SPI_ReturnType SPI_Init(void) {

  gpioCfgClock(1, 1, 1); //Settup sample rate to be 1 MHz.
  if (gpioInitialise() < 0) {
      return SPI_E_ISR_INIT_ERROR;
  }

  spiHandle = spiOpen(SPI_CHANNEL, SPI_COMM_SPEED, SPI_FLAGS);
  if (spiHandle > 0) {
      return SPI_E_SUCCESS;
  }
  else {
      return SPI_E_INIT_ERROR;
  }
}

//transfer data over spi. Return true on success
SPI_ReturnType SPI_Transfer(uint8_t* rxBuffer, uint8_t* txBuffer, uint16_t length) {
    int numTransfered = -1;

    numTransfered = spiXfer(spiHandle, (char*)txBuffer, (char*)rxBuffer, length);
    if (numTransfered > 0) {
        return SPI_E_SUCCESS;
    }
    else {
        return SPI_E_UNKNOWN_ERROR;
    }
}

SPI_ReturnType SPI_Cleanup(void) { 
    SPI_ReturnType ret = SPI_E_UNKNOWN_ERROR;
    
    if (spiHandle > 0) {
        spiClose(spiHandle);
        ret = SPI_E_SUCCESS;
    }

    gpioTerminate();
    return ret;
}
