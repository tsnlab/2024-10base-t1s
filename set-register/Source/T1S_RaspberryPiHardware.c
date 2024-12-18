/***************************************************************************//**
* @mainpage :    10Base-T1S MACPHY - NCN26010
***************************************************************************//**
* @file     : T1S_RaspberryPiHardware.c
* @brief     : Implements T1S_Hardware.h to use pigpio library for RaspberryPi Hardware support
* @author     : Manjunath H M, Arndt Schuebel, Kyle Storey
* $Rev:    $
* $Date:    $
******************************************************************************
* @copyright (c) 2021 ON Semiconductor. All rights reserved.
* ON Semiconductor is supplying this software for use with ON Semiconductor
* ethernet products only.
*
* THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
* OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
* ON SEMICONDUCTOR SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL,
* INCIDENTAL, OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
*
* @details
*/

#include "T1S_Hardware.h"
#include <pigpio.h>

/*************************************************************************************************
*  Symbolic constants                                                                            *
*************************************************************************************************/
#define DIO0_GPIO_PIN                (uint32_t)23       //!< GPIO pin 23
#define MACPHY_INTERRUPT_GPIO_PIN    (uint32_t)25       //!< GPIO pin 25
#define INTERRUPT_TIMEOUT            (uint32_t)250      //!< 250ms timeout 
#define NO_INTERRUPT_TIMEOUT         (uint32_t)-1       //!< No timeout wait any amount of time until next interrupt
#define SPI_COMM_SPEED               (uint32_t)15000000 // 15MHz speed
#define SPI_CHANNEL                  (uint32_t)0        // Channel 0
#define SPI_FLAGS                    (uint32_t)0        // No special setup

/*************************************************************************************************
 *   Implementations
 ************************************************************************************************/
static int spiHandle;

//Configures to send and recieve SPI data to the NCN26010. Return true on success
uint32_t SPI_Init() 
{
  gpioCfgClock(1, 1, 1); //Settup sample rate to be 1 MHz.
  if (gpioInitialise() < 0)
  {
      return ISR_INIT_ERROR;
  }
  spiHandle = spiOpen(SPI_CHANNEL, SPI_COMM_SPEED, SPI_FLAGS);
  if (spiHandle > 0) 
  {
      return OK;
  }
  else
  {
      return SPI_INIT_ERROR;
  }
}

//transfer data over spi. Return true on success
uint32_t SPI_Transfer(uint8_t* rxBuffer, uint8_t* txBuffer, uint16_t numBytesToTransfer) 
{
    int numTransfered;
    numTransfered = spiXfer(spiHandle, (char*)txBuffer, (char*)rxBuffer, numBytesToTransfer);
    if (numTransfered > 0) {
        return OK;
    }
    else {
        return SPI_UNKNOWN_ERROR;
    }
}

static ISR_t dataReadyCallback;
static void PigpioDataReadyCallback(int gpio, int level, uint32_t tick)
{
    dataReadyCallback(tick);
}

// Register a callback for when we get a data ready interrupt
uint32_t SetDataReadyISR(ISR_t callback) 
{
    dataReadyCallback = callback;
    if (0 != (gpioSetISRFunc(MACPHY_INTERRUPT_GPIO_PIN, FALLING_EDGE, NO_INTERRUPT_TIMEOUT, PigpioDataReadyCallback))) 
    {
        return ISR_INIT_ERROR;
    }
    return OK;
}

static ISR_t p_TimestampCallback;
static void PigpioTimestampCallback(int gpio, int level, uint32_t tick)
{
    p_TimestampCallback(tick);
}
//register a callback for when we get a timestamp trigger interrupt
uint32_t SetTimestampISR(ISR_t callback) 
{

    p_TimestampCallback = callback;
    if (0 != gpioSetISRFunc(DIO0_GPIO_PIN, RISING_EDGE, NO_INTERRUPT_TIMEOUT, PigpioTimestampCallback)) 
    {
        return ISR_INIT_ERROR;
    }
    return OK;
}
//returns after the number of microseconds specified
void DelayMicroseconds(uint32_t microseconds) 
{
    gpioDelay(microseconds);
}

uint32_t SPI_Cleanup() 
{
    gpioTerminate();
    return OK;
}

