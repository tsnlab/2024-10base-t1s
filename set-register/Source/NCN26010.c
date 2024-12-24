/***************************************************************************//**
* @mainpage :    10Base-T1S MACPHY - NCN26010
***************************************************************************//**
* @file     : NCN26010.c
* @brief     : Support functions to simplify utilization of NCN26010 Features
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
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include "NCN26010.h"
#include "T1S_Hardware.h"
#include "T1S_TCP-IP.h"

/*************************************************************************************************
*  Symbolic constants                                                                            *
*************************************************************************************************/
#define MAX_CONTROL_CMD_LEN			(uint8_t)(0x7F)
#define HEADER_FOOTER_SIZE			(uint8_t)4
#define MAX_REG_DATA_ONECONTROLCMD	(uint8_t)(MAX_CONTROL_CMD_LEN * EACH_REG_SIZE)
#define MESSAGETYPE_SEND			(uint8_t)0
#define MESSAGETYPE_RESPONSE		(uint8_t)1
#define CONFIGTYPE_REGREAD			(uint8_t)0
#define CONFIGTYPE_REGWRITE			(uint8_t)1
#define ETHERNET_HEADER_SIZE		(uint8_t)14
#define ETHERNET_TYPE_MACPHYCONFIG	(uint16_t)0x0909
#define MAX_RCA_VALUE				(uint8_t)31 // max 5 bit value as RCA is 5bit value
#define CONFIG_FILE_MAXLINE_LEN		(uint8_t)100
#define FCS_POLYNOMIAL 				(uint32_t)0xEDB88320
#define SIZE_OF_FCS					(uint8_t)4

//#define DEBUG_ENABLE				// Debug mode MACRO to enable printf statement
/*************************************************************************************************
*  Type definitions Enums and Unions                                                             *
*************************************************************************************************/
typedef enum
{
	DNC_COMMANDTYPE_CONTROL = 0,
	DNC_COMMANDTYPE_DATA
} DNC_COMMANDTYPE;

typedef enum
{
	REG_ADDR_INCREMENT_ENABLE = 0,
	REG_ADDR_INCREMENT_DISABLE
} REG_ADDR_INCREMENT;

typedef enum
{
	REG_COMMAND_TYPE_READ = 0,
	REG_COMMAND_TYPE_WRITE
} REG_COMMAND_TYPE;

typedef union
{
	uint8_t controlHeaderArray[HEADER_FOOTER_SIZE];
	uint32_t controlFrameHead;
	struct stHeadFootBits
	{
		uint32_t P		: 1;
		uint32_t LEN  	: 7;
		uint32_t ADDR 	: 16;
		uint32_t MMS	: 4;
		uint32_t AID	: 1;
		uint32_t WNR	: 1;
		uint32_t HDRB	: 1;
		uint32_t DNC	: 1;
	}stVarHeadFootBits;
} uCommandHeaderFooter_t;

/*************************************************************************************************
 *   	Structures
 *************************************************************************************************/
typedef struct
{
	uint8_t memoryMap;
	uint8_t length;
	uint16_t address;
	uint32_t databuffer[MAX_CONTROL_CMD_LEN];
}stControlCmdReg_t;

typedef struct
{
	uint8_t startValid;
	uint8_t startWordOffset;
	uint8_t endValid;
	uint8_t endValidOffset;
	//uint32_t databuffer[MAX_DATA_DWORD_ONECHUNK];
	uint8_t transmitCredits;
	uint8_t receiveChunkAvailable;
}stDataTransfer_t;

typedef struct
{
	uint16_t totalBytesToTransfer;
	stDataTransfer_t stVarEachChunkTransfer;
}stBulkDataTransfer_t;

typedef struct
{
	uint8_t destMACAddr[6];
	uint8_t srcMACAddr[6];
	uint16_t ethernetType;
} stEthernetFrame_t;

typedef union
{
	uint8_t receiveBuffer[MAX_RCA_VALUE * (MAX_PAYLOAD_BYTE+HEADER_FOOTER_SIZE)];
	uint8_t receiveBufferInChunks[MAX_RCA_VALUE][(MAX_PAYLOAD_BYTE+HEADER_FOOTER_SIZE)];
} uTAPReceiveBuffer_t;

typedef union
{
	uint32_t statusRegister0;
	struct stVarStatusReg0
	{
		uint32_t TXPE	: 1;
		uint32_t TXBOE 	: 1;
		uint32_t TXBUE 	: 1;
		uint32_t RXBOE	: 1;
		uint32_t LOFE	: 1;
		uint32_t HDRE	: 1;
		uint32_t RESETC	: 1;
		uint32_t PHYINT : 1;
		uint32_t TTSCAA : 1;
		uint32_t TTSCAB : 1;
		uint32_t TTSCAC : 1;
		uint32_t TXFCSE : 1;
		uint32_t CDPE	: 1;
		uint32_t RSVD	: 19;
	}stVarStatusReg0;
} uStatusReg0_t;

typedef struct
{
    uint64_t* data;
    uint16_t length;
    uint16_t head;
    uint16_t tail;
}timestampFIFO_t;

/*************************************************************************************************
*  		Global Variables                                                                 *
*************************************************************************************************/
uint8_t g_topoMACAddrReceived[SIZE_OF_MAC_ADDR] = {0};
uint8_t g_maxPayloadSize;
uint8_t g_transmitCreditsAvailable_TXC = 0;
const uint8_t g_broadCastMACAddr[SIZE_OF_MAC_ADDR]= {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint32_t g_plcaModeOfCurrentNode = 0;
double g_measuredLength = 0;
double g_topoLengthHighestValue = 0;
double g_topoLengthLeastValue = 0;
uint16_t g_topoMeasuredCount = 0;
//bool g_plcaModeDisabled =false;
//uint32_t g_plcaConfigBeforeTopo = 0;

/*************************************************************************************************
*  		Local Variables                                                                  *
*************************************************************************************************/
static uint8_t receiveMACPHYBuff[3000];
static uint8_t receiveChunksAvailable_RCA;
static bool isProcessOngoing;
static bool exitFlag = false;
static bool checkForDataReception;
static bool syncronizingTimestamps;
static bool g_isFCSEnabled = true;
static bool PLCA_MODE = false;
static uint8_t PLCA_ID = 0 ; // used when asked to run in PLCA mode using -plca command line option
static uint8_t PLCA_MAX_NODE = 0;
static uint16_t receiveDataIndex;
static uint32_t unsyncronizedChunks;

/*************************************************************************************************
 *   Prototypes
 ************************************************************************************************/
//PROTOTYPE FOR HELPER FUNCTIONS NOT DEFINED IN API
void ConfigurePLCAToMACHPHY(void);
void ConfigureMACHPHY(void);
void CheckIfFCSEnabled(void);
void CheckConfigFileAndConfigMACPHY(void);
uint32_t FCS_Calculator(const uint8_t * p_data, uint16_t dataSize);
bool T1S_ReceiveDataChunk(stDataReceive_t* p_dataReceiveInfo);
bool GetParity(uint32_t valueToCalculateParity);
void ConvertEndianness(uint32_t valueToConvert, uint32_t *convertedValue);
bool T1S_WriteMACPHYReg(stControlCmdReg_t *p_regData);
bool T1S_ReadMACPHYReg(stControlCmdReg_t *p_regInfoInput,  stControlCmdReg_t *p_readRegData);
uint32_t T1S_RegRead(uint8_t MMS, uint16_t Address);
uint32_t T1S_ClearStatus(void);
void T1S_InterpretEthernetTypeFrame(uint8_t *receiveMACPHYBuff, stEthernetFrame_t stVarEthernetFrame);
uint32_t T1S_Receive(stDataReceive_t* p_dataReceiveInfo, uint16_t num_chunks_to_collect);
//void timestampCallback(int gpio, int level, uint32_t tick, void* timestampFIFO);
uint32_t CheckRCABuffAndReceiveData(bool readTAPDataState);
void T1S_RemoteRegWrite(uint8_t *mac_addr, uint8_t mms, uint16_t reg_address, uint32_t regValueToWrite);
void T1S_RemoteRegRead(uint8_t *mac_addr, uint8_t mms, uint16_t reg_address);

/*************************************************************************************************
*  Functions                                                                                     *
*************************************************************************************************/
#if 0
//static int MACPHYCalls = 0;
void DataReadyCallback(uint32_t tick)
{
	//printf("MACPHYCallback: %d\n", MACPHYCalls++);
	(isProcessOngoing == false) ? (checkForDataReception = true) : (checkForDataReception = false);
}
#endif

int print_message(const char *format, ...)
{
    #ifdef DEBUG_ENABLE
    va_list args;
    va_start(args, format);

    vprintf(format, args);

    va_end(args);
	#endif

	return 0;
}

void ConfigurePLCAToMACHPHY(void)
{
	bool executionStatus;
	stControlCmdReg_t stVarReadRegInfoInput;
	stControlCmdReg_t stVarReadRegData;
	stControlCmdReg_t stVarWriteRegInput;

	if (PLCA_MODE) // User requested PLCA mode
	{
		// write node ID and max node count
		stVarWriteRegInput.memoryMap = 4;
		stVarWriteRegInput.length = 0;
		stVarWriteRegInput.address = 0xCA02;
		if (PLCA_ID == 0)
		{
			stVarWriteRegInput.databuffer[0] = PLCA_ID | (PLCA_MAX_NODE << 8); 
		}
		else
		{
			stVarWriteRegInput.databuffer[0] = PLCA_ID;
		}
		executionStatus = T1S_WriteMACPHYReg(&stVarWriteRegInput);
		if (executionStatus == false)
		{
			// ToDo Action to be taken if reading register fails
			printf("Writing into PLCA reg failed\n");
		}
		else
		{
			print_message("PLCA Config1 register was written successfully with value 0x%08x \n", stVarWriteRegInput.databuffer[0]);
		}

		stVarWriteRegInput.memoryMap = 4;
		stVarWriteRegInput.length = 0;
		stVarWriteRegInput.address = 0xCA01;
		stVarWriteRegInput.databuffer[0] =  0b1 << 15; // enable PLCA
		executionStatus = T1S_WriteMACPHYReg(&stVarWriteRegInput);
		if (executionStatus == false)
		{
			// ToDo Action to be taken if reading register fails
			printf("Writing into PLCA reg failed\n");
		}
		else
		{
			print_message("PLCA ENABLED! PLCA Config0 register was written successfully with value 0x%08x \n", stVarWriteRegInput.databuffer[0]);
		}

		stVarReadRegInfoInput.memoryMap = 0;
		stVarReadRegInfoInput.length = 0;
		stVarReadRegInfoInput.address = 0xCA01;
		memset(&stVarReadRegInfoInput.databuffer[0], 0, MAX_REG_DATA_ONECHUNK);
		executionStatus = T1S_ReadMACPHYReg(&stVarReadRegInfoInput, &stVarReadRegData);
		if (executionStatus == false)
		{
			// ToDo Action to be taken if reading register fails
			printf("Reading CONFIG0 reg failed\n");
		}
		else
		{
			print_message("PLCA CONFIG0 reg value is 0x%08x\n", stVarReadRegInfoInput.databuffer[0]);
		}
	}
	else  // Disable PLCA if not enabled during command line argument
	{
		stVarWriteRegInput.memoryMap = 4;
		stVarWriteRegInput.length = 0;
		stVarWriteRegInput.address = 0xCA01;
		stVarWriteRegInput.databuffer[0] =  0b0 << 15; // disable PLCA
		executionStatus = T1S_WriteMACPHYReg(&stVarWriteRegInput);
		if (executionStatus == false)
		{
			// ToDo Action to be taken if reading register fails
			printf("Writing into PLCA reg failed\n");
		}
		else
		{
			print_message("PLCA DISABLED! PLCA Config0 register was written successfully with value 0x%08x \n", stVarWriteRegInput.databuffer[0]);
		}

		stVarReadRegInfoInput.memoryMap = 0;
		stVarReadRegInfoInput.length = 0;
		stVarReadRegInfoInput.address = 0xCA01;
		memset(&stVarReadRegInfoInput.databuffer[0], 0, MAX_REG_DATA_ONECHUNK);
		executionStatus = T1S_ReadMACPHYReg(&stVarReadRegInfoInput, &stVarReadRegData);
		if (executionStatus == false)
		{
			// ToDo Action to be taken if reading register fails
			printf("Reading CONFIG0 reg failed\n");
		}
		else
		{
			print_message("PLCA CONFIG0 reg value is 0x%08x\n", stVarReadRegInfoInput.databuffer[0]);
		}
	}
}

void ConfigureMACHPHY(void)
{		
	bool executionStatus;
	stControlCmdReg_t stVarReadRegInfoInput;
	stControlCmdReg_t stVarReadRegData ;
	stControlCmdReg_t stVarWriteRegInput = {0};

	// set bot LED on, we need this temporarity for triggering topdisc.
	T1S_RegWrite(12, 0x0012, 0x00008D8F);
	
	// Write CONFIG0 register
	stVarWriteRegInput.memoryMap = 0;
	stVarWriteRegInput.length = 0;
	stVarWriteRegInput.address = 0x0004;
	stVarWriteRegInput.databuffer[0] = 0x0000BCC6; // BCC6	// ToDo : Change to 6 at last, 5 for 32byte chunk size
	executionStatus = T1S_WriteMACPHYReg(&stVarWriteRegInput);
	if (executionStatus == false)
	{
		// ToDo Action to be taken if reading register fails
		printf("Writing into Config reg failed\n");
	}
	else
	{
		print_message("CONFIG0 register was written successfully with value 0x%08x \n", stVarWriteRegInput.databuffer[0]);
	}

	// Reads CONFIG0 register from MMS 0
	stVarReadRegInfoInput.memoryMap = 0;
	stVarReadRegInfoInput.length = 0;
	stVarReadRegInfoInput.address = 0x0004;
	memset(&stVarReadRegInfoInput.databuffer[0], 0, MAX_REG_DATA_ONECHUNK);
	executionStatus = T1S_ReadMACPHYReg(&stVarReadRegInfoInput, &stVarReadRegData);
	if (executionStatus == false)
	{
		// ToDo Action to be taken if reading register fails
		printf("Reading CONFIG0 reg failed\n");
	}
	else
	{
		g_maxPayloadSize = (stVarReadRegData.databuffer[0] & 0x00000007);
		print_message("CONFIG0 reg value is 0x%08x\n", stVarReadRegData.databuffer[0]);
	}

	// Reads STATUS0 register from MMS 0
	stVarReadRegInfoInput.memoryMap = 0;
	stVarReadRegInfoInput.length = 0;
	stVarReadRegInfoInput.address = 0x0008;
	memset(&stVarReadRegInfoInput.databuffer[0], 0, MAX_REG_DATA_ONECHUNK);
	executionStatus = T1S_ReadMACPHYReg(&stVarReadRegInfoInput, &stVarReadRegData);
	if (executionStatus == false)
	{
		// ToDo Action to be taken if reading register fails
		printf("Reading STATUS0 reg failed\n");
	}
	else
	{
		print_message("STATUS0 reg value is 0x%08x\n", stVarReadRegData.databuffer[0]);
	}

	// Write PHY Link up register
	stVarWriteRegInput.memoryMap = 0;
	stVarWriteRegInput.length = 0;
	stVarWriteRegInput.address = 0xFF00;
	stVarWriteRegInput.databuffer[0] = 0x00001000;
	executionStatus = T1S_WriteMACPHYReg(&stVarWriteRegInput);
	if (executionStatus == false)
	{
		// ToDo Action to be taken if reading register fails
		printf("Writing into PHY Link up register failed\n");
	}
	else
	{
		print_message("PHY Link up register was written successfully with value 0x%08x \n", stVarWriteRegInput.databuffer[0]);
	}

	// Enable TX and RX, set MAC to be promiscuous
	stVarWriteRegInput.memoryMap = 1;
	stVarWriteRegInput.length = 0;
	stVarWriteRegInput.address = 0x0000;
	stVarWriteRegInput.databuffer[0] = 0x00000103;
	executionStatus = T1S_WriteMACPHYReg(&stVarWriteRegInput);
	if (executionStatus == false)
	{
		// ToDo Action to be taken if reading register fails
		printf("Enabling Tx and Rx register failed\n");
	}
	else
	{
		print_message("Enabling Tx and Rx also MAC successfully with value 0x%08x \n", stVarWriteRegInput.databuffer[0]);
	}

	// Reading if PLCA was Enabled
	stVarReadRegInfoInput.memoryMap = 4;
	stVarReadRegInfoInput.length = 1;
	stVarReadRegInfoInput.address = 0xCA01;
	memset(&stVarReadRegInfoInput.databuffer[0], 0, MAX_REG_DATA_ONECHUNK);
	executionStatus = T1S_ReadMACPHYReg(&stVarReadRegInfoInput, &stVarReadRegData);
	if (executionStatus == false)
	{
		// ToDo Action to be taken if reading register fails
		printf("Reading configuration 0 reg failed\n");
	}
	else
	{
		unsigned int  ID,NCNT,EN;
		ID = stVarReadRegData.databuffer[1] & 0x00000F;
		NCNT = (stVarReadRegData.databuffer[1] >> 8) & 0x00000F;
		EN = 	(stVarReadRegData.databuffer[0] >> 15) & 0x00000001;
		print_message("PLCA settings from Control registers MMS4, 0xCA01 & 0xCA02: ");
		print_message(" %08x ENABLE BIT : %d	NODE ID: %d	NODE COUNT: %d \n", stVarReadRegData.databuffer[0], EN, ID, NCNT);
	}	
}

void CheckIfFCSEnabled(void)
{
	bool executionStatus;
	stControlCmdReg_t stVarReadRegInfoInput;
	stControlCmdReg_t stVarReadRegData;

	// Reads FCSA bit in register 0x0000 from MMS 1
	stVarReadRegInfoInput.memoryMap = 1;
	stVarReadRegInfoInput.length = 0;
	stVarReadRegInfoInput.address = 0x0000;
	memset(&stVarReadRegInfoInput.databuffer[0], 0, MAX_REG_DATA_ONECHUNK);
	executionStatus = T1S_ReadMACPHYReg(&stVarReadRegInfoInput, &stVarReadRegData);
	if (executionStatus == false)
	{
		// ToDo Action to be taken if reading register fails
		printf("Reading FCSA bit failed (inside WriteReg)\n");
	}
	else
	{
		if (stVarReadRegData.databuffer[0] & 0x00000100)
		{
			print_message("MMS 1 FCS Auto Append has been enabled by setting bit 8 High\n");
			g_isFCSEnabled = false;//true;
		}
		else
		{
			print_message("MMS 1 FCS Auto Append has been disabled by setting Bit 8 Low\n");
			g_isFCSEnabled = true;// false;
		}
	}
}

void CheckConfigFileAndConfigMACPHY(void)
{
	FILE* Configfileptr;
	bool executionStatus;		
	stControlCmdReg_t stVarWriteRegInput = {0};

	Configfileptr = fopen("MACPHY_ConfigFile.txt", "r");
    if (Configfileptr == NULL)
	{
    	perror("Didn't find configuration file in path, so default configuration will be loaded into MACPHY\n");
		//RegisterReadAndConfig(gSPIHandler);
		ConfigureMACHPHY();
    }
	else
	{
		char loadConfigBuffer[CONFIG_FILE_MAXLINE_LEN] = {'\0'};
		uint8_t lineNum = 0;

		// -1 to allow room for NULL terminator for really long string
		while (fgets(loadConfigBuffer, CONFIG_FILE_MAXLINE_LEN - 1, Configfileptr))
		{
			lineNum++;
			uint8_t bufferIndex = 0;
			uint8_t mms_Config = 0;
			uint16_t macphy_RegAddress_Config = 0;
			uint32_t macphy_RegValue_Config = 0;
			uint8_t lineLen = 0;
			lineLen = strcspn(loadConfigBuffer, "\n");
			if (lineLen == 0) continue;

			// Remove trailing newline
			loadConfigBuffer[strcspn(loadConfigBuffer, "\n")] = '\0';
			//Uncomment below if running on windows
			//loadConfigBuffer[strcspn(loadConfigBuffer, "\r")] = '\0';
			//bufferIndex = 0;
			if (loadConfigBuffer[bufferIndex] != '#')	// If line is comment line in config file then ignore that line and go ahead
			{
				printf("Interpreted Line %d-> %s\n", lineNum, loadConfigBuffer);
				while ((loadConfigBuffer[bufferIndex] != ' ') && (loadConfigBuffer[bufferIndex] != '\0'))
				{
					mms_Config = (mms_Config * 0x10) + (ASCIIValueConversion(loadConfigBuffer[bufferIndex]));
					bufferIndex++;
				}
				bufferIndex++;

				while ((loadConfigBuffer[bufferIndex] != ' ') && (loadConfigBuffer[bufferIndex] != '\0'))
				{
					macphy_RegAddress_Config = (macphy_RegAddress_Config * 0x10) + (ASCIIValueConversion(loadConfigBuffer[bufferIndex]));
					bufferIndex++;
				}
				bufferIndex++;

				//while ((loadConfigBuffer[bufferIndex] != ' ') && (loadConfigBuffer[bufferIndex] != '\0'))
				while ((loadConfigBuffer[bufferIndex] != '\0') && ((loadConfigBuffer[bufferIndex] != '\r') && (loadConfigBuffer[bufferIndex] != '\n')))
				{
					macphy_RegValue_Config = (macphy_RegValue_Config * 0x10) + (ASCIIValueConversion(loadConfigBuffer[bufferIndex]));
					bufferIndex++;
				}
				printf("Interpreted Param values from Config file for MMS:0x%02X, Reg Addr: 0x%04X, Reg Value:0x%08X\n", \
								mms_Config, macphy_RegAddress_Config, macphy_RegValue_Config);
				stVarWriteRegInput.memoryMap = mms_Config;
				stVarWriteRegInput.address = macphy_RegAddress_Config;
				stVarWriteRegInput.databuffer[0] = macphy_RegValue_Config;
				stVarWriteRegInput.length = 0;
				
				executionStatus = T1S_WriteMACPHYReg(&stVarWriteRegInput);
				if (executionStatus == false)
				{
					printf("MACPHY register writing failed from Config file for MMS:0x%02X, Reg Addr: 0x%04X, Reg Value:0x%08X\n", \
								stVarWriteRegInput.memoryMap, stVarWriteRegInput.address, stVarWriteRegInput.databuffer[0]);
				}
				else
				{
					printf("MACPHY register writing Successful from Config file for MMS:0x%02X, Reg Addr: 0x%04X, Reg Value:0x%08X\n", \
								stVarWriteRegInput.memoryMap, stVarWriteRegInput.address, stVarWriteRegInput.databuffer[0]);
				}
			}
			else
			{
				// Ignore comment line
			}
		}
		fclose(Configfileptr);
	}

	stVarWriteRegInput.memoryMap = 0x0C;
	stVarWriteRegInput.address = 0x0012;
	stVarWriteRegInput.databuffer[0] = 0x00008D8F;
	stVarWriteRegInput.length = 0;
	
	executionStatus = T1S_WriteMACPHYReg(&stVarWriteRegInput);
	if (executionStatus == false)
	{
		printf("MACPHY register writing failed from Config file for MMS:0x%02X, Reg Addr: 0x%04X, Reg Value:0x%08X\n", \
					stVarWriteRegInput.memoryMap, stVarWriteRegInput.address, stVarWriteRegInput.databuffer[0]);
	}
	else
	{
		printf("MACPHY register writing Successful from Config file for MMS:0x%02X, Reg Addr: 0x%04X, Reg Value:0x%08X\n", \
					stVarWriteRegInput.memoryMap, stVarWriteRegInput.address, stVarWriteRegInput.databuffer[0]);
	}

	CheckIfFCSEnabled();
}

uint32_t NCN26010_Init(void) 
{
	SPI_Init();
	#if 0 // This interrupt is not enabled as we moved onto polling based data reception
	if (0 != (SetDataReadyISR(DataReadyCallback))) //!< Enables interrupt to check if any data to be read and assigns its ISR callback 
	{
		// ToDo: Handle Interrupt setup failed
		perror ("Error enabling interrupt\n");
	}
	#endif
	// ToDo Set TXFCSVE bit in CONFIG0 register if FCS and Frame padding need to be enabled

	// ToDo before doing any configuration setting, need to check if that option is enabled or not in MACPHY

	ConfigurePLCAToMACHPHY(); // PLCA configuration is done separately as rest of configuration is dependent on Configuration file existence

	CheckConfigFileAndConfigMACPHY(); // Check if any configuration file exists in folder path, is yes then configure based on file info,
										// if not then go ahead with default configuration

	if (TCPIP_init() != OK)
	{
		return INIT_ERROR;
	}

	return OK;
}

uint32_t FCS_Calculator(const uint8_t * p_data, uint16_t dataSize)
{
	uint32_t fcs = 0xFFFFFFFF;
    uint16_t buff_index;
	uint8_t data_bits;

    for (buff_index = 0; buff_index < dataSize; ++buff_index)
	{
        uint32_t val = (fcs ^ ((uint32_t) p_data[buff_index])) & 0xFF;
        
        for (data_bits = 0; data_bits < 8; ++data_bits) 
		{
            if (val & 1)
			{
                val = (val >> 1) ^ FCS_POLYNOMIAL;
			}
            else
			{
                val >>= 1;
			}
        }

        fcs = val ^ (fcs >> 8);
    }

    return ~fcs;
}

bool T1S_ReceiveDataChunk(stDataReceive_t* p_dataReceiveInfo)
{
	bool receiveDataStatus = true;
	uint8_t txBuffer[MAX_PAYLOAD_BYTE+HEADER_FOOTER_SIZE] = {0};
	uint8_t rxBuffer[MAX_PAYLOAD_BYTE+HEADER_FOOTER_SIZE] = {0};
	uint8_t bufferIndex = 0;
	uint8_t receiveDataStartOffset = 0;
	//uint8_t receiveDataEndOffset = MAX_PAYLOAD_BYTE;
	uint32_t bigEndianRxFooter = 0;
	static uDataHeaderFooter_t dataTransferHeader;
	uDataHeaderFooter_t datatransferRxFooter;

	dataTransferHeader.stVarTxHeadBits.DNC = DNC_COMMANDTYPE_DATA;
	//if (SEQE | CONFIG0) //ToDo Add check here to verify if SEQ check if enabled in config register
	{
		dataTransferHeader.stVarTxHeadBits.SEQ = ~dataTransferHeader.stVarTxHeadBits.SEQ;
	}
	//else
	{
		dataTransferHeader.stVarTxHeadBits.SEQ = 0;
	}

	dataTransferHeader.stVarTxHeadBits.NORX = 0;
	dataTransferHeader.stVarTxHeadBits.VS = 0;
	dataTransferHeader.stVarTxHeadBits.RSVD1 = 0;
	dataTransferHeader.stVarTxHeadBits.DV = 0;
	dataTransferHeader.stVarTxHeadBits.SV = 0;
	dataTransferHeader.stVarTxHeadBits.SWO = 0;
	dataTransferHeader.stVarTxHeadBits.RSVD2 = 0;
	dataTransferHeader.stVarTxHeadBits.EV = 0;
	dataTransferHeader.stVarTxHeadBits.EBO = 0;
	dataTransferHeader.stVarTxHeadBits.RSVD3 = 0;
	dataTransferHeader.stVarTxHeadBits.TSC = 0;
	dataTransferHeader.stVarTxHeadBits.P = 0;
	dataTransferHeader.stVarTxHeadBits.P = (!GetParity(dataTransferHeader.dataFrameHeadFoot));

	//ToDo Handle FCS and frame padding here by checking TXFCSVC bit in STDCAP register

	for (int8_t headerByteCount = 3; headerByteCount >= 0; headerByteCount--)
	{
		txBuffer[bufferIndex++] = dataTransferHeader.dataFrameHeaderBuffer[headerByteCount];
	}

	//memset(&txBuffer[bufferIndex], 0, MAX_PAYLOAD_BYTE); // This is not required as already buffer is initialised with 0

	//bufferIndex += MAX_PAYLOAD_BYTE;

	SPI_Transfer((uint8_t *)&rxBuffer[0], (uint8_t *)&txBuffer[0], (uint16_t)(g_maxPayloadSize+HEADER_FOOTER_SIZE));
	// Test : Verify if bcopy below copies every bit correctly from big endian to little endian and validate bits
	memmove((uint8_t *)&datatransferRxFooter.dataFrameHeadFoot, &rxBuffer[g_maxPayloadSize], HEADER_FOOTER_SIZE);
	ConvertEndianness(datatransferRxFooter.dataFrameHeadFoot, &bigEndianRxFooter);
	p_dataReceiveInfo->receivedFooter.dataFrameHeadFoot = datatransferRxFooter.dataFrameHeadFoot = bigEndianRxFooter;

//	if (datatransferRxFooter.stVarRxFooterBits.P == (!(GetParity(datatransferRxFooter.dataFrameHeadFoot)))) // This needs to be enabled
	//if (datatransferRxFooter.stVarRxFooterBits.P == ((GetParity(datatransferRxFooter.dataFrameHeadFoot))))
	{
		if (datatransferRxFooter.stVarRxFooterBits.EXST == 1)
		{
			printf("EXST bit set high in ReceiveDataChunk, Footer value is : 0x%08x\n", datatransferRxFooter.dataFrameHeadFoot);
			T1S_ClearStatus();
			//receiveDataStatus = false;  // This isn't required because STATUS0 register cleared
		}
		// ToDo check if received data need to be converted to little endian and then copied into buffer
		if (datatransferRxFooter.stVarRxFooterBits.DV == 1)
		{
			if (datatransferRxFooter.stVarRxFooterBits.HDRB == 1)
			{
				printf("HDRB bit set high, Footer value is : 0x%08x\n", datatransferRxFooter.dataFrameHeadFoot);
				receiveDataStatus = false;
			}

			if (datatransferRxFooter.stVarRxFooterBits.SYNC == 0)
			{
				printf("SYNC failure, Footer value is : 0x%08x\n", datatransferRxFooter.dataFrameHeadFoot);
				//p_dataReceiveInfo->syncFailure = true;
				//receiveDataStatus = false;
				// ToDo: Check for config file
				CheckConfigFileAndConfigMACPHY();
			}
			else
			{
				//p_dataReceiveInfo->syncFailure = false;
			}

			if (datatransferRxFooter.stVarRxFooterBits.SV == 1)
			{
				receiveDataIndex = 0;
				receiveDataStartOffset = (uint8_t)datatransferRxFooter.stVarRxFooterBits.SWO;
				//dataReceptionInProgress = true;
				if (datatransferRxFooter.stVarRxFooterBits.EV != 1)
				{
					(void)memcpy(&receiveMACPHYBuff[receiveDataIndex], &rxBuffer[receiveDataStartOffset], (g_maxPayloadSize-receiveDataStartOffset));
					receiveDataIndex = g_maxPayloadSize - receiveDataStartOffset;
				}
				//printf("I got SV bit set in ReceiveDataChunk, receiveDataIndex = %d, RCA value is %d\n", receiveDataIndex, (uint8_t)datatransferRxFooter.stVarRxFooterBits.RCA);
			}

			if (datatransferRxFooter.stVarRxFooterBits.EV == 1)
			{
				//printf("I got EV bit set in ReceiveDataChunk, RCA value is %d\n", (uint8_t)datatransferRxFooter.stVarRxFooterBits.RCA);
				//receiveDataEndOffset = datatransferRxFooter.stVarRxFooterBits.EBO;
				if (datatransferRxFooter.stVarRxFooterBits.FD == 1)
				{
					printf("Frame Drop bit Set, Footer value is : 0x%08X\n", datatransferRxFooter.dataFrameHeadFoot);
					receiveDataStatus = false;
				}
				else
				{
					stEthernetFrame_t stVarEthernetFrame = {0};
					if ((datatransferRxFooter.stVarRxFooterBits.EBO+1) > g_maxPayloadSize)
					{
						printf("Receive last Data chunk more than mayPayload size\n");
					}
					(void)memcpy(&receiveMACPHYBuff[receiveDataIndex], &rxBuffer[0], (datatransferRxFooter.stVarRxFooterBits.EBO+1));
					receiveDataIndex += (datatransferRxFooter.stVarRxFooterBits.EBO+1);
					if (g_isFCSEnabled)
					{
						uint32_t fcsCalculated = 0;
						uint32_t fcsReceived = 0;
						fcsCalculated = FCS_Calculator(&receiveMACPHYBuff[0], (uint32_t)(receiveDataIndex-SIZE_OF_FCS));
						fcsReceived = receiveMACPHYBuff[(receiveDataIndex-SIZE_OF_FCS) + 3];
						fcsReceived = (uint32_t)(((fcsReceived << 8) | (uint8_t)receiveMACPHYBuff[(receiveDataIndex-SIZE_OF_FCS) + 2]));
						fcsReceived = (uint32_t)(((fcsReceived << 8) | (uint8_t)receiveMACPHYBuff[(receiveDataIndex-SIZE_OF_FCS) + 1]));
						fcsReceived = (uint32_t)((fcsReceived << 8) | (uint8_t)receiveMACPHYBuff[(receiveDataIndex-SIZE_OF_FCS)]);
						if (fcsReceived == fcsCalculated)
						{
							//printf("FCS Received in Data reception was correct, received is 0x%08X and calculated is 0x%08X\n", fcsReceived, fcsCalculated);
						}
						else
						{
							printf("FCS Received in Data reception was wrong, received is 0x%08X and calculated is 0x%08X\n", fcsReceived, fcsCalculated);
						}
					}

					(void)memcpy(&stVarEthernetFrame.destMACAddr[0], &receiveMACPHYBuff[0], (size_t)(sizeof(stEthernetFrame_t)));
					//printf("Ether Type Received is 0x%04X\n", stVarEthernetFrame.ethernetType);
					///< If Ethernet Type is 0x0909 that is Configuration Ethernet frame
					if (stVarEthernetFrame.ethernetType == ETHERNET_TYPE_MACPHYCONFIG)
					{
						//printf("Received EtherType %d Received MAC Dest Address is 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n", stVarEthernetFrame.ethernetType, stVarEthernetFrame.destMACAddr[0], stVarEthernetFrame.destMACAddr[1], stVarEthernetFrame.destMACAddr[2],stVarEthernetFrame.destMACAddr[3],stVarEthernetFrame.destMACAddr[4],stVarEthernetFrame.destMACAddr[5]);
						///< Only if addressed to this device or if broadcast configuration for all nodes
						if ((0 == (memcmp(&stVarEthernetFrame.destMACAddr[0], &g_currentNodeMACAddr[0], SIZE_OF_MAC_ADDR))) || \
								(0 == (memcmp(&stVarEthernetFrame.destMACAddr[0], &g_broadCastMACAddr[0], SIZE_OF_MAC_ADDR))))
						{
							T1S_InterpretEthernetTypeFrame(&receiveMACPHYBuff[0], stVarEthernetFrame);
						}
						else
						{
							printf("Received EtherType 0x%04X but not to our MAC Address, Received MAC Dest Address is 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n", \
							stVarEthernetFrame.ethernetType, stVarEthernetFrame.destMACAddr[0], stVarEthernetFrame.destMACAddr[1], stVarEthernetFrame.destMACAddr[2],\
									stVarEthernetFrame.destMACAddr[3],stVarEthernetFrame.destMACAddr[4],stVarEthernetFrame.destMACAddr[5]);
						}
					}
					//else
					{
						//printf("Wrote into TAP at Receive Data Chunk side, receiveDataIndex = %d, datatransferRxFooter.stVarRxFooterBits.EBO = %d, Num of bytes : %d \n", receiveDataIndex, datatransferRxFooter.stVarRxFooterBits.EBO, (receiveDataIndex + (datatransferRxFooter.stVarRxFooterBits.EBO - 3)));
						uint16_t bytesToRcv = receiveDataIndex;
						if (g_isFCSEnabled)
						{
							bytesToRcv = receiveDataIndex-SIZE_OF_FCS;
						}
						if (TCPIP_Receive(&receiveMACPHYBuff[0], bytesToRcv) == OK) 
						{
							receiveDataIndex = 0;
						}
					}
					//dataReceptionInProgress = false;
					//printf("RCA value after sending data to TAP in Receive Data Chunk is %d\n", (uint8_t)datatransferRxFooter.stVarRxFooterBits.RCA);
				}				
			}
			
			if (((datatransferRxFooter.stVarRxFooterBits.SV != 1) && (datatransferRxFooter.stVarRxFooterBits.EV != 1)) && (receiveDataStatus != false))
			{
				(void)memcpy(&receiveMACPHYBuff[receiveDataIndex], &rxBuffer[0], g_maxPayloadSize);
				receiveDataIndex += g_maxPayloadSize;
				//dataReceptionInProgress = true;
				//printf("SV bit not set and EV not set in ReceiveDataChunk\n");
			}
		
			if (receiveDataStatus == false)
			{
				(void)memset(&receiveMACPHYBuff[0], 0, sizeof(receiveMACPHYBuff));
				receiveDataIndex = 0;
				//dataReceptionInProgress = false;
				printf("Clearing data reception process in ReceiveDataChunk due to Footer: 0x%08x\n", datatransferRxFooter.dataFrameHeadFoot);
			}

			//p_dataReceiveInfo->numOfReceiveChunkAvailable = (uint8_t)datatransferRxFooter.stVarRxFooterBits.RCA;
			//p_dataReceiveInfo->transmitCredits = g_transmitCreditsAvailable_TXC =  (uint8_t)datatransferRxFooter.stVarRxFooterBits.TXC;

			/*if (syncronizingTimestamps && unsyncronizedChunks == 0) {
			  p_dataReceiveInfo->timestamp = timestampFIFO.data[timestampFIFO.head];
			  printf("0: Assigning frame the timestamp: %lld\n", p_dataReceiveInfo->timestamp);
			  if (timestampFIFO.head == timestampFIFO.tail) {
				  perror("Timestamp FIFO Buffer UNDERFLOW!");
			  }
			}
			else {
			  //printf("Assigning frame the timestamp dummy value\n");
			  p_dataReceiveInfo->timestamp = -1;
			}
			*/

			//memcpy(&p_dataReceiveInfo->databuffer[0], &rxBuffer[0+receiveDataStartOffset], receiveDataEndOffset);
			//(void)memcpy(&receiveMACPHYBuff[receiveDataIndex], &rxBuffer[0], MAX_PAYLOAD_BYTE);
		}
	}
	#if 0
	if (datatransferRxFooter.stVarRxFooterBits.P == (!(GetParity(datatransferRxFooter.dataFrameHeadFoot))))
	{
		printf("ReceiveDataChunk footer Parity Passed\n");
	}
	else
	{
		printf("ReceiveDataChunk footer Parity failed\n");
	//	receiveDataStatus = false;
	}
	#endif

	return receiveDataStatus;
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

  bcopy((char *)&valueToConvert, convertedBytes, variableSize);      /** cast and copy an uint32_t to a uint8_t array **/
  position = variableSize - (uint8_t)1;
  for (uint8_t byteIndex = 0; byteIndex < (variableSize/2); byteIndex++)  /** swap bytes in this uint8_t array **/
  {       
      tempVar = (uint8_t)convertedBytes[byteIndex];
      convertedBytes[byteIndex] = convertedBytes[position];
      convertedBytes[position--] = tempVar;
  }
  bcopy(convertedBytes, (uint8_t *)convertedValue, variableSize);      /* copy the swapped convertedBytes to an uint32_t */
}

bool T1S_WriteMACPHYReg(stControlCmdReg_t *p_regData)
{
	uint8_t bufferIndex = 0;
	uint8_t multiplicationFactor = 1;
	uint8_t numberOfbytesToSend = 0;
	uint8_t numberOfRegisterTosend = 0;
	//uint8_t echoedAddOffset = 0;
	bool writeStatus = true;
	const uint8_t igoredEchoedBytes = 4;
	uint8_t txBuffer[MAX_PAYLOAD_BYTE + HEADER_FOOTER_SIZE] = {0};
	uint8_t rxBuffer[MAX_PAYLOAD_BYTE + HEADER_FOOTER_SIZE] = {0};
	uint32_t bigEndianHeader = 0;
	uCommandHeaderFooter_t commandHeader;
	uCommandHeaderFooter_t commandHeaderEchoed;
	
	commandHeader.controlFrameHead = commandHeaderEchoed.controlFrameHead = 0;

	commandHeader.stVarHeadFootBits.DNC = DNC_COMMANDTYPE_CONTROL;
	commandHeader.stVarHeadFootBits.HDRB = 0;
	commandHeader.stVarHeadFootBits.WNR = REG_COMMAND_TYPE_WRITE; // Write into register
	if (p_regData->length != 0) // ToDo : Check if this option is supported and add that check
	{
		commandHeader.stVarHeadFootBits.AID = REG_ADDR_INCREMENT_ENABLE;	// Write register continously from given address
	}
	else
	{
		commandHeader.stVarHeadFootBits.AID = REG_ADDR_INCREMENT_DISABLE;	// Write into same register 
	}
	commandHeader.stVarHeadFootBits.MMS = (uint32_t)(p_regData->memoryMap & 0x0F);
	commandHeader.stVarHeadFootBits.ADDR = (uint32_t)p_regData->address;
	commandHeader.stVarHeadFootBits.LEN = (uint32_t)(p_regData->length & 0x7F);
	commandHeader.stVarHeadFootBits.P = 0;
	commandHeader.stVarHeadFootBits.P = (!GetParity(commandHeader.controlFrameHead));
	
	for (int8_t headerByteCount = 3; headerByteCount >= 0; headerByteCount--)
	{
		txBuffer[bufferIndex++] = commandHeader.controlHeaderArray[headerByteCount];
	}
	
	#if 0
	if () // ToDo Add check if Protected mode in configuration(PROTE in CONFIG0) enabled, so that if protected then number of bytes in payload will be x2
	{
		multiplicationFactor = 2;
	}
	#endif
	
	numberOfRegisterTosend = (uint8_t)(multiplicationFactor * ((uint8_t)commandHeader.stVarHeadFootBits.LEN + (uint8_t)1));
	
	for (uint8_t controlRegIndex = 0; controlRegIndex < numberOfRegisterTosend; controlRegIndex++)
	{
		for (int8_t regByteIndex = 3; regByteIndex >= 0; regByteIndex--)
		{
			txBuffer[bufferIndex++] = (uint8_t)((p_regData->databuffer[controlRegIndex] >> (8 * regByteIndex)) & 0xFF); // Converting into big endian as MACPHY is big endian
		}
		// ToDo if PROTE in CONFIG0 enabled then add 1's complement in between each register data(4bytes)
		#if 0
		for (int8_t regByteIndex = 3; regByteIndex >= 0; regByteIndex--)
		{
			if (PROTE | CONFIG0) // Use correct CONFIG0 byte taken from MACPHY
			{
				txBuffer[bufferIndex++] = (uint8_t) ~((p_regData->databuffer[controlRegIndex] >> (8 * regByteIndex))) & 0xFF); // Converting into big endian as MACPHY is big endian
			}
		}
		#endif
	}
	
	numberOfbytesToSend = (uint8_t)(bufferIndex + HEADER_FOOTER_SIZE); // Added extra 4 bytes because last 4 bytes of payload will be ignored by MACPHY
	
	SPI_Transfer((uint8_t *)&rxBuffer[0], (uint8_t *)&txBuffer[0], numberOfbytesToSend);
	
	//echoedAddOffset = (numberOfbytesToSend - igoredEchoedBytes) - 1; // -1 at last to get correct index value to use(as bufferIndex was incremented at last)
	
	// ToDo Need to check CDPE bit in STATUS0 register is set, which indicates writing into register failed and update writeStatus to false

	memmove((uint8_t *)&commandHeaderEchoed.controlFrameHead, &rxBuffer[igoredEchoedBytes], HEADER_FOOTER_SIZE);
	ConvertEndianness(commandHeaderEchoed.controlFrameHead, &bigEndianHeader);
	commandHeaderEchoed.controlFrameHead = bigEndianHeader;

	#if 0 // Error Handling
	if (commandHeaderEchoed.controlFrameHead != commandHeader.controlFrameHead)
	{
		// ToDo Handle Error when sent and echoed header doesn't match, need to check for respective bits to validate what went wrong
		writeStatus = false;
	}
	else
	{
		if (0 != (memcmp(&rxBuffer[igoredEchoedBytes+HEADER_FOOTER_SIZE], &txBuffer[HEADER_FOOTER_SIZE], (bufferIndex - (igoredEchoedBytes+HEADER_FOOTER_SIZE)))))
		{
			// ToDo Error handling if echoed data doesn't match with transmitted register data(leaving header info and ignored bytes)
			writeStatus = false;
		}
	}
	#endif

	if (commandHeader.stVarHeadFootBits.MMS == 0)
	{
		bool executionStatus;
		stControlCmdReg_t stVarReadRegInfoInput;
		stControlCmdReg_t stVarReadRegData;

		// Reads CONFIG0 register from MMS 0
		stVarReadRegInfoInput.memoryMap = 0;
		stVarReadRegInfoInput.length = 0;
		stVarReadRegInfoInput.address = 0x0004;
		memset(&stVarReadRegInfoInput.databuffer[0], 0, MAX_REG_DATA_ONECHUNK);
		executionStatus = T1S_ReadMACPHYReg(&stVarReadRegInfoInput, &stVarReadRegData);
		if (executionStatus == false)
		{
			// ToDo Action to be taken if reading register fails
			printf("Reading CONFIG0 reg failed after writing (inside WriteReg)\n");
		}
		else
		{
			uint8_t payloadSizeConfiguredValue;
			payloadSizeConfiguredValue = (stVarReadRegData.databuffer[0] & 0x00000007);
			
			switch (payloadSizeConfiguredValue)
			{
				case 3:
					g_maxPayloadSize = 8;
					break;

				case 4:
					g_maxPayloadSize = 16;
					break;

				case 5:
					g_maxPayloadSize = 32;
					break;

				case 6:
				default:
					g_maxPayloadSize = 64;
					break;
			}
			print_message("CONFIG0 reg value is 0x%08x in WriteReg function\n", stVarReadRegData.databuffer[0]);
		}		
	}
	else if (commandHeader.stVarHeadFootBits.MMS == 1)
	{
		CheckIfFCSEnabled();
	}
	else
	{

	}

	return writeStatus;
}

bool T1S_ReadMACPHYReg(stControlCmdReg_t *p_regInfoInput,  stControlCmdReg_t *p_readRegData)
{
	uint8_t bufferIndex = 0;
	const uint8_t igoredEchoedBytes = 4;
	bool readStatus = true;
	int8_t txBuffer[MAX_REG_DATA_ONECONTROLCMD + HEADER_FOOTER_SIZE + EACH_REG_SIZE] = {0};
	int8_t rxBuffer[MAX_REG_DATA_ONECONTROLCMD + HEADER_FOOTER_SIZE + EACH_REG_SIZE] = {0};
	uint16_t numberOfbytesToSend = 0;
	uint32_t bigEndianHeader = 0;
	uCommandHeaderFooter_t commandHeader;
	uCommandHeaderFooter_t commandHeaderEchoed;

	commandHeader.controlFrameHead = commandHeaderEchoed.controlFrameHead = 0;
	
	commandHeader.stVarHeadFootBits.DNC = DNC_COMMANDTYPE_CONTROL;
	commandHeader.stVarHeadFootBits.HDRB = 0;
	commandHeader.stVarHeadFootBits.WNR = REG_COMMAND_TYPE_READ; // Read from register
	if (p_regInfoInput->length != 0)
	{
		commandHeader.stVarHeadFootBits.AID = REG_ADDR_INCREMENT_ENABLE;	// Read register continously from given address
	}
	else
	{
		commandHeader.stVarHeadFootBits.AID = REG_ADDR_INCREMENT_DISABLE;	// Read from same register 
	}
		
	commandHeader.stVarHeadFootBits.MMS = (uint32_t)p_regInfoInput->memoryMap;
	commandHeader.stVarHeadFootBits.ADDR = (uint32_t)p_regInfoInput->address;
	commandHeader.stVarHeadFootBits.LEN = (uint32_t)(p_regInfoInput->length & 0x7F);
	commandHeader.stVarHeadFootBits.P = 0;
	commandHeader.stVarHeadFootBits.P = (!GetParity(commandHeader.controlFrameHead));

	for (int8_t headerByteCount = 3; headerByteCount >= 0; headerByteCount--)
	{
		txBuffer[bufferIndex++] = commandHeader.controlHeaderArray[headerByteCount];
	}

	numberOfbytesToSend = (uint16_t)(bufferIndex + ((commandHeader.stVarHeadFootBits.LEN + 1) * EACH_REG_SIZE) + igoredEchoedBytes); // Added extra 4 bytes because first 4 bytes during reception shall be ignored

	SPI_Transfer((uint8_t *)&rxBuffer[0], (uint8_t *)&txBuffer[0], numberOfbytesToSend);

	memmove((uint8_t *)&commandHeaderEchoed.controlFrameHead, &rxBuffer[igoredEchoedBytes], HEADER_FOOTER_SIZE);
	
	ConvertEndianness(commandHeaderEchoed.controlFrameHead, &bigEndianHeader);
	commandHeaderEchoed.controlFrameHead = bigEndianHeader;

	if (commandHeaderEchoed.stVarHeadFootBits.HDRB != 1) // if MACPHY received header with parity error then it will be 1
	{
		uint32_t endiannessConvertedValue = 0;
		if (commandHeader.stVarHeadFootBits.LEN == 0)
		{
			memmove((uint8_t *)&p_readRegData->databuffer[0], &(rxBuffer[igoredEchoedBytes+HEADER_FOOTER_SIZE]), EACH_REG_SIZE);
			ConvertEndianness(p_readRegData->databuffer[0], &endiannessConvertedValue);	
			p_readRegData->databuffer[0] = endiannessConvertedValue;				 
		}
		else
		{
			for (uint8_t regCount = 0; regCount <= commandHeader.stVarHeadFootBits.LEN; regCount++)
			{
				memmove((uint8_t *)&p_readRegData->databuffer[regCount], &rxBuffer[igoredEchoedBytes+HEADER_FOOTER_SIZE+(EACH_REG_SIZE * regCount)], EACH_REG_SIZE);	
				ConvertEndianness(p_readRegData->databuffer[regCount], &endiannessConvertedValue);	
				p_readRegData->databuffer[regCount] = endiannessConvertedValue;				 
			}
		}
	}
	else
	{
		// ToDo Error handling if MACPHY received with header parity error
		printf("Parity Error READMACPHYReg header value : 0x%08x\n", commandHeaderEchoed.controlFrameHead );
		readStatus = false;
	}

	return readStatus;
}

uint32_t T1S_RegRead(uint8_t MMS, uint16_t Address)
{
	bool executionStatus;
	stControlCmdReg_t stVarReadRegInfoInput;
	stControlCmdReg_t stVarReadRegData;
	stVarReadRegInfoInput.memoryMap = MMS;
	stVarReadRegInfoInput.length = 0;
	stVarReadRegInfoInput.address = Address;

	executionStatus = T1S_ReadMACPHYReg(&stVarReadRegInfoInput, &stVarReadRegData);
	if (executionStatus == true)
	{
		return stVarReadRegData.databuffer[0];
	}
	else
	{
		printf("ERROR: register Read failed to read %d.%4x", MMS, Address);
		return 0;
	}
}

uint32_t T1S_RegWrite(uint8_t MMS, uint16_t Address, uint32_t data)
{
	stControlCmdReg_t stVarWriteRegInput;
	stVarWriteRegInput.memoryMap = MMS;
	stVarWriteRegInput.length = 0;
	stVarWriteRegInput.address = Address;
	stVarWriteRegInput.databuffer[0] = data;
	bool executionStatus = T1S_WriteMACPHYReg(&stVarWriteRegInput);
	if (executionStatus == false)
	{
		// ToDo Action to be taken if reading register fails
		printf("Writing into STATUS0 reg failed while clearing error\n");
		return 0;
	}
	else
	{
		return stVarWriteRegInput.databuffer[0];
	}
}

uint32_t T1S_ClearStatus(void)
{
	bool executionStatus;
	stControlCmdReg_t stVarReadRegInfoInput;
	stControlCmdReg_t stVarReadRegData;

	// Reads Buffer Status register from MMS 0
	stVarReadRegInfoInput.memoryMap = 0;
	stVarReadRegInfoInput.length = 0;
	stVarReadRegInfoInput.address = 0x0008;
	
	executionStatus = T1S_ReadMACPHYReg(&stVarReadRegInfoInput, &stVarReadRegData);
	if (executionStatus == false)
	{
		// ToDo Action to be taken if reading register fails
		printf("Reading STATUS0 register failed\n");
	}
	else
	{
		print_message("STATUS0 reg value before clearing is 0x%08x \r\n", stVarReadRegData.databuffer[0]);
	}

	if (stVarReadRegData.databuffer[0] != 0x00000000) //!< If all values are not at default then set to default
	{
		stControlCmdReg_t stVarWriteRegInput;

		//!< Clear STATUS0 register
		stVarWriteRegInput.memoryMap = 0;
		stVarWriteRegInput.length = 0;
		stVarWriteRegInput.address = 0x0008;
		stVarWriteRegInput.databuffer[0] = 0xFFFFFFFF;
		executionStatus = T1S_WriteMACPHYReg(&stVarWriteRegInput);
		if (executionStatus == false)
		{
			// ToDo Action to be taken if reading register fails
			printf("Writing into STATUS0 reg failed while clearing error\n");
		}
		else
		{
			print_message("STATUS0 reg written with value 0x%08x successfully\n", stVarWriteRegInput.databuffer[0]);
		}
	}

	// Reads Buffer Status register from MMS 0 after clearing	
	executionStatus = T1S_ReadMACPHYReg(&stVarReadRegInfoInput, &stVarReadRegData);
	if (executionStatus == false)
	{
		// ToDo Action to be taken if reading register fails
		printf("Reading STATUS0 register failed\n");
		return UNKNOWN_ERROR;
	}
	else
	{
		print_message("STATUS0 reg value after clearing is 0x%08x \r\n", stVarReadRegData.databuffer[0]);
		return OK;
	}
}

void T1S_InterpretEthernetTypeFrame(uint8_t *receiveMACPHYBuff, stEthernetFrame_t stVarEthernetFrame)
{
	uint8_t numberOfConfigToDo = 0;
	bool isResponseToBeSent = false;
	bool executionStatus;
	uint8_t messageType = 0;
	uint8_t regReadResponse_SendBuffIndex = 0;
	uint8_t regReadResponseBuff[1000] = {0};
	uint16_t receivedDataBuffIndex = 0;

	print_message("\n Received Configuration Ethernet Frame with EtherType 0x%04X and MAC Dest Addr is 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \
			And Src Address is 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n", \
			stVarEthernetFrame.ethernetType, stVarEthernetFrame.destMACAddr[0], stVarEthernetFrame.destMACAddr[1], stVarEthernetFrame.destMACAddr[2],\
			stVarEthernetFrame.destMACAddr[3], stVarEthernetFrame.destMACAddr[4], stVarEthernetFrame.destMACAddr[5], stVarEthernetFrame.srcMACAddr[0],\
				stVarEthernetFrame.srcMACAddr[1], stVarEthernetFrame.srcMACAddr[2], stVarEthernetFrame.srcMACAddr[3], stVarEthernetFrame.srcMACAddr[4],
				stVarEthernetFrame.srcMACAddr[5]);

	receivedDataBuffIndex = (sizeof(stEthernetFrame_t));
	messageType = receiveMACPHYBuff[receivedDataBuffIndex++];
	memcpy(&regReadResponseBuff[0], &stVarEthernetFrame.srcMACAddr[0], SIZE_OF_MAC_ADDR);
	regReadResponse_SendBuffIndex += SIZE_OF_MAC_ADDR;
	memcpy(&regReadResponseBuff[regReadResponse_SendBuffIndex], &g_currentNodeMACAddr[0], SIZE_OF_MAC_ADDR);
	regReadResponse_SendBuffIndex += SIZE_OF_MAC_ADDR;
	memcpy(&regReadResponseBuff[regReadResponse_SendBuffIndex], &stVarEthernetFrame.ethernetType, sizeof(stVarEthernetFrame.ethernetType));
	regReadResponse_SendBuffIndex += sizeof(stVarEthernetFrame.ethernetType);
	numberOfConfigToDo = receiveMACPHYBuff[receivedDataBuffIndex++];
	/*for (uint16_t receiveIndexCount = 0; receiveIndexCount < receiveDataIndex; receiveIndexCount++)
	{
		print_message("0x%02X ", receiveMACPHYBuff[receiveIndexCount]);
	}
	print_message("\nEnd of Received Bytes in EtherType 0909\n");*/
	//print_message("Received Ethernet frame, need to verify if Topo Disc volt measurement\n");

	if (messageType == MESSAGETYPE_SEND)
	{
		regReadResponseBuff[regReadResponse_SendBuffIndex++] = MESSAGETYPE_RESPONSE;
		regReadResponseBuff[regReadResponse_SendBuffIndex++] = numberOfConfigToDo;
		for (uint8_t configCount = 0; configCount < numberOfConfigToDo; configCount++)
		{
			stControlCmdReg_t stVarReadRegInfoInput;
			stControlCmdReg_t stVarReadRegData;
			stControlCmdReg_t stVarWriteRegInput;
			uint8_t typeOfConfig = receiveMACPHYBuff[receivedDataBuffIndex++];
			regReadResponseBuff[regReadResponse_SendBuffIndex++] = typeOfConfig;
			
			if (typeOfConfig == CONFIGTYPE_REGREAD)
			{				
			//	printf("Received Register read command from other Ethernet node \n");						
				isResponseToBeSent = true;
				stVarReadRegInfoInput.memoryMap = receiveMACPHYBuff[receivedDataBuffIndex++];
				stVarReadRegInfoInput.length = 0;
				stVarReadRegInfoInput.address =  ((receiveMACPHYBuff[receivedDataBuffIndex+1] << 8) | receiveMACPHYBuff[receivedDataBuffIndex]);//(uint16_t)((receiveMACPHYBuff[receivedDataBuffIndex++] << 8) | regAddr);
				receivedDataBuffIndex += sizeof(stVarReadRegInfoInput.address);
				memset(&stVarReadRegInfoInput.databuffer[0], 0, MAX_REG_DATA_ONECHUNK);
				executionStatus = T1S_ReadMACPHYReg(&stVarReadRegInfoInput, &stVarReadRegData);
				if (executionStatus == false)
				{
					printf("Reading address 0x%04X of MMS %d in Node Register read failed\n", stVarReadRegInfoInput.address, stVarReadRegInfoInput.memoryMap);
				}
				else
				{
					printf("Reg value of addr 0x%04X of MMS %d is 0x%08X\n", stVarReadRegInfoInput.address, stVarReadRegInfoInput.memoryMap, stVarReadRegData.databuffer[0]);
				}
				regReadResponseBuff[regReadResponse_SendBuffIndex++] = stVarReadRegInfoInput.memoryMap;
				regReadResponseBuff[regReadResponse_SendBuffIndex++] = (uint8_t)stVarReadRegInfoInput.address;
				regReadResponseBuff[regReadResponse_SendBuffIndex++] = (uint8_t)((stVarReadRegInfoInput.address >> 8) & (0xFF));
				memcpy(&regReadResponseBuff[regReadResponse_SendBuffIndex], &stVarReadRegData.databuffer[0], EACH_REG_SIZE);
				regReadResponse_SendBuffIndex += EACH_REG_SIZE;
			}
			else if (typeOfConfig == CONFIGTYPE_REGWRITE)
			{
				uint32_t regWriteValue = 0;
				//printf("Received Register write command from other Ethernet node \n");
				isResponseToBeSent = true;
				stVarWriteRegInput.memoryMap = receiveMACPHYBuff[receivedDataBuffIndex++];
				stVarWriteRegInput.length = 0;

				stVarWriteRegInput.address =  ((receiveMACPHYBuff[receivedDataBuffIndex+1] << 8) | receiveMACPHYBuff[receivedDataBuffIndex]);
				receivedDataBuffIndex += sizeof(stVarWriteRegInput.address);
				regWriteValue = (uint32_t)((receiveMACPHYBuff[receivedDataBuffIndex+3] << 24) | (receiveMACPHYBuff[receivedDataBuffIndex+2] << 16)\
									| (receiveMACPHYBuff[receivedDataBuffIndex+1] << 8) | receiveMACPHYBuff[receivedDataBuffIndex]);
				receivedDataBuffIndex += EACH_REG_SIZE;

				stVarWriteRegInput.databuffer[0] = regWriteValue;
				executionStatus = T1S_WriteMACPHYReg(&stVarWriteRegInput); if (executionStatus == false)
				{
					printf("Writing into reg failed during Node Configuration\n");
				}
				else
				{
					print_message("Register 0x%04X was written successfully with value 0x%08X through Node Configuration\n", stVarWriteRegInput.address, stVarWriteRegInput.databuffer[0]);
				}

				regReadResponseBuff[regReadResponse_SendBuffIndex++] = stVarWriteRegInput.memoryMap;
				regReadResponseBuff[regReadResponse_SendBuffIndex++] = (uint8_t)stVarWriteRegInput.address;
				regReadResponseBuff[regReadResponse_SendBuffIndex++] = (uint8_t)((stVarWriteRegInput.address >> 8) & (0xFF));
			}
			else
			{
				isResponseToBeSent = false;
				printf("Invalid Type of config value received, received value is %d\n", typeOfConfig);
			}
		}
	}
	else if (messageType == MESSAGETYPE_RESPONSE)
	{
		isResponseToBeSent = false;
		for (uint8_t configCount = 0; configCount < numberOfConfigToDo; configCount++)
		{
			uint8_t mms;
			uint16_t regAddr;
			uint32_t regValue;
			uint8_t typeOfConfig = receiveMACPHYBuff[receivedDataBuffIndex++];

			if (typeOfConfig == CONFIGTYPE_REGREAD)
			{
				mms = receiveMACPHYBuff[receivedDataBuffIndex++];
				regAddr	= (uint16_t)((receiveMACPHYBuff[receivedDataBuffIndex+1] << 8) | receiveMACPHYBuff[receivedDataBuffIndex]);
				receivedDataBuffIndex += sizeof(regAddr);
				regValue = (uint32_t)((receiveMACPHYBuff[receivedDataBuffIndex+3] << 24) | (receiveMACPHYBuff[receivedDataBuffIndex+2] << 16)\
									| (receiveMACPHYBuff[receivedDataBuffIndex+1] << 8) | receiveMACPHYBuff[receivedDataBuffIndex]);
				receivedDataBuffIndex += EACH_REG_SIZE;
				print_message("Reg value requested through Node Configuration for reg addr 0x%04X of MMS %d is 0x%08X\n", regAddr, mms, regValue);
			}
			else if (typeOfConfig == CONFIGTYPE_REGWRITE)
			{
				mms = receiveMACPHYBuff[receivedDataBuffIndex++];
				regAddr	= (uint16_t)((receiveMACPHYBuff[receivedDataBuffIndex+1] << 8) | receiveMACPHYBuff[receivedDataBuffIndex]);
				receivedDataBuffIndex += sizeof(regAddr);

				print_message("Reg 0x%04X written through Node Configuration for MMS %d was successful\n", regAddr, mms);
			}
			else
			{
				printf("Invalid Type of configuration sent through Node configuration, Type is %d\n", typeOfConfig);
			}
		}
	}
	else
	{
		isResponseToBeSent = false;
		printf("Invalid Message Type, received value is %d", messageType);
	}

	if ((regReadResponse_SendBuffIndex > ETHERNET_HEADER_SIZE) && (isResponseToBeSent == true))
	{
		if ((T1S_Transmit(&regReadResponseBuff[0], regReadResponse_SendBuffIndex)) != OK)
		{
			printf("Sending data failed during Node Configuration response\n");
		}
		else
		{
			print_message("Sending bulk data was successful during Node Configuration response, num of sent bytes: %d\n", regReadResponse_SendBuffIndex);
		}
	}
}

uint32_t T1S_Transmit(uint8_t* p_tapDataBuffer, uint16_t num_bytes_to_transmit)
{
	uint8_t txBuffer[2048] = {0}; //! Total required chunks is 31(31 x 64) but kept one extra chunk bytes added extra to buffer so total 32x64
	uint8_t rxBuffer[2048] = {0};

	uint16_t sentChunkCount = 0;
	uint16_t numberOfChunksToSend = 0;
	uint16_t numberOfBytesToSend = 0;
	uint16_t bytesToTransmit = num_bytes_to_transmit;
	uint32_t bigEndianRxFooter = 0;
	static uDataHeaderFooter_t dataTransferHeader;
	uDataHeaderFooter_t datatransferRxFooter;

	if (g_isFCSEnabled)
	{
		uint32_t fcsCalculated = 0;

		if (bytesToTransmit < (MAX_PAYLOAD_BYTE - HEADER_FOOTER_SIZE))
		{
			uint8_t frameCountDiff = (MAX_PAYLOAD_BYTE - HEADER_FOOTER_SIZE) - bytesToTransmit;
			memset(&p_tapDataBuffer[bytesToTransmit], 0x00, frameCountDiff);
			bytesToTransmit += frameCountDiff;
		}
		fcsCalculated = FCS_Calculator(&p_tapDataBuffer[0], (uint32_t)bytesToTransmit);
		p_tapDataBuffer[bytesToTransmit] = (uint8_t)fcsCalculated;
		p_tapDataBuffer[bytesToTransmit + 1] = (uint8_t)(fcsCalculated >> 8);
		p_tapDataBuffer[bytesToTransmit + 2] = (uint8_t)(fcsCalculated >> 16);
		p_tapDataBuffer[bytesToTransmit + 3] = (uint8_t)(fcsCalculated >> 24);
		bytesToTransmit += SIZE_OF_FCS;
	}

	//ToDo Handle FCS and frame padding here by checking TXFCSVC bit in STDCAP register
	numberOfChunksToSend  = (bytesToTransmit/g_maxPayloadSize);
	if ((bytesToTransmit % g_maxPayloadSize) > 0)
	{
		numberOfChunksToSend++;
	}
	//printf("Number of bytes to send %d, Number of chunks %d\n", bytesToTransmit, numberOfChunksToSend); 

	for (sentChunkCount = 0; sentChunkCount < numberOfChunksToSend; sentChunkCount++)
	{
		uint8_t bufferIndex = 0;
		dataTransferHeader.stVarTxHeadBits.DNC = DNC_COMMANDTYPE_DATA;
		//if (SEQE | CONFIG0) //ToDo Add check here to verify if SEQ check if enabled in config register
		{
			dataTransferHeader.stVarTxHeadBits.SEQ = (uint8_t)(~dataTransferHeader.stVarTxHeadBits.SEQ);
		}
		//else
		{
			dataTransferHeader.stVarTxHeadBits.SEQ = 0;
		}

		dataTransferHeader.stVarTxHeadBits.NORX = 0;
		dataTransferHeader.stVarTxHeadBits.RSVD1 = 0;
		dataTransferHeader.stVarTxHeadBits.VS = 0;
		dataTransferHeader.stVarTxHeadBits.DV = 1;
		dataTransferHeader.stVarTxHeadBits.RSVD2 = 0;
		dataTransferHeader.stVarTxHeadBits.TSC = 0;
		dataTransferHeader.stVarTxHeadBits.RSVD3 = 0;
		dataTransferHeader.stVarTxHeadBits.P = 0;

		if (sentChunkCount == 0)
		{
			dataTransferHeader.stVarTxHeadBits.SV = 1;
			dataTransferHeader.stVarTxHeadBits.SWO = 0;
		}
		else
		{
			dataTransferHeader.stVarTxHeadBits.SV = 0;
		}

		if (sentChunkCount == (numberOfChunksToSend-1))
		{
			dataTransferHeader.stVarTxHeadBits.EV = 1;
			if ((bytesToTransmit % g_maxPayloadSize) > 0)
			{
				dataTransferHeader.stVarTxHeadBits.EBO = ((bytesToTransmit % g_maxPayloadSize) - 1);
			}
			else
			{
				dataTransferHeader.stVarTxHeadBits.EBO = (g_maxPayloadSize-1);
			}
		}
		else
		{
			dataTransferHeader.stVarTxHeadBits.EV = 0;
			dataTransferHeader.stVarTxHeadBits.EBO = 0;
		}
		//printf("EBO is BulkTransmit is %d \n", dataTransferHeader.stVarTxHeadBits.EBO);
		dataTransferHeader.stVarTxHeadBits.P = (!GetParity(dataTransferHeader.dataFrameHeadFoot));
		
		for (int8_t headerByteCount = 3; headerByteCount >= 0; headerByteCount--)
		{
			txBuffer[(sentChunkCount*(g_maxPayloadSize + HEADER_FOOTER_SIZE)) + bufferIndex] = dataTransferHeader.dataFrameHeaderBuffer[headerByteCount];
			bufferIndex++;
		}

		numberOfBytesToSend += bufferIndex;
		memcpy(&txBuffer[(sentChunkCount*(g_maxPayloadSize + HEADER_FOOTER_SIZE)) + bufferIndex], &p_tapDataBuffer[(sentChunkCount*g_maxPayloadSize)], g_maxPayloadSize);
		numberOfBytesToSend += g_maxPayloadSize;
		//printf("Sending: Chunk Count %d, number of Bytes added to buff %d, Header 0x%08X \n", sentChunkCount, numberOfBytesToSend, dataTransferHeader.dataFrameHeadFoot);

		if (sentChunkCount == (numberOfChunksToSend-1))
		{
			SPI_Transfer((uint8_t *)&rxBuffer[0], (uint8_t *)&txBuffer[0], (uint16_t)numberOfBytesToSend);
			printf("\nStart of Transmit Bytes in Bulk Data transfer \n");
			for (uint16_t txIndexCount = 0; txIndexCount < numberOfBytesToSend; txIndexCount++)
			{
				printf("0x%02X ", txBuffer[txIndexCount]);
			}
			printf("\nEnd of Transmit Bytes in Bulk Data transfer \n");
			
		}
	}
	
	for (sentChunkCount = 0; sentChunkCount < numberOfChunksToSend; sentChunkCount++)
	{
		bool dataTransmissionStatus = true;
		uint16_t rxBufferIndex = (sentChunkCount * (g_maxPayloadSize+HEADER_FOOTER_SIZE));

		memmove((uint8_t *)&datatransferRxFooter.dataFrameHeadFoot, &(rxBuffer[rxBufferIndex + g_maxPayloadSize]), HEADER_FOOTER_SIZE);
		ConvertEndianness(datatransferRxFooter.dataFrameHeadFoot, &bigEndianRxFooter);
		datatransferRxFooter.dataFrameHeadFoot = bigEndianRxFooter;
		//print_message("Reception: Chunk Count %d, Footer 0x%08X \n", sentChunkCount, datatransferRxFooter.dataFrameHeadFoot);
		if (dataTransferHeader.stVarTxHeadBits.NORX == 0)
		{
			//ToDo Calculate received footer parity first and then go for next bits validation
			if (datatransferRxFooter.stVarRxFooterBits.EXST == 1)
			{
				// ToDo Error Handling based on checking which bits are set in STATUS registers
				dataTransmissionStatus = false;
				T1S_ClearStatus();
				printf("EXST Bit set in TransmitData, Footer: 0x%08x\n", datatransferRxFooter.dataFrameHeadFoot);
			}

			if (datatransferRxFooter.stVarRxFooterBits.SYNC == 0)
			{
				// ToDo : SYNC bit indicates MACPHY configuration is not same as SPI so need to update caller of this function to configure MACPHY
				printf("SYNC failure occured in TransmitData, Footer: 0x%08x\n", datatransferRxFooter.dataFrameHeadFoot);
				//ConfigureMACHPHY();
				CheckConfigFileAndConfigMACPHY();
				dataTransmissionStatus = false;
			}

			if (datatransferRxFooter.stVarRxFooterBits.HDRB == 1)
			{
				printf("Header Bad in TransmitData, Footer: 0x%08x\n", datatransferRxFooter.dataFrameHeadFoot);
				dataTransmissionStatus = false;
			}

			if (datatransferRxFooter.stVarRxFooterBits.DV == 1)
			{
				uint8_t bufferIndexStartOffset = 0;
				// This indicates if received bytes are valid and need to update User about this data
				// ToDo : Check how to handle this data and do what with this
				if (datatransferRxFooter.stVarRxFooterBits.SV == 1)
				{
					//printf("I got SV bit set in TransmitDataChunk, RCA value is %d\n", (uint8_t)datatransferRxFooter.stVarRxFooterBits.RCA);
					if (datatransferRxFooter.stVarRxFooterBits.SWO > 0)
					{
						bufferIndexStartOffset = (uint8_t)datatransferRxFooter.stVarRxFooterBits.SWO;
					}
					//dataReceptionInProgress = true;
					receiveDataIndex = 0;
					(void)memcpy(&receiveMACPHYBuff[receiveDataIndex], &rxBuffer[rxBufferIndex+bufferIndexStartOffset], (g_maxPayloadSize - bufferIndexStartOffset));
					receiveDataIndex = g_maxPayloadSize - bufferIndexStartOffset;
				}

				if (datatransferRxFooter.stVarRxFooterBits.EV == 1)
				{
					//printf("I got EV bit set in TransmitDataChunk\n");
					if (datatransferRxFooter.stVarRxFooterBits.FD == 1)
					{
						// ToDo Frame Drop bit set meaning, current receiving frame is not valid so need to read again - happens only in cut through mode
						printf("Frame Drop in TransmitData, Footer: 0x%08x\n", datatransferRxFooter.dataFrameHeadFoot);
						dataTransmissionStatus = false;
					}
					else
					{
						(void)memcpy(&receiveMACPHYBuff[receiveDataIndex], &rxBuffer[rxBufferIndex], (datatransferRxFooter.stVarRxFooterBits.EBO+1)); // +1 here as checksum is 4 bytes and so -3 is made whiles sending data into TAP
						receiveDataIndex += (datatransferRxFooter.stVarRxFooterBits.EBO+1);
						if (g_isFCSEnabled)
						{
							uint32_t fcsCalculated = 0;
							uint32_t fcsReceived = 0;
							fcsCalculated = FCS_Calculator(&receiveMACPHYBuff[0], (uint32_t)(receiveDataIndex-SIZE_OF_FCS));
							fcsReceived = receiveMACPHYBuff[(receiveDataIndex-SIZE_OF_FCS) + 3];
							fcsReceived = (uint32_t)(((fcsReceived << 8) | (uint8_t)receiveMACPHYBuff[(receiveDataIndex-SIZE_OF_FCS) + 2]));
							fcsReceived = (uint32_t)(((fcsReceived << 8) | (uint8_t)receiveMACPHYBuff[(receiveDataIndex-SIZE_OF_FCS) + 1]));
							fcsReceived = (uint32_t)((fcsReceived << 8) | (uint8_t)receiveMACPHYBuff[(receiveDataIndex-SIZE_OF_FCS)]);
							if (fcsReceived == fcsCalculated)
							{
								//print_message("FCS Received in Bulk data transmission was correct, received is 0x%08X and calculated is 0x%08X\n", fcsReceived, fcsCalculated);
							}
							else
							{
								printf("FCS Received in Bulk data transmission was wrong, received is 0x%08X and calculated is 0x%08X\n", fcsReceived, fcsCalculated);
							}
						}
						//printf("Wrote into TAP at Transmit Data Chunk side, Num of bytes : %d \n", (receiveDataIndex + (datatransferRxFooter.stVarRxFooterBits.EBO - 3)));
						if (TCPIP_Receive(&receiveMACPHYBuff[0], receiveDataIndex) == OK) 
						{
							receiveDataIndex = 0;
						}
						//printf("RCA value after sending data to TAP in TransmitDataChunk is %d\n", (uint8_t)datatransferRxFooter.stVarRxFooterBits.RCA);
					}
				}
				
				if (((datatransferRxFooter.stVarRxFooterBits.SV != 1) && (datatransferRxFooter.stVarRxFooterBits.EV != 1)) && (dataTransmissionStatus != false))
				{
					(void)memcpy(&receiveMACPHYBuff[receiveDataIndex], &rxBuffer[rxBufferIndex], g_maxPayloadSize);
					//printf("SV bit not set and EV not set in TransmitDataChunk, Chunk count %d \n", sentChunkCount);
					receiveDataIndex += g_maxPayloadSize;
				}
			}
			else
			{
				//printf("Received data is not valid, chunk count %d\n", sentChunkCount);
			}			

			if (dataTransmissionStatus == false)
			{
				(void)memset(&receiveMACPHYBuff[0], 0, sizeof(receiveMACPHYBuff));
				receiveDataIndex = 0;
				printf("Clearing data reception process in TransmitDataChunk due to Footer: 0x%08x where sentChunkCount of Tx is %d\n", datatransferRxFooter.dataFrameHeadFoot, sentChunkCount);
				return UNKNOWN_ERROR;
			}
			//p_bulkDataTx->stVarEachChunkTransfer.receiveChunkAvailable = receiveChunksAvailable_RCA = (uint8_t)datatransferRxFooter.stVarRxFooterBits.RCA;
			//p_bulkDataTx->stVarEachChunkTransfer.transmitCredits = (uint8_t)datatransferRxFooter.stVarRxFooterBits.TXC;
		}
	}
	/*
	if (syncronizingTimestamps && executionStatus) {
		//printf("Poping Queue after send\n");
		if (timestampFIFO.head == timestampFIFO.tail) {
			//perror("Timestamp FIFO Buffer UNDERFLOW!");
		}
		timestampFIFO.head = (timestampFIFO.head + 1) % timestampFIFO.length;
	}
	*/

	return OK;
}

uint32_t T1S_Receive(stDataReceive_t* p_dataReceiveInfo, uint16_t num_chunks_to_collect)
{
	uint32_t errorCode = OK;
	uint8_t txBuffer[2048] = {0}; //! Total required chunks is 31(31 x 64) but kept one extra chunk bytes added extra to buffer so total 32x64
	uint8_t rxBuffer[2048] = {0};
	uint8_t bufferIndex = 0;
	uint8_t receiveDataStartOffset = 0;
	uint8_t numberOfChunksToReceive = 0;
	uint16_t numberOfBytesToReceive = 0;
	uint32_t bigEndianRxFooter = 0;
	static uDataHeaderFooter_t dataTransferHeader;
	uDataHeaderFooter_t datatransferRxFooter;

	dataTransferHeader.stVarTxHeadBits.DNC = DNC_COMMANDTYPE_DATA;
	//if (SEQE | CONFIG0) //ToDo Add check here to verify if SEQ check if enabled in config register
	{
		dataTransferHeader.stVarTxHeadBits.SEQ = ~dataTransferHeader.stVarTxHeadBits.SEQ;
	}
	//else
	{
		dataTransferHeader.stVarTxHeadBits.SEQ = 0;
	}

	dataTransferHeader.stVarTxHeadBits.NORX = 0;
	dataTransferHeader.stVarTxHeadBits.VS = 0;
	dataTransferHeader.stVarTxHeadBits.RSVD1 = 0;
	dataTransferHeader.stVarTxHeadBits.DV = 0;
	dataTransferHeader.stVarTxHeadBits.SV = 0;
	dataTransferHeader.stVarTxHeadBits.SWO = 0;
	dataTransferHeader.stVarTxHeadBits.RSVD2 = 0;
	dataTransferHeader.stVarTxHeadBits.EV = 0;
	dataTransferHeader.stVarTxHeadBits.EBO = 0;
	dataTransferHeader.stVarTxHeadBits.RSVD3 = 0;
	dataTransferHeader.stVarTxHeadBits.TSC = 0;
	dataTransferHeader.stVarTxHeadBits.P = 0;
	dataTransferHeader.stVarTxHeadBits.P = (!GetParity(dataTransferHeader.dataFrameHeadFoot));

	//ToDo Handle FCS and frame padding here by checking TXFCSVC bit in STDCAP register
	if (num_chunks_to_collect == 0xFF)
	{
		numberOfChunksToReceive = 1;
	}
	else
	{
		numberOfChunksToReceive = num_chunks_to_collect;
	}

	for (uint8_t headerForChunks = 0; headerForChunks < numberOfChunksToReceive; headerForChunks++)
	{
		bufferIndex = 0;
		for (int8_t headerByteCount = 3; headerByteCount >= 0; headerByteCount--)
		{
			txBuffer[(headerForChunks * (g_maxPayloadSize+HEADER_FOOTER_SIZE)) + bufferIndex] = dataTransferHeader.dataFrameHeaderBuffer[headerByteCount];
			bufferIndex++;
		}
	}

	numberOfBytesToReceive = numberOfChunksToReceive * (g_maxPayloadSize+HEADER_FOOTER_SIZE);

	SPI_Transfer((uint8_t *)&rxBuffer[0], (uint8_t *)&txBuffer[0], numberOfBytesToReceive);

	for (uint8_t headerForChunks = 0; headerForChunks < numberOfChunksToReceive; headerForChunks++)
	{
		uint16_t rxBufferIndexChunk = (headerForChunks * (g_maxPayloadSize+HEADER_FOOTER_SIZE));
		memmove((uint8_t *)&datatransferRxFooter.dataFrameHeadFoot, &rxBuffer[(rxBufferIndexChunk + g_maxPayloadSize)], HEADER_FOOTER_SIZE);
		ConvertEndianness(datatransferRxFooter.dataFrameHeadFoot, &bigEndianRxFooter);
		p_dataReceiveInfo->receivedFooter.dataFrameHeadFoot = datatransferRxFooter.dataFrameHeadFoot = bigEndianRxFooter;
		//if (datatransferRxFooter.stVarRxFooterBits.P == (!(GetParity(datatransferRxFooter.dataFrameHeadFoot)))) // This needs to be enabled
		//if (datatransferRxFooter.stVarRxFooterBits.P == ((GetParity(datatransferRxFooter.dataFrameHeadFoot))))
		{
			if (datatransferRxFooter.stVarRxFooterBits.EXST == 1)
			{
				printf("EXST bit set high in ReceiveDataChunk of %d chunk count, Footer value is : 0x%08x\n", headerForChunks, datatransferRxFooter.dataFrameHeadFoot);
				T1S_ClearStatus();
				//receiveDataStatus = false;  // This isn't required because STATUS0 register cleared
			}
		    	
			if (datatransferRxFooter.stVarRxFooterBits.DV == 1)
			{
				//printf("ReceiveDataChunk Footer value is : 0x%08x\n", datatransferRxFooter.dataFrameHeadFoot);
				if (datatransferRxFooter.stVarRxFooterBits.HDRB == 1)
				{
					printf("HDRB bit set high in %d chunk count, Footer value is : 0x%08x\n", headerForChunks, datatransferRxFooter.dataFrameHeadFoot);
					errorCode = UNKNOWN_ERROR;
				}

				if (datatransferRxFooter.stVarRxFooterBits.SYNC == 0)
				{
					printf("SYNC failure in %d chunk count, Footer value is : 0x%08x\n", headerForChunks, datatransferRxFooter.dataFrameHeadFoot);
					//p_dataReceiveInfo->syncFailure = true;
					CheckConfigFileAndConfigMACPHY();
					// ToDo: Check if function has to return from here
				}
				else
				{
					//p_dataReceiveInfo->syncFailure = false;
				}

				if (datatransferRxFooter.stVarRxFooterBits.SV == 1)
				{
					receiveDataIndex = 0;
					receiveDataStartOffset = (uint8_t)datatransferRxFooter.stVarRxFooterBits.SWO;
					if (datatransferRxFooter.stVarRxFooterBits.EV != 1)
					{
						(void)memcpy(&receiveMACPHYBuff[receiveDataIndex], &rxBuffer[(rxBufferIndexChunk + receiveDataStartOffset)], (g_maxPayloadSize-receiveDataStartOffset));
						receiveDataIndex = g_maxPayloadSize - receiveDataStartOffset;
					}
					//printf("I got SV bit set in ReceiveDataChunk, receiveDataIndex = %d, RCA value is %d\n", receiveDataIndex, (uint8_t)datatransferRxFooter.stVarRxFooterBits.RCA);
				}

				if (datatransferRxFooter.stVarRxFooterBits.EV == 1)
				{
					//printf("I got EV bit set in ReceiveDataChunk, RCA value is %d\n", (uint8_t)datatransferRxFooter.stVarRxFooterBits.RCA);
					//receiveDataEndOffset = datatransferRxFooter.stVarRxFooterBits.EBO;
					if (datatransferRxFooter.stVarRxFooterBits.FD == 1)
					{
						printf("Frame Drop bit Set in %d chunk count, Footer value is : 0x%08x\n", headerForChunks, datatransferRxFooter.dataFrameHeadFoot);
						errorCode = UNKNOWN_ERROR;
					}
					else
					{
						stEthernetFrame_t stVarEthernetFrame = {0};
						(void)memcpy(&receiveMACPHYBuff[receiveDataIndex], &rxBuffer[rxBufferIndexChunk], (datatransferRxFooter.stVarRxFooterBits.EBO+1));
						receiveDataIndex += (datatransferRxFooter.stVarRxFooterBits.EBO+1);
						(void)memcpy(&stVarEthernetFrame.destMACAddr[0], &receiveMACPHYBuff[0], (size_t)(sizeof(stEthernetFrame_t)));
						//printf("EBO is BulkReception is %d and Receive Data Index is %d \n", datatransferRxFooter.stVarRxFooterBits.EBO+1, receiveDataIndex);
						if (g_isFCSEnabled)
						{
							uint32_t fcsCalculated = 0;
							uint32_t fcsReceived = 0;
							fcsCalculated = FCS_Calculator(&receiveMACPHYBuff[0], (uint32_t)(receiveDataIndex-SIZE_OF_FCS));
							fcsReceived = receiveMACPHYBuff[(receiveDataIndex-SIZE_OF_FCS) + 3];
							fcsReceived = (uint32_t)(((fcsReceived << 8) | (uint8_t)receiveMACPHYBuff[(receiveDataIndex-SIZE_OF_FCS) + 2]));
							fcsReceived = (uint32_t)(((fcsReceived << 8) | (uint8_t)receiveMACPHYBuff[(receiveDataIndex-SIZE_OF_FCS) + 1]));
							fcsReceived = (uint32_t)((fcsReceived << 8) | (uint8_t)receiveMACPHYBuff[(receiveDataIndex-SIZE_OF_FCS)]);
							if (fcsReceived == fcsCalculated)
							{
								//print_message("FCS Received in Bulk data reception was correct, received is 0x%08X and calculated is 0x%08X\n", fcsReceived, fcsCalculated);
							}
							else
							{
								printf("FCS Received in Bulk data reception was wrong, received is 0x%08X and calculated is 0x%08X\n", fcsReceived, fcsCalculated);
							}
						}
						//printf("Ether Type Received is 0x%04X\n", stVarEthernetFrame.ethernetType);
						///< If Ethernet Type is 0x0909 that is Configuration Ethernet frame
						if (stVarEthernetFrame.ethernetType == ETHERNET_TYPE_MACPHYCONFIG)
						{
							//printf("Received EtherType 0x%04X Received MAC Dest Address is 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n", stVarEthernetFrame.ethernetType, stVarEthernetFrame.destMACAddr[0], stVarEthernetFrame.destMACAddr[1], stVarEthernetFrame.destMACAddr[2],stVarEthernetFrame.destMACAddr[3],stVarEthernetFrame.destMACAddr[4],stVarEthernetFrame.destMACAddr[5]);
							///< Only if addressed to this device or if broadcast configuration for all nodes
							if ((0 == (memcmp(&stVarEthernetFrame.destMACAddr[0], &g_currentNodeMACAddr[0], SIZE_OF_MAC_ADDR))) || \
									(0 == (memcmp(&stVarEthernetFrame.destMACAddr[0], &g_broadCastMACAddr[0], SIZE_OF_MAC_ADDR))))
							{
								T1S_InterpretEthernetTypeFrame(&receiveMACPHYBuff[0], stVarEthernetFrame);
							}
							else
							{
								printf("Received EtherType %d but not to our MAC Address, Received MAC Dest Address is 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n", \
								stVarEthernetFrame.ethernetType, stVarEthernetFrame.destMACAddr[0], stVarEthernetFrame.destMACAddr[1], stVarEthernetFrame.destMACAddr[2],\
										stVarEthernetFrame.destMACAddr[3],stVarEthernetFrame.destMACAddr[4],stVarEthernetFrame.destMACAddr[5]);
							}
						}
						//else
						{
							uint16_t bytesToSendOverTAP = receiveDataIndex;
							if (g_isFCSEnabled)
							{
								bytesToSendOverTAP = (receiveDataIndex-SIZE_OF_FCS);
							}
							
							if (TCPIP_Receive(&receiveMACPHYBuff[0], bytesToSendOverTAP) == OK) 
							{
								receiveDataIndex = 0;
							}
							//printf("Wrote into TAP at Receive Data Chunk side, receiveDataIndex = %d, datatransferRxFooter.stVarRxFooterBits.EBO = %d, Num of bytes : %d \n", receiveDataIndex, datatransferRxFooter.stVarRxFooterBits.EBO, (bytesToSendOverTAP + (datatransferRxFooter.stVarRxFooterBits.EBO - 3)));
							
							//printf("RCA value after sending data to TAP in Receive Data Chunk is %d\n", (uint8_t)datatransferRxFooter.stVarRxFooterBits.RCA);
						}			
					}	
				}
				
				if (((datatransferRxFooter.stVarRxFooterBits.SV != 1) && (datatransferRxFooter.stVarRxFooterBits.EV != 1)) && (errorCode == OK))
				{
					(void)memcpy(&receiveMACPHYBuff[receiveDataIndex], &rxBuffer[rxBufferIndexChunk], g_maxPayloadSize);
					receiveDataIndex += g_maxPayloadSize;
					//printf("SV bit not set and EV not set in ReceiveDataChunk\n");
				}
			
				if (errorCode != OK)
				{
					(void)memset(&receiveMACPHYBuff[0], 0, sizeof(receiveMACPHYBuff));
					receiveDataIndex = 0;
					printf("Clearing data reception process in ReceiveDataChunk of %d chunk count due to Footer: 0x%08x\n", headerForChunks, datatransferRxFooter.dataFrameHeadFoot);
				}

				//p_dataReceiveInfo->numOfReceiveChunkAvailable = (uint8_t)datatransferRxFooter.stVarRxFooterBits.RCA;
				//p_dataReceiveInfo->transmitCredits = g_transmitCreditsAvailable_TXC =  (uint8_t)datatransferRxFooter.stVarRxFooterBits.TXC;
			}
		}
	}
	/*if (syncronizingTimestamps && unsyncronizedChunks == 0) 
	{
		p_dataReceiveInfo->timestamp = timestampFIFO.data[timestampFIFO.head];
		if ((p_dataReceiveInfo->timestamp - lastTimestamp) != 0) 
		{
			printf("1: Assigning frame the timestamp with delta: %lld\n", (p_dataReceiveInfo->timestamp - lastTimestamp)/100);
		}
		lastTimestamp = p_dataReceiveInfo->timestamp;
		if (timestampFIFO.head == timestampFIFO.tail) 
		{
			//perror("Timestamp FIFO Buffer UNDERFLOW!");
		}
	}
	else 
	{
		//printf("Assigning frame the timestamp dummy value\n");
		p_dataReceiveInfo->timestamp = -1;
	}
	*/

	return errorCode;
}
//
//static int timestampCalls = 0;
void timestampCallback(int gpio, int level, uint32_t tick, void* userData) 
{
    //static uint64_t lastTime = 0;
    static timestampFIFO_t* FIFO;
    uint32_t thisTime = tick;
    FIFO = (timestampFIFO_t*) userData;
    if (level == 1) {

      FIFO->data[FIFO->tail] = thisTime;
      //printf("timestampCallback %d: %llu\n", timestampCalls++, thisTime);
      FIFO->tail = (FIFO->tail + 1) % FIFO->length;
      if (FIFO->tail == FIFO->head) {
          //perror("Timestamp FIFO Buffer Overflow!");
      }
      printf("DIO0 Rise at: %u\n", thisTime);
    }
    else {
      //printf("DIO0 Fall at: %lu\n", thisTime-lastTime);
    }
    //lastTime = thisTime;
}

/*
void syncTimestamps(volatile timestampFIFO_t* timestampFIFO) 
{
    //procedure
    // Configure MAC to trigger DIO0 when PHY receives Frame by setting MMS12 0012 to 0067 (Maybe 0066)
	T1S_RegWrite(0xc, 0x12, 0x0707); 
    // Set the MAC into promiscous mode by writting bit 16 in MM1 0 to 0
	uint32_t MAC_Conf = T1S_RegRead(0xc, 0x12); 
	MAC_Conf &= ~(1 << 16);
	//T1S_RegWrite(0xc, 0x12, MAC_Conf); 

    // Read and store the number of chunks in the buffer MMS 0 Register 000B
	unsyncronizedChunks = (0x0F) & (T1S_RegRead(0, 0xb) + 1); 
	printf("Begin Syncronizing Timestamps with %d chunks in buffer\n", unsyncronizedChunks);
    // Set up interupt routine to place timestamps into FIFO when DIO0 rises.
    if (0 != gpioSetAlertFuncEx(DIO0_GPIO_PIN, timestampCallback, (void*)timestampFIFO)) {
		// ToDo: Handle Interrupt setup failed
		perror ("Error enabling timestamp interrupt\n");
    }
    syncronizingTimestamps = true;

    // Read from MACPHY buffer and handle as usual but subract the number of chunks read from the previously stored number
    // If after a read the stored chunks == 0, FIFO timestamps now line up with Frames received. Move to synced state.
    // If after a read the stored chunks < 0, an error has occured. Move to an error state.
}
*/

void ReadBufferStatusReg(void)
{
	bool executionStatus;
	stControlCmdReg_t stVarReadRegInfoInput;
	stControlCmdReg_t stVarReadRegData;

	// Reads Buffer Status register from MMS 0
	stVarReadRegInfoInput.memoryMap = 0;
	stVarReadRegInfoInput.length = 0;
	stVarReadRegInfoInput.address = 0x000B;
	//memset(&stVarReadRegInfoInput.databuffer[0], 0, MAX_REG_DATA_ONECHUNK);
	executionStatus = T1S_ReadMACPHYReg(&stVarReadRegInfoInput, &stVarReadRegData);
	if (executionStatus == false)
	{
		// ToDo Action to be taken if reading register fails
		printf("Reading Buffer Status register failed\n");
		g_transmitCreditsAvailable_TXC = 0;
		receiveChunksAvailable_RCA = 0;
	}
	else
	{
		//printf("Buffer Status reg value is 0x%08x \r", stVarReadRegData.databuffer[0]);
		receiveChunksAvailable_RCA = (uint8_t)(stVarReadRegData.databuffer[0] & 0x000000FF);
		g_transmitCreditsAvailable_TXC = (uint8_t)((stVarReadRegData.databuffer[0] >> 8) & 0x000000FF);
	}
}

uint32_t CheckRCABuffAndReceiveData(bool readTAPDataState) 
{
	bool executionStatus;
	uint32_t errorCode;
	uint8_t receiveChunkCount = 0;
	stDataReceive_t stVarReceiveData;
	stControlCmdReg_t stVarReadRegInfoInput;
	//stControlCmdReg_t stVarReadRegData;
	uStatusReg0_t uSTATUSREG0;

	ReadBufferStatusReg();

	if (receiveChunksAvailable_RCA > 0)
	{
		//memset(&uSendDataToTAP.receiveBuffer[0], 0, (MAX_RCA_VALUE * MAX_DATA_DWORD_ONECHUNK));
		isProcessOngoing = true;
		for (receiveChunkCount = 1; receiveChunkCount <= receiveChunksAvailable_RCA; receiveChunkCount++)
		{
			executionStatus = T1S_ReceiveDataChunk(&stVarReceiveData);
			
			if (executionStatus == false)
			{
				printf("Receive Data count %d - FOOTER content\n", receiveChunkCount);
				printf("EXST -> %d\n", stVarReceiveData.receivedFooter.stVarRxFooterBits.EXST);
				printf("HDRB -> %d\n", stVarReceiveData.receivedFooter.stVarRxFooterBits.HDRB);
				printf("SYNC -> %d\n", stVarReceiveData.receivedFooter.stVarRxFooterBits.SYNC);
				printf("RCA  -> %d\n", stVarReceiveData.receivedFooter.stVarRxFooterBits.RCA);
				printf("DV   -> %d\n", stVarReceiveData.receivedFooter.stVarRxFooterBits.DV);
				printf("SV   -> %d\n", stVarReceiveData.receivedFooter.stVarRxFooterBits.SV);
				printf("SWO  -> %d\n", stVarReceiveData.receivedFooter.stVarRxFooterBits.SWO);
				printf("FD   -> %d\n", stVarReceiveData.receivedFooter.stVarRxFooterBits.FD);
				printf("EV   -> %d\n", stVarReceiveData.receivedFooter.stVarRxFooterBits.EV);
				printf("EBO  -> %d\n", stVarReceiveData.receivedFooter.stVarRxFooterBits.EBO);
				printf("TXC  -> %d\n", stVarReceiveData.receivedFooter.stVarRxFooterBits.TXC);
			}
			else
			{
				g_transmitCreditsAvailable_TXC = stVarReceiveData.receivedFooter.stVarRxFooterBits.TXC; 
				//printf("Receive Data Chunk is %d - FOOTER content is 0x%08x \n", stVarReceiveData.receivedFooter.stVarRxFooterBits.RCA, stVarReceiveData.receivedFooter.dataFrameHeadFoot);
			}

			if (stVarReceiveData.receivedFooter.stVarRxFooterBits.EXST)
			{
				// Reads STATUS0 register from MMS 0
				stVarReadRegInfoInput.memoryMap = 0;
				//stVarReadRegInfoInput.length = 0;
				stVarReadRegInfoInput.address = 0x0008;
				memset(&stVarReadRegInfoInput.databuffer[0], 0, MAX_REG_DATA_ONECHUNK);
				//executionStatus = T1S_ReadMACPHYReg(&stVarReadRegInfoInput, &stVarReadRegData);
				uSTATUSREG0.statusRegister0 = T1S_RegRead(stVarReadRegInfoInput.memoryMap, stVarReadRegInfoInput.address);
				//if (executionStatus == false)
				//{
					// ToDo Action to be taken if reading register fails
					//printf("Reading STATUS0 reg in main function failed\n");
				//}
				//else
				{
					//uSTATUSREG0.statusRegister0 = stVarReadRegData.databuffer[0];
					printf("EXST bit set and STATUS0 reg value in main is 0x%08x\n", uSTATUSREG0.statusRegister0);
					printf("TXPE    -> %d\n", uSTATUSREG0.stVarStatusReg0.TXPE);
					printf("HDRE    -> %d\n", uSTATUSREG0.stVarStatusReg0.HDRE);
					printf("TXFCSE  -> %d\n", uSTATUSREG0.stVarStatusReg0.TXFCSE);
					printf("TXBUE   -> %d\n", uSTATUSREG0.stVarStatusReg0.TXBUE);
					printf("TXBOE   -> %d\n", uSTATUSREG0.stVarStatusReg0.TXBOE);
					printf("TTSCAC  -> %d\n", uSTATUSREG0.stVarStatusReg0.TTSCAC);
					printf("TTSCAB  -> %d\n", uSTATUSREG0.stVarStatusReg0.TTSCAB);
					printf("TTSCAA  -> %d\n", uSTATUSREG0.stVarStatusReg0.TTSCAA);
					printf("RXBOE   -> %d\n", uSTATUSREG0.stVarStatusReg0.RXBOE);
					printf("RESETC  -> %d\n", uSTATUSREG0.stVarStatusReg0.RESETC);
					printf("PHYINT  -> %d\n", uSTATUSREG0.stVarStatusReg0.PHYINT);
					printf("LOFE    -> %d\n", uSTATUSREG0.stVarStatusReg0.LOFE);
					printf("CDPE    -> %d\n", uSTATUSREG0.stVarStatusReg0.CDPE);
				}
			}
		}
		receiveChunksAvailable_RCA = 0;
		//printf("POPPING OFF QUEUE ON CHUNK RECEIVE\n");
		//timestampFIFO.head = (timestampFIFO.head + 1) % timestampFIFO.length;
		isProcessOngoing = false;
	}
	else if (readTAPDataState == false) // When no data received is useful, just read data to clear TXC bit
	{
		executionStatus = T1S_ReceiveDataChunk(&stVarReceiveData);
		if (executionStatus == false)
		{
			printf("Failed Receiving data chunk during waiting for TXC to become 0\n");
		}
		else
		{
			//printf("Waiting for TXC to clear and have full value\n");
			g_transmitCreditsAvailable_TXC = stVarReceiveData.receivedFooter.stVarRxFooterBits.TXC; 
		}
	}
	else
	{
		// Just do nothing, May be we can add below sleep code here as well
	}

	if (checkForDataReception)
	{
		bool firstTimeCollect = false;
		uint8_t num_chunks_to_collect = 0;
		do
		{
			if (firstTimeCollect == false)
			{
				num_chunks_to_collect = 0xFF;
				firstTimeCollect = true;
				if (syncronizingTimestamps && unsyncronizedChunks != 0) 
				{
					unsyncronizedChunks -= 1;
					printf("Remaining Unsyncronized Chunks:%d\n", unsyncronizedChunks);
				}
			}
			else
			{
				num_chunks_to_collect = stVarReceiveData.receivedFooter.stVarRxFooterBits.RCA;
				if (syncronizingTimestamps && unsyncronizedChunks != 0) 
				{
					unsyncronizedChunks -= num_chunks_to_collect;
					printf("Remaining Unsyncronized Chunks:%d\n", unsyncronizedChunks);
				}
			}
			if ((errorCode = T1S_Receive(&stVarReceiveData, num_chunks_to_collect)) != OK)
			{
				perror("Failed Receiving data chunk during IRQ request\n");
			}
			else
			{
				g_transmitCreditsAvailable_TXC = stVarReceiveData.receivedFooter.stVarRxFooterBits.TXC; 
			}
		} while (stVarReceiveData.receivedFooter.stVarRxFooterBits.RCA != 0);

		/*if (syncronizingTimestamps && executionStatus) 
		{
			//printf("Poping Queue after receive\n");
			if (timestampFIFO.head == timestampFIFO.tail) 
			{
				//perror("Timestamp FIFO Buffer UNDERFLOW!");
			}
			timestampFIFO.head = (timestampFIFO.head + 1) % timestampFIFO.length;
		}
		*/
		checkForDataReception = false;
	}
	return OK;
}

void NCN26010_Begin(void) 
{
	//bool executionStatus;
	bool readTAPDataStatus = true;
	//stDataReceive_t stVarReceiveData;
	while (1)
	{
		#if 0
		if (TCPIP_ReadTAPAndTransmit(&readTAPDataStatus) != OK) 
		{
			// Error handled can be done over here
			/*executionStatus = T1S_ReceiveDataChunk(&stVarReceiveData);
			if (executionStatus == false)
			{
				printf("Failed Receiving data chunk during waiting for TXC to become 0\n");
			}
			else 
			{
				g_transmitCreditsAvailable_TXC = stVarReceiveData.receivedFooter.stVarRxFooterBits.TXC; 
			}*/

			// No reading Data here. After updating to polling instead of interrupt, all data read at once in CheckRCABuffAndReceiveData()
		}
		#endif
		(void)TCPIP_ReadTAPAndTransmit(&readTAPDataStatus);
		CheckRCABuffAndReceiveData(readTAPDataStatus);

		if (exitFlag == true)
		{
			break;
		}
	}
	SPI_Cleanup();
	//return 0;
}

uint32_t NCN26010_Exit() 
{
	exitFlag = true;
	return OK;
}

void T1S_RemoteRegWrite(uint8_t *mac_addr, uint8_t mms, uint16_t reg_address, uint32_t regValueToWrite)
{
	uint8_t sendConfigFrameBuff[100] = {0};
	//bool executionStatus = true;
	uint16_t sendFrameIndex = 0;
	uint16_t etherTypeForNodeConfig = ETHERNET_TYPE_MACPHYCONFIG;

	sendFrameIndex = 0;
	memcpy(&sendConfigFrameBuff[sendFrameIndex], mac_addr, SIZE_OF_MAC_ADDR);
	sendFrameIndex += SIZE_OF_MAC_ADDR;
	memcpy(&sendConfigFrameBuff[sendFrameIndex], &g_currentNodeMACAddr[0], SIZE_OF_MAC_ADDR);
	sendFrameIndex += SIZE_OF_MAC_ADDR;				

	sendConfigFrameBuff[sendFrameIndex++] = (uint8_t)etherTypeForNodeConfig;
	sendConfigFrameBuff[sendFrameIndex++] = (uint8_t)(etherTypeForNodeConfig >> 8);

	sendConfigFrameBuff[sendFrameIndex++] = (uint8_t)MESSAGETYPE_SEND;

	sendConfigFrameBuff[sendFrameIndex++] = (uint8_t)1; // Number of Config

	sendConfigFrameBuff[sendFrameIndex++] = (uint8_t)CONFIGTYPE_REGWRITE;  // Config Type

	sendConfigFrameBuff[sendFrameIndex++] = mms; 
	
	sendConfigFrameBuff[sendFrameIndex++] = (uint8_t)(reg_address);  // Reg Addr value
	sendConfigFrameBuff[sendFrameIndex++] = (uint8_t)(reg_address >> 8);  // Reg Addr value
	
	sendConfigFrameBuff[sendFrameIndex++] = (uint8_t)(regValueToWrite);  // Reg Value to write
	sendConfigFrameBuff[sendFrameIndex++] = (uint8_t)(regValueToWrite >> 8);  // Reg Value to write
	sendConfigFrameBuff[sendFrameIndex++] = (uint8_t)(regValueToWrite >> 16);  // Reg Value to write
	sendConfigFrameBuff[sendFrameIndex++] = (uint8_t)(regValueToWrite >> 24);  // Reg Value to write

	if (sendFrameIndex > ETHERNET_HEADER_SIZE)
	{
		//stBulkDataTransfer_t stVarBulkTransfer;

		//stVarBulkTransfer.totalBytesToTransfer = sendFrameIndex;
		/*for (uint8_t transmitCount = 0; transmitCount < sendFrameIndex; transmitCount++)
		{
			printf("0x%02X ", sendConfigFrameBuff[transmitCount]);
		}*/
		//printf(" \n  Sent Bytes were above \n");
		//executionStatus = T1S_TransmitBulkData(&sendConfigFrameBuff[0], &stVarBulkTransfer);

		if (T1S_Transmit(&sendConfigFrameBuff[0], sendFrameIndex) != OK)
		{
			printf("Sending bulk data failed during Topology discovery\n");
		}
		else
		{
			//print_message("Remote Write: Sending bulk data was successful during Topo disc, num of sent bytes: %d\n", sendFrameIndex);
		}
	}
	//return executionStatus;
}

void T1S_RemoteRegRead(uint8_t *mac_addr, uint8_t mms, uint16_t reg_address)
{
	uint8_t sendConfigFrameBuff[100] = {0};
	//bool executionStatus;
	uint16_t sendFrameIndex = 0;
	uint16_t etherTypeForNodeConfig = ETHERNET_TYPE_MACPHYCONFIG;

	sendFrameIndex = 0;
	memcpy(&sendConfigFrameBuff[sendFrameIndex], mac_addr, SIZE_OF_MAC_ADDR);
	sendFrameIndex += SIZE_OF_MAC_ADDR;
	memcpy(&sendConfigFrameBuff[sendFrameIndex], &g_currentNodeMACAddr[0], SIZE_OF_MAC_ADDR);
	sendFrameIndex += SIZE_OF_MAC_ADDR;				

	sendConfigFrameBuff[sendFrameIndex++] = (uint8_t)etherTypeForNodeConfig;
	sendConfigFrameBuff[sendFrameIndex++] = (uint8_t)(etherTypeForNodeConfig >> 8);

	sendConfigFrameBuff[sendFrameIndex++] = (uint8_t)MESSAGETYPE_SEND;

	sendConfigFrameBuff[sendFrameIndex++] = (uint8_t)1; // Number of Config

	sendConfigFrameBuff[sendFrameIndex++] = (uint8_t)CONFIGTYPE_REGREAD;  // Config Type

	sendConfigFrameBuff[sendFrameIndex++] = mms;
	
	sendConfigFrameBuff[sendFrameIndex++] = (uint8_t)(reg_address);  // Reg Addr value
	sendConfigFrameBuff[sendFrameIndex++] = (uint8_t)(reg_address >> 8);  // Reg Addr value

	if (sendFrameIndex > ETHERNET_HEADER_SIZE)
	{
		//stBulkDataTransfer_t stVarBulkTransfer;

		//stVarBulkTransfer.totalBytesToTransfer = sendFrameIndex;
		/*for (uint8_t transmitCount = 0; transmitCount < sendFrameIndex; transmitCount++)
		{
			printf("0x%02X ", sendConfigFrameBuff[transmitCount]);
		}*/
		//printf(" \n  Sent Bytes were above \n");
		/*executionStatus = T1S_TransmitBulkData(&sendConfigFrameBuff[0], &stVarBulkTransfer);
		if (executionStatus == false)*/
		if (T1S_Transmit(&sendConfigFrameBuff[0], sendFrameIndex) != OK)
		{
			printf("Sending bulk data failed during Topology discovery\n");
		}
		else
		{
			//print_message("Remote Read: Sending bulk data was successful during Topo disc, num of sent bytes: %d\n", sendFrameIndex);
		}
	}
	//return executionStatus;
}

uint32_t T1S_SendRemoteConfiguration(uint8_t *targetMAC, stRemoteConfigurationCommand_t* commands, uint32_t numCommands)
{
	uint8_t sendConfigFrameBuff[200] = { 0 };
	uint16_t etherTypeForNodeConfig = ETHERNET_TYPE_MACPHYCONFIG;
	uint8_t sendFrameIndex = 0;

	memcpy(&sendConfigFrameBuff[sendFrameIndex], targetMAC, SIZE_OF_MAC_ADDR);
	sendFrameIndex += SIZE_OF_MAC_ADDR;
	memcpy(&sendConfigFrameBuff[sendFrameIndex], g_currentNodeMACAddr, SIZE_OF_MAC_ADDR);
	sendFrameIndex += SIZE_OF_MAC_ADDR;
	sendConfigFrameBuff[sendFrameIndex++] = (uint8_t)etherTypeForNodeConfig;
	sendConfigFrameBuff[sendFrameIndex++] = (uint8_t)(etherTypeForNodeConfig >> 8);
	sendConfigFrameBuff[sendFrameIndex++] = (uint8_t)MESSAGETYPE_SEND;
	//printf("Total Bytes is %d, Step 3 passed\n", sendFrameIndex);
	sendConfigFrameBuff[sendFrameIndex++] = (uint8_t)numCommands; // Number of Config
	for (uint8_t configLoaded = 0; configLoaded < numCommands; configLoaded++)
	{
		sendConfigFrameBuff[sendFrameIndex++] = (uint8_t)commands[configLoaded].type;  // Config Type
		sendConfigFrameBuff[sendFrameIndex++] = (uint8_t)commands[configLoaded].MMS;  // MMS value
		sendConfigFrameBuff[sendFrameIndex++] = (uint8_t)(commands[configLoaded].address);  // Reg Addr value
		sendConfigFrameBuff[sendFrameIndex++] = (uint8_t)(commands[configLoaded].address >> 8);  // Reg Addr value
		if (commands[configLoaded].type == 1)
		{
			sendConfigFrameBuff[sendFrameIndex++] = (uint8_t)(commands[configLoaded].data);  // Reg Value to write
			sendConfigFrameBuff[sendFrameIndex++] = (uint8_t)(commands[configLoaded].data >> 8);  // Reg Value to write
			sendConfigFrameBuff[sendFrameIndex++] = (uint8_t)(commands[configLoaded].data >> 16);  // Reg Value to write
			sendConfigFrameBuff[sendFrameIndex++] = (uint8_t)(commands[configLoaded].data >> 24);  // Reg Value to write
		}
	}
	//printf("2. Reg Remote config, NumOfConfig %d, Config Type %d, MMS 0x%02X, Reg Addr  0x%04X\n", numCommands, configType, regAddress, regValueToWrite);
	print_message("Total Bytes to send over network is %d\n", sendFrameIndex);
	if (sendFrameIndex > ETHERNET_HEADER_SIZE)
	{

		if (T1S_Transmit(&sendConfigFrameBuff[0], sendFrameIndex) != OK)
		{
			printf("Sending bulk data failed during Node Configuration through command line input\n");
			return UNKNOWN_ERROR;
		}
		else
		{
			print_message("Sending bulk data was successful during Node Configuration through command line input, num of sent bytes: %d\n", sendFrameIndex);
		}
	}
	return OK;
}

void T1S_ConfigurePLCA(uint8_t id, uint8_t max_id, bool mode)
{
	PLCA_ID = id;
	if (PLCA_ID == 0)
	{
		PLCA_MAX_NODE = max_id;
	}
	PLCA_MODE = mode;
	print_message("Configuring PLCA , Node ID :%d, max_node_ID : %d\n", PLCA_ID, PLCA_MAX_NODE);
}
