#ifndef T1S_TCPIP_H
#define T1S_TCPIP_H
/***************************************************************************//**
* @mainpage :	10Base-T1S MACPHY - NCN26010
***************************************************************************//**
* @file 	: T1S_Hardware.h
* @brief 	: Defines function prototypes for interfacing with the TCP/IP layers of the network stack
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

#include "NCN26010.h"

/*************************************************************************************************
*  Configuration
*************************************************************************************************/
#define DEFAULT_MAC_ADDRESS           			(uint32_t) 0
#define DEFAULT_IP_ADDRESS             			(uint32_t) 0
//TODO FILL IN OTHER CONFIGURATION

/*************************************************************************************************
 *   Error Codes
 ************************************************************************************************/
#define OK	                             	    (uint32_t) 0
#define PEND		                 	        (uint32_t) 1
#define TCPIP_UNKNOWN_ERROR                     (uint32_t) 2
//TODO fill in other error codes

/*************************************************************************************************
*  		Global Variables                                                                 *
*************************************************************************************************/
extern uint8_t g_currentNodeMACAddr[SIZE_OF_MAC_ADDR];

/*************************************************************************************************
 *   Prototypes
 ************************************************************************************************/
uint32_t TCPIP_init(); //Initiallize the TCP/IP stack. Return true on success
//Calls T1S_Transmit when it wants to send something
//TCPIP_Receive is called when data should be recieved
//Cascade Specific code calls the setDataReadyISR to know when to collec that data
uint32_t TCPIP_Receive(uint8_t* buffer, uint16_t number_of_bytes_received);
uint32_t TCPIP_ReadTAPAndTransmit(bool *readTAPDataStatus);
uint32_t setMACAddress(uint32_t address); //calls RegWrite in NCN26010
uint32_t setIPAddress(uint32_t address); //

#endif /* T1S_TCPIP_H */
