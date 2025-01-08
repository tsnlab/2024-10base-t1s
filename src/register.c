#include <arpa/inet.h>

#include "arch.h"
#include "spi.h"

uint8_t g_maxpayloadsize;

bool init_register(int mode) {
    uint32_t regval;
    write_register(0x0, 0x0003, 0x00000001); // Reset LAN8651

    regval = read_register(0x4, 0x0087);
    write_register(0x4, 0x0087, regval | (1 << 15)); // Initial logic(disable collision detection); Set bit 15

    // PLCA Configuration based on mode
    // TODO: This process is temporary and assumes that there are only two nodes.
    // TODO: Should be changed to get node info. from the command line.
    if (mode == PLCA_MODE_COORDINATOR) {
        write_register(0x4, 0xCA02, 0x00000200); // Coordinator(node 0), 2 nodes
        write_register(0x1, 0x0022, 0x313D1AD1); // Configure MAC Address (Temporary)
        write_register(0x1, 0x0023, 0x000C0001); // Configure MAC Address (Temporary)
    } else if (mode == PLCA_MODE_FOLLOWER) {
        write_register(0x4, 0xCA02, 0x00000801); // Follower, node 1
        write_register(0x1, 0x0022, 0x10E13130); // Configure MAC Address (Temporary)
        write_register(0x1, 0x0023, 0x000F0110); // Configure MAC Address (Temporary)
    } else {
        printf("Invalid mode: %d\n", mode);
        return false;
    }
    write_register(0x4, 0xCA01, 0x00008000); // Enable PLCA
    write_register(0x1, 0x0001, 0x000000C0); // Enable unicast, multicast
    write_register(0x1, 0x0000, 0x0000000C); // Enable MACPHY TX, RX
    write_register(0x0, 0x0008, 0x00000040); // Clear RESETC
    write_register(0x0, 0x0004, 0x00008006); // SYNC bit SET (last configuration)

    return true;
}

bool t1s_hw_readreg(struct ctrl_cmd_reg* p_regInfoInput, struct ctrl_cmd_reg* p_readRegData) {
    uint8_t bufferindex = 0;
    const uint8_t ignored_echoedbytes = 4;
    bool readstatus = false;
    uint8_t txbuffer[MAX_REG_DATA_ONECONTROLCMD + HEADER_SIZE + REG_SIZE] = {0};
    uint8_t rxbuffer[MAX_REG_DATA_ONECONTROLCMD + FOOTER_SIZE + REG_SIZE] = {0};
    uint16_t numberof_bytestosend = 0;
    union ctrl_header commandheader;
    union ctrl_header commandheader_echoed;

    commandheader.ctrl_frame_head = 0;
    commandheader_echoed.ctrl_frame_head = 0;
    commandheader.ctrl_head_bits.dnc = DNC_COMMANDTYPE_CONTROL;
    commandheader.ctrl_head_bits.hdrb = 0;
    commandheader.ctrl_head_bits.wnr = REG_COMMAND_TYPE_READ; // Read from register
    if (p_regInfoInput->length != 0) {
        commandheader.ctrl_head_bits.aid = REG_ADDR_INCREMENT_ENABLE; // Read register continously from given address
    } else {
        commandheader.ctrl_head_bits.aid = REG_ADDR_INCREMENT_DISABLE; // Read from same register
    }
    commandheader.ctrl_head_bits.mms = (uint32_t)p_regInfoInput->memorymap;
    commandheader.ctrl_head_bits.addr = (uint32_t)p_regInfoInput->address;
    commandheader.ctrl_head_bits.len = (uint32_t)(p_regInfoInput->length & 0x7F);
    commandheader.ctrl_head_bits.p = ((get_parity(commandheader.ctrl_frame_head) == 0) ? 1 : 0);
    for (int8_t header_byte_count = 3; header_byte_count >= 0; header_byte_count--) {
        txbuffer[bufferindex++] = commandheader.ctrl_header_array[header_byte_count];
    }

    numberof_bytestosend =
        (uint16_t)(bufferindex + ((commandheader.ctrl_head_bits.len + 1) * REG_SIZE) +
                   ignored_echoedbytes); // Added extra 4 bytes because first 4 bytes during reception shall be ignored

    spi_transfer((uint8_t*)&rxbuffer[0], (uint8_t*)&txbuffer[0], numberof_bytestosend);

    memmove((uint8_t*)&commandheader_echoed.ctrl_frame_head, &rxbuffer[ignored_echoedbytes], HEADER_SIZE);
    commandheader_echoed.ctrl_frame_head = ntohl(commandheader_echoed.ctrl_frame_head);

    if (commandheader_echoed.ctrl_head_bits.hdrb != 1) // if MACPHY received header with parity error then it will be 1
    {
        if (commandheader.ctrl_head_bits.len == 0) {
            memmove((uint8_t*)&p_readRegData->databuffer[0], &(rxbuffer[ignored_echoedbytes + FOOTER_SIZE]), REG_SIZE);
            p_readRegData->databuffer[0] = ntohl(p_readRegData->databuffer[0]);
        } else {
            for (int regCount = 0; regCount <= commandheader.ctrl_head_bits.len; regCount++) {
                memmove((uint8_t*)&p_readRegData->databuffer[regCount],
                        &rxbuffer[ignored_echoedbytes + FOOTER_SIZE + (REG_SIZE * regCount)], REG_SIZE);
                p_readRegData->databuffer[regCount] = ntohl(p_readRegData->databuffer[regCount]);
            }
        }
        readstatus = true;
    } else {
        // TODO Error handling if MACPHY received with header parity error
        printf("Parity Error READMACPHYReg header value : 0x%08x\n", commandheader_echoed.ctrl_frame_head);
    }

    return readstatus;
}

bool t1s_hw_writereg(struct ctrl_cmd_reg* p_regData) {
    uint8_t bufferindex = 0;
    uint8_t numberof_bytestosend = 0;
    uint8_t numberof_registerstosend = 0;
    bool writestatus = true;
    bool execution_status = false;
    const uint8_t ignored_echoedbytes = 4;
    uint8_t txbuffer[MAX_PAYLOAD_BYTE + HEADER_SIZE] = {0};
    uint8_t rxbuffer[MAX_PAYLOAD_BYTE + FOOTER_SIZE] = {0};
    union ctrl_header commandheader;
    union ctrl_header commandheader_echoed;

    commandheader.ctrl_frame_head = commandheader_echoed.ctrl_frame_head = 0;

    commandheader.ctrl_head_bits.dnc = DNC_COMMANDTYPE_CONTROL;
    commandheader.ctrl_head_bits.hdrb = 0;
    commandheader.ctrl_head_bits.wnr = REG_COMMAND_TYPE_WRITE; // Write into register
    if (p_regData->length != 0) {
        commandheader.ctrl_head_bits.aid = REG_ADDR_INCREMENT_ENABLE; // Write register continously from given address
    } else {
        commandheader.ctrl_head_bits.aid = REG_ADDR_INCREMENT_DISABLE; // Write into same register
    }
    commandheader.ctrl_head_bits.mms = (uint32_t)(p_regData->memorymap & 0x0F);
    commandheader.ctrl_head_bits.addr = (uint32_t)p_regData->address;
    commandheader.ctrl_head_bits.len = (uint32_t)(p_regData->length & 0x7F);
    commandheader.ctrl_head_bits.p = ((get_parity(commandheader.ctrl_frame_head) == 0) ? 1 : 0);

    for (int8_t header_byte_count = 3; header_byte_count >= 0; header_byte_count--) {
        txbuffer[bufferindex++] = commandheader.ctrl_header_array[header_byte_count];
    }

    numberof_registerstosend = commandheader.ctrl_head_bits.len + 1;

    for (uint8_t controlRegIndex = 0; controlRegIndex < numberof_registerstosend; controlRegIndex++) {
        txbuffer[bufferindex] = htonl(p_regData->databuffer[controlRegIndex]);
        bufferindex += REG_SIZE;
    }

    numberof_bytestosend =
        (uint8_t)(bufferindex +
                  HEADER_SIZE); // Added extra 4 bytes because last 4 bytes of payload will be ignored by MACPHY

    spi_transfer((uint8_t*)&rxbuffer[0], (uint8_t*)&txbuffer[0], numberof_bytestosend);

    memmove((uint8_t*)&commandheader_echoed.ctrl_frame_head, &rxbuffer[ignored_echoedbytes], FOOTER_SIZE);
    commandheader_echoed.ctrl_frame_head = ntohl(commandheader_echoed.ctrl_frame_head);
    // TODO check this logic and modify it if needed
    if (commandheader.ctrl_head_bits.mms == 0) {
        struct ctrl_cmd_reg readreg_infoinput;
        struct ctrl_cmd_reg readreg_data;

        // Reads CONFIG0 register from MMS 0
        readreg_infoinput.memorymap = 0;
        readreg_infoinput.length = 0;
        readreg_infoinput.address = 0x0004;
        memset(&readreg_infoinput.databuffer[0], 0, MAX_REG_DATA_ONECHUNK);

        execution_status = t1s_hw_readreg(&readreg_infoinput, &readreg_data);
        if (execution_status == false) {
            printf("Reading CONFIG0 reg failed after writing (inside WriteReg)\n");
        } else {
            uint8_t payload_sizeconfiguredval;
            payload_sizeconfiguredval = (readreg_data.databuffer[0] & 0x00000007);

            switch (payload_sizeconfiguredval) {
            case 3:
                g_maxpayloadsize = 8;
                break;

            case 4:
                g_maxpayloadsize = 16;
                break;

            case 5:
                g_maxpayloadsize = 32;
                break;

            case 6:
            default:
                g_maxpayloadsize = 64;
                break;
            }
            printf("CONFIG0 reg value is 0x%08x in WriteReg function\n", readreg_data.databuffer[0]);
        }
    } else if (commandheader.ctrl_head_bits.mms == 1) {
        // CheckIfFCSEnabled();
    } else {
    }

    return writestatus;
}

uint32_t write_register(uint8_t MMS, uint16_t Address, uint32_t data) {
    struct ctrl_cmd_reg writereg_input;
    bool execution_status = false;
    writereg_input.memorymap = MMS;
    writereg_input.length = 0;
    writereg_input.address = Address;
    writereg_input.databuffer[0] = data;

    execution_status = t1s_hw_writereg(&writereg_input);
    if (execution_status == true) {
        return writereg_input.databuffer[0];
    } else {
        printf("ERROR: Register Write failed at MMS %d, Address %4x\n", MMS, Address);
        return 0;
    }
}

uint32_t read_register(uint8_t MMS, uint16_t Address) {
    bool execution_status = false;
    struct ctrl_cmd_reg readreg_infoinput;
    struct ctrl_cmd_reg readreg_data;
    readreg_infoinput.memorymap = MMS;
    readreg_infoinput.length = 0;
    readreg_infoinput.address = Address;

    execution_status = t1s_hw_readreg(&readreg_infoinput, &readreg_data);
    if (execution_status == true) {
        return readreg_data.databuffer[0];
    } else {
        printf("ERROR: Register Read failed at MMS %d, Address %4x\n", MMS, Address);
        return 0;
    }
}

int clear_status(void) {
    bool execution_status = false;
    struct ctrl_cmd_reg readreg_infoinput;
    struct ctrl_cmd_reg readreg_data;
    struct ctrl_cmd_reg writereg_input;

    // Reads Buffer Status register from MMS 0
    readreg_infoinput.memorymap = 0;
    readreg_infoinput.length = 0;
    readreg_infoinput.address = 0x0008;

    execution_status = t1s_hw_readreg(&readreg_infoinput, &readreg_data);
    if (execution_status == false) {
        printf("Reading STATUS0 register failed\n");
    } else {
        printf("STATUS0 reg value before clearing is 0x%08x\n", readreg_data.databuffer[0]);
    }

    if (readreg_data.databuffer[0] != 0x00000000) // If all values are not at default then set to default
    {
        // Clear STATUS0 register
        writereg_input.memorymap = 0;
        writereg_input.length = 0;
        writereg_input.address = 0x0008;
        writereg_input.databuffer[0] = 0xFFFFFFFF;
        execution_status = t1s_hw_writereg(&writereg_input);
        if (execution_status == false) {
            printf("ERROR: Writing into STATUS0 reg failed while clearing error\n");
        } else {
            printf("STATUS0 reg written with value 0x%08x successfully\n", writereg_input.databuffer[0]);
        }
    }

    // Reads Buffer Status register from MMS 0 after clearing
    execution_status = t1s_hw_readreg(&readreg_infoinput, &readreg_data);
    if (execution_status == false) {
        // TODO: Action to be taken if reading register fails
        printf("Reading STATUS0 register failed\n");
        return -SPI_E_UNKNOWN_ERROR;
    } else {
        printf("STATUS0 reg value after clearing is 0x%08x\n", readreg_data.databuffer[0]);
        return SPI_E_SUCCESS;
    }
}
