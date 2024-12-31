#include <hardware_dependent.h>
#include <spi.h>

uint8_t g_maxPayloadSize;

bool InitRegister(PLCA_Mode_t mode) {
    uint32_t regValue;
    WriteRegister(0x0u, 0x0003u, 0x00000001u); // Reset LAN8651

    regValue = ReadRegister(0x4u, 0x0087u);
    WriteRegister(0x4u, 0x0087u, regValue | (1u << 15)); // Initial logic(disable collision detection); Set bit 15

    // PLCA Configuration based on mode
    // TODO: This process is temporary and assumes that there are only two nodes.
    // TODO: Should be changed to get node info. from the command line.
    if (mode == PLCA_MODE_COORDINATOR) {
        WriteRegister(0x4u, 0xCA02u, 0x00000200u); // Coordinator(node 0), 2 nodes
        WriteRegister(0x1u, 0x0022u, 0x313D1AD1u); // Configure MAC Address (Temporary)
        WriteRegister(0x1u, 0x0023u, 0x000C0001u); // Configure MAC Address (Temporary)
    } else if (mode == PLCA_MODE_FOLLOWER) {
        WriteRegister(0x4u, 0xCA02u, 0x00000801u); // Follower, node 1
        WriteRegister(0x1u, 0x0022u, 0x10E13130u); // Configure MAC Address (Temporary)
        WriteRegister(0x1u, 0x0023u, 0x000F0110u); // Configure MAC Address (Temporary)
    } else {
        printf("Invalid mode: %d\n", mode);
        return false;
    }
    WriteRegister(0x4u, 0xCA01u, 0x00008000u); // Enable PLCA
    WriteRegister(0x1u, 0x0001u, 0x000000C0u); // Enable unicast, multicast
    WriteRegister(0x1u, 0x0000u, 0x0000000Cu); // Enable MACPHY TX, RX
    WriteRegister(0x0u, 0x0008u, 0x00000040u); // Clear RESETC
    WriteRegister(0x0u, 0x0004u, 0x00008006u); // SYNC bit SET (last configuration)

    return true;
}

bool T1S_HW_ReadReg(stControlCmdReg_t* p_regInfoInput, stControlCmdReg_t* p_readRegData) {
    uint8_t bufferIndex = 0u;
    const uint8_t ignoredEchoedBytes = 4u;
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
    if (p_regInfoInput->length != 0u) {
        commandHeader.stVarHeadFootBits.AID = REG_ADDR_INCREMENT_ENABLE; // Read register continously from given address
    } else {
        commandHeader.stVarHeadFootBits.AID = REG_ADDR_INCREMENT_DISABLE; // Read from same register
    }
    commandHeader.stVarHeadFootBits.MMS = (uint32_t)p_regInfoInput->memoryMap;
    commandHeader.stVarHeadFootBits.ADDR = (uint32_t)p_regInfoInput->address;
    commandHeader.stVarHeadFootBits.LEN = (uint32_t)(p_regInfoInput->length & 0x7Fu);
    commandHeader.stVarHeadFootBits.P = 0u;
    commandHeader.stVarHeadFootBits.P = (!GetParity(commandHeader.controlFrameHead));
    for (int8_t headerByteCount = 3; headerByteCount >= 0; headerByteCount--) {
        txBuffer[bufferIndex++] = commandHeader.controlHeaderArray[headerByteCount];
    }

    numberOfbytesToSend =
        (uint16_t)(bufferIndex + ((commandHeader.stVarHeadFootBits.LEN + 1u) * EACH_REG_SIZE) +
                   ignoredEchoedBytes); // Added extra 4 bytes because first 4 bytes during reception shall be ignored

    SPI_Transfer((uint8_t*)&rxBuffer[0], (uint8_t*)&txBuffer[0], numberOfbytesToSend);

    memmove((uint8_t*)&commandHeaderEchoed.controlFrameHead, &rxBuffer[ignoredEchoedBytes], HEADER_FOOTER_SIZE);
    ConvertEndianness(commandHeaderEchoed.controlFrameHead, &bigEndianHeader);
    commandHeaderEchoed.controlFrameHead = bigEndianHeader;

    if (commandHeaderEchoed.stVarHeadFootBits.HDRB !=
        1u) // if MACPHY received header with parity error then it will be 1
    {
        uint32_t endiannessConvertedValue = 0u;
        if (commandHeader.stVarHeadFootBits.LEN == 0u) {
            memmove((uint8_t*)&p_readRegData->databuffer[0], &(rxBuffer[ignoredEchoedBytes + HEADER_FOOTER_SIZE]),
                    EACH_REG_SIZE);
            ConvertEndianness(p_readRegData->databuffer[0], &endiannessConvertedValue);
            p_readRegData->databuffer[0] = endiannessConvertedValue;
        } else {
            for (uint8_t regCount = 0u; regCount <= commandHeader.stVarHeadFootBits.LEN; regCount++) {
                memmove((uint8_t*)&p_readRegData->databuffer[regCount],
                        &rxBuffer[ignoredEchoedBytes + HEADER_FOOTER_SIZE + (EACH_REG_SIZE * regCount)], EACH_REG_SIZE);
                ConvertEndianness(p_readRegData->databuffer[regCount], &endiannessConvertedValue);
                p_readRegData->databuffer[regCount] = endiannessConvertedValue;
            }
        }
        readStatus = true;
    } else {
        // TODO Error handling if MACPHY received with header parity error
        printf("Parity Error READMACPHYReg header value : 0x%08x\n", commandHeaderEchoed.controlFrameHead);
    }

    return readStatus;
}

bool T1S_HW_WriteReg(stControlCmdReg_t* p_regData) {
    uint8_t bufferIndex = 0u;
    uint8_t numberOfbytesToSend = 0u;
    uint8_t numberOfRegistersToSend = 0u;
    bool writeStatus = true;
    bool executionStatus = false;
    const uint8_t ignoredEchoedBytes = 4u;
    uint8_t txBuffer[MAX_PAYLOAD_BYTE + HEADER_FOOTER_SIZE] = {0u};
    uint8_t rxBuffer[MAX_PAYLOAD_BYTE + HEADER_FOOTER_SIZE] = {0u};
    uint32_t bigEndianHeader = 0u;
    uint32_t endiannessConvertedValue = 0u;
    uCommandHeaderFooter_t commandHeader;
    uCommandHeaderFooter_t commandHeaderEchoed;

    commandHeader.controlFrameHead = commandHeaderEchoed.controlFrameHead = 0u;

    commandHeader.stVarHeadFootBits.DNC = DNC_COMMANDTYPE_CONTROL;
    commandHeader.stVarHeadFootBits.HDRB = 0u;
    commandHeader.stVarHeadFootBits.WNR = REG_COMMAND_TYPE_WRITE; // Write into register
    if (p_regData->length != 0u) {
        commandHeader.stVarHeadFootBits.AID =
            REG_ADDR_INCREMENT_ENABLE; // Write register continously from given address
    } else {
        commandHeader.stVarHeadFootBits.AID = REG_ADDR_INCREMENT_DISABLE; // Write into same register
    }
    commandHeader.stVarHeadFootBits.MMS = (uint32_t)(p_regData->memoryMap & 0x0Fu);
    commandHeader.stVarHeadFootBits.ADDR = (uint32_t)p_regData->address;
    commandHeader.stVarHeadFootBits.LEN = (uint32_t)(p_regData->length & 0x7Fu);
    commandHeader.stVarHeadFootBits.P = 0u;
    commandHeader.stVarHeadFootBits.P = (!GetParity(commandHeader.controlFrameHead));

    for (int8_t headerByteCount = 3; headerByteCount >= 0; headerByteCount--) {
        txBuffer[bufferIndex++] = commandHeader.controlHeaderArray[headerByteCount];
    }

    numberOfRegistersToSend = commandHeader.stVarHeadFootBits.LEN + 1u;

    for (uint8_t controlRegIndex = 0u; controlRegIndex < numberOfRegistersToSend; controlRegIndex++) {
        ConvertEndianness(p_regData->databuffer[controlRegIndex], &endiannessConvertedValue);
        memcpy(&txBuffer[bufferIndex], &endiannessConvertedValue, EACH_REG_SIZE);
        bufferIndex += EACH_REG_SIZE;
    }

    numberOfbytesToSend =
        (uint8_t)(bufferIndex +
                  HEADER_FOOTER_SIZE); // Added extra 4 bytes because last 4 bytes of payload will be ignored by MACPHY

    SPI_Transfer((uint8_t*)&rxBuffer[0], (uint8_t*)&txBuffer[0], numberOfbytesToSend);

    memmove((uint8_t*)&commandHeaderEchoed.controlFrameHead, &rxBuffer[ignoredEchoedBytes], HEADER_FOOTER_SIZE);
    ConvertEndianness(commandHeaderEchoed.controlFrameHead, &bigEndianHeader);
    commandHeaderEchoed.controlFrameHead = bigEndianHeader;
    // TODO check this logic and modify it if needed
    if (commandHeader.stVarHeadFootBits.MMS == 0u) {
        stControlCmdReg_t stVarReadRegInfoInput;
        stControlCmdReg_t stVarReadRegData;

        // Reads CONFIG0 register from MMS 0
        stVarReadRegInfoInput.memoryMap = 0u;
        stVarReadRegInfoInput.length = 0u;
        stVarReadRegInfoInput.address = 0x0004u;
        memset(&stVarReadRegInfoInput.databuffer[0], 0u, MAX_REG_DATA_ONECHUNK);

        executionStatus = T1S_HW_ReadReg(&stVarReadRegInfoInput, &stVarReadRegData);
        if (executionStatus == false) {
            printf("Reading CONFIG0 reg failed after writing (inside WriteReg)\n");
        } else {
            uint8_t payloadSizeConfiguredValue;
            payloadSizeConfiguredValue = (stVarReadRegData.databuffer[0] & 0x00000007u);

            switch (payloadSizeConfiguredValue) {
            case 3:
                g_maxPayloadSize = 8u;
                break;

            case 4:
                g_maxPayloadSize = 16u;
                break;

            case 5:
                g_maxPayloadSize = 32u;
                break;

            case 6:
            default:
                g_maxPayloadSize = 64u;
                break;
            }
            printf("CONFIG0 reg value is 0x%08x in WriteReg function\n", stVarReadRegData.databuffer[0]);
        }
    } else if (commandHeader.stVarHeadFootBits.MMS == 1) {
        // CheckIfFCSEnabled();
    } else {
    }

    return writeStatus;
}

uint32_t WriteRegister(uint8_t MMS, uint16_t Address, uint32_t data) {
    stControlCmdReg_t stVarWriteRegInput;
    bool executionStatus = false;
    stVarWriteRegInput.memoryMap = MMS;
    stVarWriteRegInput.length = 0;
    stVarWriteRegInput.address = Address;
    stVarWriteRegInput.databuffer[0] = data;

    executionStatus = T1S_HW_WriteReg(&stVarWriteRegInput);
    if (executionStatus == true) {
        return stVarWriteRegInput.databuffer[0];
    } else {
        printf("ERROR: Register Write failed at MMS %d, Address %4x\n", MMS, Address);
        return 0;
    }
}

uint32_t ReadRegister(uint8_t MMS, uint16_t Address) {
    bool executionStatus = false;
    stControlCmdReg_t stVarReadRegInfoInput;
    stControlCmdReg_t stVarReadRegData;
    stVarReadRegInfoInput.memoryMap = MMS;
    stVarReadRegInfoInput.length = 0u;
    stVarReadRegInfoInput.address = Address;

    executionStatus = T1S_HW_ReadReg(&stVarReadRegInfoInput, &stVarReadRegData);
    if (executionStatus == true) {
        return stVarReadRegData.databuffer[0];
    } else {
        printf("ERROR: Register Read failed at MMS %d, Address %4x\n", MMS, Address);
        return 0;
    }
}

SPI_ReturnType ClearStatus(void) {
    bool executionStatus = false;
    stControlCmdReg_t stVarReadRegInfoInput;
    stControlCmdReg_t stVarReadRegData;
    stControlCmdReg_t stVarWriteRegInput;

    // Reads Buffer Status register from MMS 0
    stVarReadRegInfoInput.memoryMap = 0;
    stVarReadRegInfoInput.length = 0;
    stVarReadRegInfoInput.address = 0x0008;

    executionStatus = T1S_HW_ReadReg(&stVarReadRegInfoInput, &stVarReadRegData);
    if (executionStatus == false) {
        printf("Reading STATUS0 register failed\n");
    } else {
        printf("STATUS0 reg value before clearing is 0x%08x\n", stVarReadRegData.databuffer[0]);
    }

    if (stVarReadRegData.databuffer[0] != 0x00000000) // If all values are not at default then set to default
    {
        // Clear STATUS0 register
        stVarWriteRegInput.memoryMap = 0;
        stVarWriteRegInput.length = 0;
        stVarWriteRegInput.address = 0x0008;
        stVarWriteRegInput.databuffer[0] = 0xFFFFFFFF;
        executionStatus = T1S_HW_WriteReg(&stVarWriteRegInput);
        if (executionStatus == false) {
            printf("ERROR: Writing into STATUS0 reg failed while clearing error\n");
        } else {
            printf("STATUS0 reg written with value 0x%08x successfully\n", stVarWriteRegInput.databuffer[0]);
        }
    }

    // Reads Buffer Status register from MMS 0 after clearing
    executionStatus = T1S_HW_ReadReg(&stVarReadRegInfoInput, &stVarReadRegData);
    if (executionStatus == false) {
        // ToDo Action to be taken if reading register fails
        printf("Reading STATUS0 register failed\n");
        return SPI_E_UNKNOWN_ERROR;
    } else {
        printf("STATUS0 reg value after clearing is 0x%08x\n", stVarReadRegData.databuffer[0]);
        return SPI_E_SUCCESS;
    }
}
