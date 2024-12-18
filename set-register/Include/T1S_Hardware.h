#ifndef T1S_HARDWARE_H
#define T1S_HARDWARE_H
/***************************************************************************//**
* @mainpage :	10Base-T1S MACPHY - NCN26010
***************************************************************************//**
* @file 	: T1S_Hardware.h
* @brief 	: Defines function prototypes for hardware specific functions required by NCN26010
* @author 	: Arndt Schuebel, Kyle Storey
* $Rev:	$
* $Date:	$
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

#include <stdint.h>
typedef void (*ISR_t)(uint32_t tick);
typedef void (*ErrorCallback_t)(uint32_t error);

/*************************************************************************************************
 *   Error Codes
 ************************************************************************************************/
#define OK                               	(uint32_t) 0
#define SPI_UNKNOWN_ERROR                 	(uint32_t) 1
#define SPI_INIT_ERROR                   	(uint32_t) 2
#define ISR_INIT_ERROR                   	(uint32_t) 3
//TODO fill in other error codes

/*************************************************************************************************
 *   Prototypes
 ************************************************************************************************/
uint32_t SPI_Init(); //Configures to send and recieve SPI data to the NCN26010. Return true on success
uint32_t SPI_Transfer(uint8_t* rx_buffer, uint8_t* tx_buffer, uint16_t num_bytes_to_transfer); //transfer data over spi. Return true on success
uint32_t SPI_GetStatus();
uint32_t SPI_SetErrorCallback(ErrorCallback_t callback); //
uint32_t SetDataReadyISR(ISR_t callback); //register a callback for when we get a data ready interrupt
uint32_t SetTimestampISR(ISR_t callback); //register a callback for when we get a timestamp trigger interrupt
void DelayMicroseconds(uint32_t microseconds); //returns after the number of microseconds specified
uint32_t SPI_Cleanup();


#endif /*T1S_HARDWARE_H*/
