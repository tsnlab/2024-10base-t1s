#include <hardware_dependent.h>
#include <spi.h>

bool T1S_HW_ReadReg(stControlCmdReg_t *p_regInfoInput,  stControlCmdReg_t *p_readRegData)
{
	uint8_t bufferIndex = 0u;
	const uint8_t igoredEchoedBytes = 4u;
	bool readStatus = false;
	uint8_t txBuffer[MAX_REG_DATA_ONECONTROLCMD + HEADER_FOOTER_SIZE + EACH_REG_SIZE] = {0u};
	uint8_t rxBuffer[MAX_REG_DATA_ONECONTROLCMD + HEADER_FOOTER_SIZE + EACH_REG_SIZE] = {0u};
	uint16_t numberOfbytesToSend = 0u;
	uint32_t bigEndianHeader = 0u;
	uCommandHeaderFooter_t commandHeader;
	uCommandHeaderFooter_t commandHeaderEchoed;

	commandHeader.controlFrameHead = 0u;
    commandHeaderEchoed.controlFrameHead = 0u;
	
	commandHeader.stVarHeadFootBits.DNC = DNC_COMMANDTYPE_CONTROL;
	commandHeader.stVarHeadFootBits.HDRB = 0u;
	commandHeader.stVarHeadFootBits.WNR = REG_COMMAND_TYPE_READ; // Read from register
	if (p_regInfoInput->length != 0u)
	{
		commandHeader.stVarHeadFootBits.AID = REG_ADDR_INCREMENT_ENABLE;	// Read register continously from given address
	}
	else
	{
		commandHeader.stVarHeadFootBits.AID = REG_ADDR_INCREMENT_DISABLE;	// Read from same register 
	}
		
	commandHeader.stVarHeadFootBits.MMS = (uint32_t)p_regInfoInput->memoryMap;
	commandHeader.stVarHeadFootBits.ADDR = (uint32_t)p_regInfoInput->address;
	commandHeader.stVarHeadFootBits.LEN = (uint32_t)(p_regInfoInput->length & 0x7Fu);
	commandHeader.stVarHeadFootBits.P = 0u;
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
	}

	return readStatus;
}

bool T1S_HW_WriteReg(stControlCmdReg_t *p_regData)
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
		executionStatus = T1S_HW_ReadReg(&stVarReadRegInfoInput, &stVarReadRegData);
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

uint32_t WriteRegister(uint8_t MMS, uint16_t Address, uint32_t data)
{
	stControlCmdReg_t stVarWriteRegInput;
    bool executionStatus = false;
	stVarWriteRegInput.memoryMap = MMS;
	stVarWriteRegInput.length = 0;
	stVarWriteRegInput.address = Address;
	stVarWriteRegInput.databuffer[0] = data;

	executionStatus = T1S_HW_WriteReg(&stVarWriteRegInput);
	if (executionStatus == true)
	{
		return stVarWriteRegInput.databuffer[0];
	}
	else
	{
        printf("ERROR: Register Write failed at MMS %d, Address %4x\n", MMS, Address);
		return 0;
	}
}

uint32_t ReadRegister(uint8_t MMS, uint16_t Address)
{
	bool executionStatus = false;
	stControlCmdReg_t stVarReadRegInfoInput;
	stControlCmdReg_t stVarReadRegData;
	stVarReadRegInfoInput.memoryMap = MMS;
	stVarReadRegInfoInput.length = 0;
	stVarReadRegInfoInput.address = Address;

	executionStatus = T1S_HW_ReadReg(&stVarReadRegInfoInput, &stVarReadRegData);
	if (executionStatus == true)
	{
		return stVarReadRegData.databuffer[0];
	}
	else
	{
		printf("ERROR: Register Read failed at MMS %d, Address %4x\n", MMS, Address);
		return 0;
	}
}

SPI_ReturnType ClearStatus(void)
{
	bool executionStatus = false;
	stControlCmdReg_t stVarReadRegInfoInput;
	stControlCmdReg_t stVarReadRegData;
    stControlCmdReg_t stVarWriteRegInput;

	// Reads Buffer Status register from MMS 0
	stVarReadRegInfoInput.memoryMap = 0;
	stVarReadRegInfoInput.length = 0;
	stVarReadRegInfoInput.address = 0x0008;
	
	executionStatus = T1S_HW_ReadReg(&stVarReadRegInfoInput, &stVarReadRegData);
	if (executionStatus == false)
	{
		printf("Reading STATUS0 register failed\n");
	}
	else
	{
		print_message("STATUS0 reg value before clearing is 0x%08x\n", stVarReadRegData.databuffer[0]);
	}

	if (stVarReadRegData.databuffer[0] != 0x00000000) // If all values are not at default then set to default
	{
		// Clear STATUS0 register
		stVarWriteRegInput.memoryMap = 0;
		stVarWriteRegInput.length = 0;
		stVarWriteRegInput.address = 0x0008;
		stVarWriteRegInput.databuffer[0] = 0xFFFFFFFF;
		executionStatus = T1S_HW_WriteReg(&stVarWriteRegInput);
		if (executionStatus == false)
		{
			printf("ERROR: Writing into STATUS0 reg failed while clearing error\n");
		}
		else
		{
			print_message("STATUS0 reg written with value 0x%08x successfully\n", stVarWriteRegInput.databuffer[0]);
		}
	}

	// Reads Buffer Status register from MMS 0 after clearing	
	executionStatus = T1S_HW_ReadReg(&stVarReadRegInfoInput, &stVarReadRegData);
	if (executionStatus == false)
	{
		// ToDo Action to be taken if reading register fails
		printf("Reading STATUS0 register failed\n");
		return SPI_E_UNKNOWN_ERROR;
	}
	else
	{
		print_message("STATUS0 reg value after clearing is 0x%08x\n", stVarReadRegData.databuffer[0]);
		return SPI_E_SUCCESS;
	}
}

