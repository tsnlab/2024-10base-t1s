/***************************************************************************//**
* @mainpage :	10Base-T1S MACPHY - NCN26010
***************************************************************************//**
* @file 	: NCN26010_Main.c
* @brief 	: Project - 10Base-T1S MACPHY(NCN26010) Begins here
* @author 	: Manjunath H M, Arndt Schuebel, Kyle Storey
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
/*************************************************************************************************
*  Header files                                                                                  *
*************************************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>  
#include <stdlib.h>
#include <string.h>
#include <pigpio.h>
#include <stdarg.h>
#include "NCN26010.h"

/*************************************************************************************************
*  Symbolic constants                                                                            *
*************************************************************************************************/
#define INTERRUPT_TIMEOUT			(uint32_t)250	//!< 250ms timeout 

/*************************************************************************************************
 *   Prototypes
 ************************************************************************************************/
void Report_MAC_Statistics(void);
void ProcessReceivedCommandAndRespond(char * ptrCommandString);
uint8_t ASCIIValueConversion(uint8_t asciiValue);

/*************************************************************************************************
*  Functions                                                                                     *
*************************************************************************************************/

void Report_MAC_Statistics(void)
{
	const char regNames[35][52] = {
		"Sent Bytes Low: ", "Sent Bytes High: ", "Frames Sent OK:", "Broadcast Frames sent OK:",
		"Multicast Frames Sent OK:", "64-Byte Frames Sent OK:", "65 - 127 Byte frames sent OK", "128 - 255 Byte Frames sent OK:", 
		"256 - 511 Byte Frames sent OK:", "510 - 1023 Byte Frames sent OK:", "1024 or larger byte Frames sent OK:",
		"Frames aborted due to TX-buffer underflow:", "Frames transmitted after singel collision:", 
		"Frames transmitted after multiple collisions:", "Frames transmitted after excessive collisions:", "Frames transmitted after deferal:",
		"CRS de-assertions during frame transmission Errors:",
		"Receive Bytes Counter Low:", "Received Bytes Counter high","Frames received OK:",
		"Broadcast frames received OK:", "Multicast frames received OK:", "64 Byte Frames resceived OK:",
		"65 - 127 Byte frames received OK:", "128-255 Byte frames received OK:", "256-511 Byte frames received OK:",
		"512-1023 Byte Frames recevied OK:", "1024 Byte Frames received OK:", "Droped Frames too short errors:",
		"Dropped frame to long errors:", "droped frame due to wrong FCS errors:", "symbol errors during frame reception:",
		"Align errors during frame reception:", "RX buffer overflow orrors", "RX dropped frame count:" };
	system ("clear");
	uint32_t databuffer[35];
	for (uint32_t i = 0; i < 35; i++) {
		uint16_t address = 0x30 + i;
		databuffer[i] = T1S_RegRead(1, address);
		printf ("MMS1 - Address 0x%04x -> %s %d \n",address, regNames[i],databuffer[i]);
	}
}	

uint8_t ASCIIValueConversion(uint8_t asciiValue)
{
	uint8_t returnIntValue = 0;

	if ((asciiValue >= 'A') && (asciiValue <= 'F'))
	{
		returnIntValue = (asciiValue - 0x41) + 0x0A;
	}
	else if ((asciiValue >= 'a') && (asciiValue <= 'f'))
	{
		returnIntValue = (asciiValue - 0x61) + 0x0A;
	}
	else if ((asciiValue >= '0') && (asciiValue <= '9'))
	{
		returnIntValue = asciiValue - 0x30;
	}
	else
	{
		printf("Invalid input : %c \n", asciiValue);
	}

	return returnIntValue;
}

void ProcessReceivedCommandAndRespond(char* ptrCommandString)
{
	uint8_t bufferPtrIndex = 0;
	uint8_t mmsValue = 0;
	uint16_t regAddress = 0;

	switch (ptrCommandString[0])
	{
	case 'R':
	case 'r':
	{
		//uint8_t ignoreHEXCharInput = 0;

		// Expected format: R <MMS> <address>      MMS: 1 byte, address: 2 byte data in HEX format
		while (((ptrCommandString[bufferPtrIndex++] != ' ') && (ptrCommandString[bufferPtrIndex] != '\0')) && (bufferPtrIndex < 20)); // Run till space characeter is found

		while (((ptrCommandString[bufferPtrIndex] != ' ') && (ptrCommandString[bufferPtrIndex] != '\0')) && (bufferPtrIndex < 20))
		{
			mmsValue = (mmsValue * 10) + (ASCIIValueConversion(ptrCommandString[bufferPtrIndex]));
			//printf("MMS value is 0x%02X, convertedASCII is 0x%02X, actualvalue : %c\n", mmsValue, (ASCIIValueConversion(ptrCommandString[bufferPtrIndex])), ptrCommandString[bufferPtrIndex]);
			bufferPtrIndex++;
		}
		bufferPtrIndex++; // To go past SPACE character

		while (((ptrCommandString[bufferPtrIndex] != '\n') && (ptrCommandString[bufferPtrIndex] != '\0')) && (bufferPtrIndex < 20))
		{
			//if (ignoreHEXCharInput >= 2) // Ignores '0x' characters
			{
				//regAddress = (regAddress * 0x10) + asciiToHexValue;
				regAddress = (regAddress * 0x10) + (ASCIIValueConversion(ptrCommandString[bufferPtrIndex]));
				//printf("reg Addr : 0x%04X, convertedASCII is 0x%02X, actualvalue : %c\n", regAddress, (ASCIIValueConversion(ptrCommandString[bufferPtrIndex])), ptrCommandString[bufferPtrIndex]);
			}
			//ignoreHEXCharInput++;
			bufferPtrIndex++;
		}

		// Reads register from MMS, given user input
		printf("Before Read- Memory map: %d, Reg address : 0x%04X\n", mmsValue, regAddress);
		uint32_t addressValue = T1S_RegRead(mmsValue, regAddress);
		printf("Register value of register 0x%04X from MMS %d is 0x%08X \n", regAddress, mmsValue, addressValue);
	}
	break;

	case 'W':
	case 'w':
	{
		//uint8_t ignoreHEXCharInput = 0;
		uint32_t regValueToWrite = 0;
		regAddress = 0;
		mmsValue = 0;

		// Expected format: W <MMS> <address> <RegDataToWrite>     MMS: 1 byte, address: 2 byte data, RegDataToWrite: 4bytes in HEX format
		while (((ptrCommandString[bufferPtrIndex++] != ' ') && (ptrCommandString[bufferPtrIndex] != '\0')) && (bufferPtrIndex < 30)); // Run till space characeter is found

		while (((ptrCommandString[bufferPtrIndex] != ' ') && (ptrCommandString[bufferPtrIndex] != '\0')) && (bufferPtrIndex < 30))
		{
			//mmsValue = (mmsValue * 0x10) + (ptrCommandString[bufferPtrIndex] - 0x30);
			mmsValue = (mmsValue * 10) + (ASCIIValueConversion(ptrCommandString[bufferPtrIndex]));
			bufferPtrIndex++;
		}
		bufferPtrIndex++;

		while (((ptrCommandString[bufferPtrIndex] != ' ') && (ptrCommandString[bufferPtrIndex] != '\0')) && (bufferPtrIndex < 30))
		{
			//if (ignoreHEXCharInput >= 2) // Ignores '0x' characters
			{
				//regAddress = (regAddress * 0x10) + (ptrCommandString[bufferPtrIndex] - 0x30);
				regAddress = (regAddress * 0x10) + (ASCIIValueConversion(ptrCommandString[bufferPtrIndex]));
			}
			//ignoreHEXCharInput++;
			bufferPtrIndex++;
		}
		bufferPtrIndex++;

		//ignoreHEXCharInput = 0;
		while (((ptrCommandString[bufferPtrIndex] != '\n') && (ptrCommandString[bufferPtrIndex] != '\0')) && (bufferPtrIndex < 30))
		{
			//if (ignoreHEXCharInput >= 2) // Ignores '0x' characters
			{
				regValueToWrite = (regValueToWrite * 0x10) + (ASCIIValueConversion(ptrCommandString[bufferPtrIndex]));
				//regValueToWrite = (regValueToWrite * 0x10) + (atoi(ptrCommandString[bufferPtrIndex]));
			}
			//ignoreHEXCharInput++;
			bufferPtrIndex++;
		}

		// Write register as per user input

		printf("Before Write- Memory map: %d, Reg address : 0x%04X, Reg value 0x%08X\n", mmsValue, regAddress, regValueToWrite);

		T1S_RegWrite(mmsValue, regAddress, regValueToWrite);
		uint32_t addressValue = T1S_RegRead(mmsValue, regAddress);
		printf("Register value read after writting for register 0x%04x from MMS %d is 0x%08X \n", regAddress, mmsValue, addressValue);
	}
	break;

	case 'S':
	case 's':
		Report_MAC_Statistics();
		break;

	case 'C':
	case 'c':
	{
		uint8_t temp_var = 0;
		uint8_t numCommands = 0;
		uint8_t configType = 0;
		uint32_t regValueToWrite = 0;
		uint8_t targetMAC[SIZE_OF_MAC_ADDR];
		stRemoteConfigurationCommand_t commands[100];
		bufferPtrIndex = 0;

		// Expected format: C   <DestMACAddr>   <NumOfConfig> <ConfigType> <MMS> <address> <RegDataToWrite-If_CTisW> <ConfigType> <MMS> <address>    
		// 					C 6c:7d:3f:42:18:29        2            1        0      01AF       78D9453A                  0          1     007E   
		//	DestMACAddr:6bytes with ':' separator, NumOfConfig: 1 byte, ConfigType: 1 byte, MMS: 1 byte, address: 2 byte data, RegDataToWrite: 4bytes in HEX format
		while (((ptrCommandString[bufferPtrIndex++] != ' ') && (ptrCommandString[bufferPtrIndex] != '\0')) && (bufferPtrIndex < 100)); // Run till space characeter is found
		//printf("Total Bytes is %d, Step 1 passed\n", sendFrameIndex);
		uint8_t macIndex = 0;
		while (((ptrCommandString[bufferPtrIndex] != ' ') && (ptrCommandString[bufferPtrIndex] != '\0')) && (bufferPtrIndex < 100)) // Dest MAC address
		{
			if (ptrCommandString[bufferPtrIndex] != ':')
			{
				temp_var = (uint8_t)((temp_var * 0x10) + (ASCIIValueConversion(ptrCommandString[bufferPtrIndex])));
			}
			else
			{
				//printf("%02X ", temp_var);
				targetMAC[macIndex++] = temp_var;
				temp_var = 0;
			}
			bufferPtrIndex++;
		}
		bufferPtrIndex++;
		//printf("%02X", temp_var);
		targetMAC[macIndex++] = temp_var;
		/*printf("\nDest MAC Address is %02X:%02X:%02X:%02X:%02X:%02X \n", sendConfigFrameBuff[sendFrameIndex-6], sendConfigFrameBuff[sendFrameIndex-5],\
		 sendConfigFrameBuff[sendFrameIndex-4], sendConfigFrameBuff[sendFrameIndex-3], sendConfigFrameBuff[sendFrameIndex-2], sendConfigFrameBuff[sendFrameIndex-1]);*/

		while (((ptrCommandString[bufferPtrIndex] != ' ') && (ptrCommandString[bufferPtrIndex] != '\0')) && (bufferPtrIndex < 100)) // Number of Config
		{
			numCommands = (numCommands * 10) + (ASCIIValueConversion(ptrCommandString[bufferPtrIndex]));
			bufferPtrIndex++;
		}
		bufferPtrIndex++;

		for (uint8_t configLoaded = 0; configLoaded < numCommands; configLoaded++)
		{
			configType = 0;
			while (((ptrCommandString[bufferPtrIndex] != ' ') && (ptrCommandString[bufferPtrIndex] != '\0')) && (bufferPtrIndex < 100)) // Config Type
			{
				configType = (configType * 0x10) + (ASCIIValueConversion(ptrCommandString[bufferPtrIndex]));
				bufferPtrIndex++;
			}
			bufferPtrIndex++;
			commands[configLoaded].type = configType;
			//printf("Total Bytes is %d, Step %d passed\n", sendFrameIndex, (configLoaded+4));

			mmsValue = 0;
			while (((ptrCommandString[bufferPtrIndex] != ' ') && (ptrCommandString[bufferPtrIndex] != '\0')) && (bufferPtrIndex < 100))
			{
				mmsValue = (mmsValue * 0x10) + (ASCIIValueConversion(ptrCommandString[bufferPtrIndex]));
				//printf("MMS value is 0x%02X, convertedASCII is 0x%02X, actualvalue : %c\n", mmsValue, (ASCIIValueConversion(ptrCommandString[bufferPtrIndex])), ptrCommandString[bufferPtrIndex]);
				bufferPtrIndex++;
			}
			bufferPtrIndex++; // To go past SPACE character
			//printf("Total Bytes is %d, Step %d passed\n", sendFrameIndex, (configLoaded+5));

			commands[configLoaded].MMS = mmsValue;

			regAddress = 0;
			while ((((ptrCommandString[bufferPtrIndex] != ' ') && (ptrCommandString[bufferPtrIndex] != '\0')) \
				&& ptrCommandString[bufferPtrIndex] != '\n') && (bufferPtrIndex < 100)) // Reg Address in MMS
			{
				regAddress = (regAddress * 0x10) + (ASCIIValueConversion(ptrCommandString[bufferPtrIndex]));
				//printf("reg Addr : 0x%04X, convertedASCII is 0x%02X, actualvalue : %c\n", regAddress, (ASCIIValueConversion(ptrCommandString[bufferPtrIndex])), ptrCommandString[bufferPtrIndex]);
				bufferPtrIndex++;
			}
			//bufferPtrIndex++;
			//printf("Total Bytes is %d, Step %d passed\n", sendFrameIndex, (configLoaded+6));
			bufferPtrIndex++; // To go past SPACE character

			commands[configLoaded].address = regAddress;

			if (configType == 1)
			{
				regValueToWrite = 0;
				while ((((ptrCommandString[bufferPtrIndex] != ' ') && (ptrCommandString[bufferPtrIndex] != '\0')) \
					&& ptrCommandString[bufferPtrIndex] != '\n') && (bufferPtrIndex < 100))
				{
					//printf("%c", ptrCommandString[bufferPtrIndex]);
					regValueToWrite = (regValueToWrite * 0x10) + (ASCIIValueConversion(ptrCommandString[bufferPtrIndex]));
					bufferPtrIndex++;
				}
				//printf("\nReg value to write is 0x%08X\n", regValueToWrite);
				bufferPtrIndex++;
				commands[configLoaded].data = regValueToWrite;
				//printf("Total Bytes is %d, Step %d passed Config Type 1\n", sendFrameIndex, (configLoaded+7));
			}
			//printf("1. Reg Remote config, NumOfConfig %d, Config Type %d, MMS 0x%02X, Reg Addr  0x%04X\n", numCommands, configType, regAddress);
		}
		T1S_SendRemoteConfiguration(targetMAC, commands, numCommands);
	}
	break;

	case 'Q':
	case 'q':
		NCN26010_Exit();
		printf("Program QUIT command received\n");
		break;

    default:
		printf("Invalid command Received Not Read, Write, Statistics, Config or Quit\n");
		break;
	}
}

void CommandProcessCallback(void)
{	
	char *line = NULL;
    size_t len = 0;
    ssize_t readLine = 0;
		
	printf("Enter Command> ");
	readLine = getline(&line, &len, stdin);
	if (readLine == -1)
	{
		//printf("Command not entered\n");
	}
	else
	{
		printf("Command Entered is %s \n", line);
		ProcessReceivedCommandAndRespond(line);
	}
	free(line);
}

int main(int argc , char *argv[])
{
	bool serverModeEnable = false;
	gpioTimerFunc_t ptrCommandProcessCallback = CommandProcessCallback;

	//printf("Argument Count is %d\n", argc);
	for (uint8_t argCount = 1; argCount < argc; argCount++)
	{
		//printf("Argument Count is %d, argv value is %s\n", argCount, argv[argCount]);
		if (strcmp(argv[argCount], "-plca") == 0) //request PLCA
		{
			int plcaID = 0;
			int plcaMaxID = 0;
			if (argCount+1 < argc)
		       	{
				plcaID = atoi(argv[argCount+1]);
				if (plcaID == 0)
			       	{
					if (argCount+2 < argc)
				       	{
						plcaMaxID = atoi(argv[argCount+2]);
					}
					else
				       	{
						printf("USAGE:\n -plca 0 <maxID>\n -plca <ID>\n");
						continue;
					}
				}
			}
			else
		       	{
				printf("USAGE:\n -plca 0 <maxID>\n -plca <ID>\n");
				continue;
			}	
			T1S_ConfigurePLCA((uint8_t) plcaID, (uint8_t) plcaMaxID, true);
			printf("Configuring PLCA , Node ID :%d, max_node_ID : %d\n", plcaID, plcaMaxID);
		}
		else if (strcmp(argv[argCount], "ServerMode") == 0) // ServerMode check
		{
			serverModeEnable = true;
			printf("Server Mode is ON\n");
		}
		else if (strcmp(argv[argCount], "-help") == 0)
		{
			printf("Command Line Options:\n\t-plca 0 <maxID>\n\t\tConfigures the NCN26010 with PLCA ID 0. The device will serve as the PLCA leader.\n\t-plca <id>\n\t\tConfigures the NCN26010 with the given PLCA ID. The device will NOT serve as the PLCA leader.\n\tServerMode\n\t\tDisables Interactive Command Line.\n\nInteractive Mode Commands:\n\tR[ead] <MMS> <address> MMS: 1 byte, address: 2 byte data in HEX format.\n\t\tReads the requested register and prints the result.\n\tW[rite] <MMS> <address> <data> MMS: 1 byte, address: 2 byte, data: 4bytes in HEX format.\n\t\tWrites the provided data to the given address.\n\tS[tatistics]\n\t\tPrints a report of statistics about the T1S communications.\n\tC[onfig] <DestMACAddr> <NumOfConfig> <ConfigType> [<MMS> <address> <data>]+\n\t\tSends commands to remotly configure the device with the given MAC address.\n\tQ[uit]\n\t\tStops the driver and exits the program.\n");
			return 0;
		}
		else
		{
			// Ignore if any other commands than these listed above
		}
	}

	if (NCN26010_Init() != OK) {
		perror ("Error initializing NCN26010 interface");
		exit(1);
	}

	if (!serverModeEnable) ///< If ServerMode is passed as command line argument then disable this dynamic read write register
	{
		if (0 != (gpioSetTimerFunc(1, INTERRUPT_TIMEOUT, ptrCommandProcessCallback)))
		{
			printf("Configuring callback for command processing at 250ms failed\n");
		}
	}

	NCN26010_Begin();
	printf("\nProgram Execution complete\n");
	
	return 0;
}



