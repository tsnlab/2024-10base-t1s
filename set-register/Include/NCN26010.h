#ifndef NCN26010_H
#define NCN26010_H
/***************************************************************************//**
* @mainpage :    10Base-T1S MACPHY - NCN26010
***************************************************************************//**
* @file     : NCN26010.h
* @brief     : Support functions to simplify utilization of NCN26010 Features
* @author     : Arndt Schuebel, Kyle Storey
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
/*************************************************************************************************
*  Configuration
*************************************************************************************************/
#define REMOTE_CONFIGURATION_ENABLED            (bool) true
#define PLCA_ENABLED                            (bool) true
#define PLCA_ROLE                               (bool) false //Not a leader unless ID 0
#define PLCA_ID_INIT                            (uint8_t) 0 //LEADER
#define PLCA_NODE_COUNT                         (uint8_t) 4 //4 nodes
#define ENI_ENABLED                             (bool) false //Enhanced Noise Immunity 
//. . . 

/*************************************************************************************************
*  Symbolic constants                                                                            *
*************************************************************************************************/

#define MAX_PAYLOAD_BYTE             (uint8_t) 64 // ToDo This is configurable so need to change based on configuration
#define EACH_REG_SIZE                (uint8_t) 4
#define MAX_REG_DATA_ONECHUNK        (uint8_t) (MAX_PAYLOAD_BYTE/EACH_REG_SIZE)
#define MAX_DATA_DWORD_ONECHUNK      MAX_REG_DATA_ONECHUNK
#define HEADER_FOOTER_SIZE			 (uint8_t)4
#define SIZE_OF_MAC_ADDR			 (uint8_t)6

/*************************************************************************************************
 *   Error Codes
 ************************************************************************************************/

#define OK                             (uint32_t) 0
#define UNKNOWN_ERROR                  (uint32_t) 1
#define PHYSICAL_COLLISSION_ERROR      2
#define PLCA_RECOVERY_ERROR            3
#define REMOTE_JABBER_ERROR            4
#define LOCAL_JABBER_ERROR             5
#define RX_BUFFER_OVERFLOW_ERROR       6
#define TX_BUFFER_OVERFLOW_ERROR       7
#define TX_BUFFER_UNDERFLOW_ERROR      8
#define REGISTER_READ_ERROR            9
#define INIT_ERROR		       		  10
//TODO FILL IN ALL POSSIBLE ERRORS

/**************************************************************************************************
 *       Structures
 *************************************************************************************************/
typedef union
{
	uint32_t dataFrameHeadFoot;
	uint8_t dataFrameHeaderBuffer[HEADER_FOOTER_SIZE];
	struct stTxHeadBits
	{
		uint32_t P		: 1;
		uint32_t RSVD3	: 5;
		uint32_t TSC  	: 2;
		uint32_t EBO	: 6;
		uint32_t EV  	: 1;
		uint32_t RSVD2	: 1;
		uint32_t SWO	: 4;
		uint32_t SV  	: 1;
		uint32_t DV 	: 1;
		uint32_t VS		: 2;
		uint32_t RSVD1	: 5;
		uint32_t NORX	: 1;
		uint32_t SEQ	: 1;
		uint32_t DNC	: 1;
	}stVarTxHeadBits;
	
	struct stRxFooterBits
	{
		uint32_t P		: 1;
		uint32_t TXC	: 5;
		uint32_t RTPS  	: 1;
		uint32_t RTSA	: 1;
		uint32_t EBO  	: 6;
		uint32_t EV		: 1;
		uint32_t FD		: 1;
		uint32_t SWO  	: 4;
		uint32_t SV 	: 1;
		uint32_t DV		: 1;
		uint32_t VS		: 2;
		uint32_t RCA	: 5;
		uint32_t SYNC	: 1;
		uint32_t HDRB	: 1;
		uint32_t EXST	: 1;
	}stVarRxFooterBits;
} uDataHeaderFooter_t;

typedef struct
{
	uDataHeaderFooter_t receivedFooter;
	uint64_t timestamp;
}stDataReceive_t;

typedef struct
{
    uint8_t type;
    uint8_t MMS;
    uint16_t address;
    uint32_t data;
} stRemoteConfigurationCommand_t;

/*************************************************************************************************
*  		Global Variables                                                                 *
*************************************************************************************************/
extern bool g_isTopoDiscOngoing;
extern uint8_t g_maxPayloadSize;
extern uint8_t g_transmitCreditsAvailable_TXC;
extern uint8_t g_topoMACAddrReceived[SIZE_OF_MAC_ADDR];
extern const uint8_t g_broadCastMACAddr[SIZE_OF_MAC_ADDR];
extern uint16_t g_numOfTopoDiscTimesToTry;
extern double g_measuredLength;
extern double g_topoLengthHighestValue;
extern double g_topoLengthLeastValue;
extern uint16_t g_topoMeasuredCount;
extern bool g_isTopoDiscFailed;

/*************************************************************************************************
 *   Prototypes
 ************************************************************************************************/
extern uint8_t ASCIIValueConversion(uint8_t asciiValue);
extern void T1S_ConfigurePLCA(uint8_t id, uint8_t max_id, bool mode); //MUST BE CALLED BEFORE INIT
extern uint32_t NCN26010_Init(); // Configures as defined in this header. Returns 0 on success otherwise an error code
extern void NCN26010_Begin(void); //May never return pending on implementation.
extern uint32_t T1S_Transmit(uint8_t* txBuffer, uint16_t num_bytes_to_transmit); //create a ethernet frame and transfer the given data. Return 0 on success otherwise an error code connected with the problem
extern uint32_t T1S_RegRead(uint8_t MMS, uint16_t address); //return the value in the given address
extern uint32_t T1S_RegWrite(uint8_t MMS, uint16_t Address, uint32_t data);//write the given value in the given address
//Some data translation may be needed
extern uint32_t T1S_DiscoverTopology(void* topology, uint8_t targetMAC[SIZE_OF_MAC_ADDR]); //returns a struct with toplology information. Struct to be defined
//TODO Define Topology type
extern uint32_t T1S_SendRemoteConfiguration(uint8_t *targetMAC, stRemoteConfigurationCommand_t* commands, uint32_t numCommands);
extern uint32_t NCN26010_Exit(); //Causes NCN26010_Begin() to return must be called asyncronously. Performs cleanup code
//extern void TopoDisc_SetVoltAndCalibrate(void);
extern void TopoDisc_BeginMeasurement(void);
extern void ReadBufferStatusReg(void);
extern void TopoDisc_CompleteProcess(void);
extern void T1S_RemoteRegWrite(uint8_t *mac_addr, uint8_t mms, uint16_t reg_address, uint32_t regValueToWrite);
extern void T1S_RemoteRegRead(uint8_t *mac_addr, uint8_t mms, uint16_t reg_address);
extern int print_message(const char *format, ...);

#endif /* NCN26010_H */
