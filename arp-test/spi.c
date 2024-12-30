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

SPI_ReturnType SPI_Init(void) {

  gpioCfgClock(1, 1, 1); //Settup sample rate to be 1 MHz.
  if (gpioInitialise() < 0) {
      return SPI_E_ISR_INIT_ERROR;
  }

  spiHandle = spiOpen(SPI_CHANNEL, SPI_COMM_SPEED, SPI_FLAGS);
  if (spiHandle >= 0) {
      return SPI_E_SUCCESS;
  }
  else {
      return SPI_E_INIT_ERROR;
  }
}

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
    
    if (spiHandle >= 0) {
        spiClose(spiHandle);
        ret = SPI_E_SUCCESS;
    }

    gpioTerminate();
    return ret;
}

bool GetParity(uint32_t valueToCalculateParity) 
{
	valueToCalculateParity ^= valueToCalculateParity >> 1;
	valueToCalculateParity ^= valueToCalculateParity >> 2;
	valueToCalculateParity = ((valueToCalculateParity & 0x11111111U) * 0x11111111U);
	return ((valueToCalculateParity >> 28) & 1);
}

void ConvertEndianness(uint32_t valueToConvert, uint32_t *convertedValue)
{
  uint8_t position = 0;
  uint8_t variableSize = (uint8_t)(sizeof(valueToConvert));
  uint8_t tempVar = 0;
  uint8_t convertedBytes[(sizeof(valueToConvert))] = {0};

  bcopy((char *)&valueToConvert, convertedBytes, variableSize);      // cast and copy an uint32_t to a uint8_t array
  position = variableSize - (uint8_t)1;
  for (uint8_t byteIndex = 0; byteIndex < (variableSize/2); byteIndex++)  // swap bytes in this uint8_t array
  {       
      tempVar = (uint8_t)convertedBytes[byteIndex];
      convertedBytes[byteIndex] = convertedBytes[position];
      convertedBytes[position--] = tempVar;
  }
  bcopy(convertedBytes, (uint8_t *)convertedValue, variableSize);      // copy the swapped convertedBytes to an uint32_t
}
