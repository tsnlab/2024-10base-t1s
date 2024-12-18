/***************************************************************************//**
* @mainpage :	10Base-T1S MACPHY - NCN26010
***************************************************************************//**
* @file 	: T1S_TAPTCP-IP.c
* @brief 	: Implements T1S_TCP-IP.h to use the linux kernel TAP implementaiton of the TCP/IP Stack
* @author 	: Kyle Storey, Manjunath H M
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
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>  
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h> 
#include <sys/select.h>
#include "NCN26010.h"
#include "T1S_TCP-IP.h"
 
/*************************************************************************************************
* All Symbolic constants defined in NCN26010.h                                              *
*************************************************************************************************/
#define MAX_CONTROL_CMD_LEN			(uint8_t)(0x7F)
#define MAX_RCA_VALUE				(uint8_t)31 // max 5 bit value as RCA is 5bit value
#define MAX_PAYLOAD_BYTE 		    (uint8_t) 64 // ToDo This is configurable so need to change based on configuration
#define HEADER_FOOTER_SIZE			(uint8_t)4
#define ETHERNET_HEADER_SIZE		(uint8_t)14
#define MAX_DATA_DWORD_ONECHUNK 	MAX_REG_DATA_ONECHUNK
/*************************************************************************************************
*  Type definitions Enums and Unions                                                             *
*************************************************************************************************/
typedef union
{
	uint8_t receiveBuffer[MAX_RCA_VALUE * (MAX_PAYLOAD_BYTE+HEADER_FOOTER_SIZE)];
	uint8_t receiveBufferInChunks[MAX_RCA_VALUE][(MAX_PAYLOAD_BYTE+HEADER_FOOTER_SIZE)];
} uTAPReceiveBuffer_t;

/*************************************************************************************************
*  		Global Variables                                                                 *
*************************************************************************************************/

uint8_t g_currentNodeMACAddr[SIZE_OF_MAC_ADDR] = {0};

/*************************************************************************************************
*  		Local Variables                                                                  *
*************************************************************************************************/
static int32_t g_numberOfBytesReadFromTAP = 0;
static int16_t tapAllocResponse = 0;
static struct ifreq ifr;
static bool readTAPData = true;
static uTAPReceiveBuffer_t uVarBuffer;

/**********************************************************************************
 *   Prototypes
 ***********************************************************************************/
uint32_t SetMACAddress(uint32_t address); //calls RegWrite in NCN26010
uint32_t SetIPAddress(uint32_t address);

/*************************************************************************************************
*  Functions                                                                                     *
*************************************************************************************************/
// tap device stuff//
int TAP_Allocation(char *dev, int flags) 
{
	int fd, err;

	if((fd = open("/dev/net/tun", O_RDWR | O_NONBLOCK )) < 0 )
	{
		perror("Opening /dev/net/tun");
		return fd;
	}

	memset(&ifr, 0, sizeof(ifr));

	ifr.ifr_flags = flags;

	if (*dev)
	{
		strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	}

	if ((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0 ) 
	{
		perror("ioctl(TUNSETIFF)");
		close(fd);
		return err;
	}

	strcpy(dev, ifr.ifr_name);

	return fd;
}

//Initiallize the TCP/IP stack. Return true on success
uint32_t TCPIP_init() 
{
	char tap_name[IFNAMSIZ] = {'\0'};

	strcpy (tap_name,"tap0");
	tapAllocResponse = TAP_Allocation(tap_name, (IFF_TAP | IFF_NO_PI)); // tap interface
	if (tapAllocResponse < 0)
	{
		return TCPIP_UNKNOWN_ERROR;
	}

	///< Determine the MAC address of the interface
	strncpy (ifr.ifr_name, tap_name, (IFNAMSIZ-1));
	if ((ioctl(tapAllocResponse, SIOCGIFHWADDR, &ifr)) < 0)
	{
		return TCPIP_UNKNOWN_ERROR;
	}

	///< Copy 6 bytes MAC address into the array g_currentNodeMACAddr
	memcpy (&g_currentNodeMACAddr[0], ifr.ifr_hwaddr.sa_data, SIZE_OF_MAC_ADDR);
	/* Output */
	printf ("The hardware MAC address of %s, type %d is %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X \n", ifr.ifr_name, \
				ifr.ifr_hwaddr.sa_family, g_currentNodeMACAddr[0], g_currentNodeMACAddr[1], g_currentNodeMACAddr[2], g_currentNodeMACAddr[3], g_currentNodeMACAddr[4], g_currentNodeMACAddr[5]); 
	return OK;
}

//Calls T1S_Transmit when it wants to send something
//TCPIP_Receive is called when data should be recieved
//Cascade Specific code calls the setDataReadyISR to know when to collec that data
uint32_t TCPIP_Receive(uint8_t* buffer, uint16_t number_of_bytes_received) 
{
	uint32_t numberOfBytesWrittenToTAP;
	uint32_t returnStatus;

	numberOfBytesWrittenToTAP = write(tapAllocResponse, buffer, number_of_bytes_received);		 
	if (numberOfBytesWrittenToTAP == number_of_bytes_received)
	{
		returnStatus = OK;
	}
	else 
	{
		returnStatus = TCPIP_UNKNOWN_ERROR;
	}

	return returnStatus;
}

uint32_t TCPIP_ReadTAPAndTransmit(bool *readTAPDataStatus) 
{
	uint32_t errorCode;
	//!< This checks if any data from TAP to send over line
	if (readTAPData)
	{
		//memset(&uVarBuffer.receiveBuffer[0] , 0, sizeof(uVarBuffer.receiveBuffer));
		g_numberOfBytesReadFromTAP = read(tapAllocResponse, &uVarBuffer.receiveBuffer[0], sizeof(uVarBuffer.receiveBuffer));
		//if (g_numberOfBytesReadFromTAP>0) printf ("Got %d Bytes from TAP0 \n",g_numberOfBytesReadFromTAP);
		if (g_numberOfBytesReadFromTAP >= ETHERNET_HEADER_SIZE)
		{
			// Reads Buffer Status register from MMS 0
			ReadBufferStatusReg();
			readTAPData = false;	
		}
	}

	#if 0 // ToDo: Written twice during modification by Kyle, any specific reason?
	//!< This checks if any data from TAP to send over line
	if (readTAPData)
	{
		//memset(&uVarBuffer.receiveBuffer[0] , 0, sizeof(uVarBuffer.receiveBuffer));
		g_numberOfBytesReadFromTAP = read(tapAllocResponse, &uVarBuffer.receiveBuffer[0], sizeof(uVarBuffer.receiveBuffer));
		//if (g_numberOfBytesReadFromTAP>0) printf ("Got %d Bytes from TAP0 \n",g_numberOfBytesReadFromTAP);
		if (g_numberOfBytesReadFromTAP >= ETHERNET_HEADER_SIZE)
		{
			// Reads Buffer Status register from MMS 0
			readTAPData = false; // when TXC is less then don't read data again till it's cleared	
			return PEND;
		}
	}
	#endif

	if ((g_transmitCreditsAvailable_TXC > ((g_numberOfBytesReadFromTAP/g_maxPayloadSize) + 1)) && (g_numberOfBytesReadFromTAP >= ETHERNET_HEADER_SIZE))
	{
		if ((errorCode = T1S_Transmit(&uVarBuffer.receiveBuffer[0], g_numberOfBytesReadFromTAP)) != OK)
		{
			printf("Data transfer failed \n");
			return errorCode;
		}
		else
		{			
			readTAPData = true;
			g_numberOfBytesReadFromTAP = 0;
		}
	}
	else
	{
		if (g_numberOfBytesReadFromTAP >= ETHERNET_HEADER_SIZE)
		{
			readTAPData = false; // when TXC is less then don't read data again till it's cleared
		}
	}
	/*if (readTAPData == false)
	{
		return PEND;
	}*/
	*readTAPDataStatus = readTAPData;
	return OK;
}

uint32_t SetMACAddress(uint32_t address)
{

	return 1;
}

uint32_t SetIPAddress(uint32_t address)
{

	return 1;
}
