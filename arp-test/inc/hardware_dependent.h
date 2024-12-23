/* This header file is based on RaspberryPi 4 and LAN8651 */
#ifndef HARDWARE_DEPENDENT_H
#define HARDWARE_DEPENDENT_H

#define MAX_PAYLOAD_BYTE             (uint8_t) 64 // ToDo This is configurable so need to change based on configuration
#define EACH_REG_SIZE                (uint8_t) 4
#define MAX_REG_DATA_ONECHUNK        (uint8_t) (MAX_PAYLOAD_BYTE/EACH_REG_SIZE)
#define MAX_DATA_DWORD_ONECHUNK      MAX_REG_DATA_ONECHUNK
#define HEADER_FOOTER_SIZE			 (uint8_t)4
#define SIZE_OF_MAC_ADDR			 (uint8_t)6

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

#endif /* HARDWARE_DEPENDENT_H */