#include <arpa/inet.h>

#include "arch.h"
#include "spi.h"

uint8_t g_maxpayloadsize;

bool init_register(int mode) {
    uint32_t regval;
    write_register(0x0u, 0x0003u, 0x00000001u); // Reset LAN8651

    regval = read_register(0x4u, 0x0087u);
    write_register(0x4u, 0x0087u, regval | (1u << 15)); // Initial logic(disable collision detection); Set bit 15

    // PLCA Configuration based on mode
    // TODO: This process is temporary and assumes that there are only two nodes.
    // TODO: Should be changed to get node info. from the command line.
    if (mode == PLCA_MODE_COORDINATOR) {
        write_register(0x4u, 0xCA02u, 0x00000200u); // Coordinator(node 0), 2 nodes
        write_register(0x1u, 0x0022u, 0x313D1AD1u); // Configure MAC Address (Temporary)
        write_register(0x1u, 0x0023u, 0x000C0001u); // Configure MAC Address (Temporary)
    } else if (mode == PLCA_MODE_FOLLOWER) {
        write_register(0x4u, 0xCA02u, 0x00000801u); // Follower, node 1
        write_register(0x1u, 0x0022u, 0x10E13130u); // Configure MAC Address (Temporary)
        write_register(0x1u, 0x0023u, 0x000F0110u); // Configure MAC Address (Temporary)
    } else {
        printf("Invalid mode: %d\n", mode);
        return false;
    }
    write_register(0x4u, 0xCA01u, 0x00008000u); // Enable PLCA
    write_register(0x1u, 0x0001u, 0x000000C0u); // Enable unicast, multicast
    write_register(0x1u, 0x0000u, 0x0000000Cu); // Enable MACPHY TX, RX
    write_register(0x0u, 0x0008u, 0x00000040u); // Clear RESETC
    write_register(0x0u, 0x0004u, 0x00008006u); // SYNC bit SET (last configuration)

    return true;
}

bool t1s_hw_readreg(struct st_ctrl_cmd_reg* p_regInfoInput, struct st_ctrl_cmd_reg* p_readRegData) {
    uint8_t bufferindex = 0u;
    const uint8_t ignored_echoedbytes = 4u;
    bool readstatus = false;
    uint8_t txbuffer[MAX_REG_DATA_ONECONTROLCMD + HEADER_FOOTER_SIZE + EACH_REG_SIZE] = {0u};
    uint8_t rxbuffer[MAX_REG_DATA_ONECONTROLCMD + HEADER_FOOTER_SIZE + EACH_REG_SIZE] = {0u};
    uint16_t numberof_bytestosend = 0u;
    union u_ctrl_header_footer commandheader;
    union u_ctrl_header_footer commandheader_echoed;

    commandheader.ctrl_frame_head = 0u;
    commandheader_echoed.ctrl_frame_head = 0u;
    commandheader.st_ctrl_head_foot_bits.DNC = DNC_COMMANDTYPE_CONTROL;
    commandheader.st_ctrl_head_foot_bits.HDRB = 0u;
    commandheader.st_ctrl_head_foot_bits.WNR = REG_COMMAND_TYPE_READ; // Read from register
    if (p_regInfoInput->length != 0u) {
        commandheader.st_ctrl_head_foot_bits.AID =
            REG_ADDR_INCREMENT_ENABLE; // Read register continously from given address
    } else {
        commandheader.st_ctrl_head_foot_bits.AID = REG_ADDR_INCREMENT_DISABLE; // Read from same register
    }
    commandheader.st_ctrl_head_foot_bits.MMS = (uint32_t)p_regInfoInput->memorymap;
    commandheader.st_ctrl_head_foot_bits.ADDR = (uint32_t)p_regInfoInput->address;
    commandheader.st_ctrl_head_foot_bits.LEN = (uint32_t)(p_regInfoInput->length & 0x7Fu);
    commandheader.st_ctrl_head_foot_bits.P = ((get_parity(commandheader.ctrl_frame_head) == 0) ? 1 : 0);
    for (int8_t header_byte_count = 3; header_byte_count >= 0; header_byte_count--) {
        txbuffer[bufferindex++] = commandheader.ctrl_header_array[header_byte_count];
    }

    numberof_bytestosend =
        (uint16_t)(bufferindex + ((commandheader.st_ctrl_head_foot_bits.LEN + 1u) * EACH_REG_SIZE) +
                   ignored_echoedbytes); // Added extra 4 bytes because first 4 bytes during reception shall be ignored

    spi_transfer((uint8_t*)&rxbuffer[0], (uint8_t*)&txbuffer[0], numberof_bytestosend);

    memmove((uint8_t*)&commandheader_echoed.ctrl_frame_head, &rxbuffer[ignored_echoedbytes], HEADER_FOOTER_SIZE);
    commandheader_echoed.ctrl_frame_head = ntohl(commandheader_echoed.ctrl_frame_head);

    if (commandheader_echoed.st_ctrl_head_foot_bits.HDRB !=
        1u) // if MACPHY received header with parity error then it will be 1
    {
        if (commandheader.st_ctrl_head_foot_bits.LEN == 0u) {
            memmove((uint8_t*)&p_readRegData->databuffer[0], &(rxbuffer[ignored_echoedbytes + HEADER_FOOTER_SIZE]),
                    EACH_REG_SIZE);
            p_readRegData->databuffer[0] = ntohl(p_readRegData->databuffer[0]);
        } else {
            for (uint8_t regCount = 0u; regCount <= commandheader.st_ctrl_head_foot_bits.LEN; regCount++) {
                memmove((uint8_t*)&p_readRegData->databuffer[regCount],
                        &rxbuffer[ignored_echoedbytes + HEADER_FOOTER_SIZE + (EACH_REG_SIZE * regCount)],
                        EACH_REG_SIZE);
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

bool t1s_hw_writereg(struct st_ctrl_cmd_reg* p_regData) {
    uint8_t bufferindex = 0u;
    uint8_t numberof_bytestosend = 0u;
    uint8_t numberof_registerstosend = 0u;
    bool writestatus = true;
    bool execution_status = false;
    const uint8_t ignored_echoedbytes = 4u;
    uint8_t txbuffer[MAX_PAYLOAD_BYTE + HEADER_FOOTER_SIZE] = {0u};
    uint8_t rxbuffer[MAX_PAYLOAD_BYTE + HEADER_FOOTER_SIZE] = {0u};
    union u_ctrl_header_footer commandheader;
    union u_ctrl_header_footer commandheader_echoed;

    commandheader.ctrl_frame_head = commandheader_echoed.ctrl_frame_head = 0u;

    commandheader.st_ctrl_head_foot_bits.DNC = DNC_COMMANDTYPE_CONTROL;
    commandheader.st_ctrl_head_foot_bits.HDRB = 0u;
    commandheader.st_ctrl_head_foot_bits.WNR = REG_COMMAND_TYPE_WRITE; // Write into register
    if (p_regData->length != 0u) {
        commandheader.st_ctrl_head_foot_bits.AID =
            REG_ADDR_INCREMENT_ENABLE; // Write register continously from given address
    } else {
        commandheader.st_ctrl_head_foot_bits.AID = REG_ADDR_INCREMENT_DISABLE; // Write into same register
    }
    commandheader.st_ctrl_head_foot_bits.MMS = (uint32_t)(p_regData->memorymap & 0x0Fu);
    commandheader.st_ctrl_head_foot_bits.ADDR = (uint32_t)p_regData->address;
    commandheader.st_ctrl_head_foot_bits.LEN = (uint32_t)(p_regData->length & 0x7Fu);
    commandheader.st_ctrl_head_foot_bits.P = ((get_parity(commandheader.ctrl_frame_head) == 0) ? 1 : 0);

    for (int8_t header_byte_count = 3; header_byte_count >= 0; header_byte_count--) {
        txbuffer[bufferindex++] = commandheader.ctrl_header_array[header_byte_count];
    }

    numberof_registerstosend = commandheader.st_ctrl_head_foot_bits.LEN + 1u;

    for (uint8_t controlRegIndex = 0u; controlRegIndex < numberof_registerstosend; controlRegIndex++) {
        txbuffer[bufferindex] = htonl(p_regData->databuffer[controlRegIndex]);
        bufferindex += EACH_REG_SIZE;
    }

    numberof_bytestosend =
        (uint8_t)(bufferindex +
                  HEADER_FOOTER_SIZE); // Added extra 4 bytes because last 4 bytes of payload will be ignored by MACPHY

    spi_transfer((uint8_t*)&rxbuffer[0], (uint8_t*)&txbuffer[0], numberof_bytestosend);

    memmove((uint8_t*)&commandheader_echoed.ctrl_frame_head, &rxbuffer[ignored_echoedbytes], HEADER_FOOTER_SIZE);
    commandheader_echoed.ctrl_frame_head = ntohl(commandheader_echoed.ctrl_frame_head);
    // TODO check this logic and modify it if needed
    if (commandheader.st_ctrl_head_foot_bits.MMS == 0u) {
        struct st_ctrl_cmd_reg st_readreg_infoinput;
        struct st_ctrl_cmd_reg st_readreg_data;

        // Reads CONFIG0 register from MMS 0
        st_readreg_infoinput.memorymap = 0u;
        st_readreg_infoinput.length = 0u;
        st_readreg_infoinput.address = 0x0004u;
        memset(&st_readreg_infoinput.databuffer[0], 0u, MAX_REG_DATA_ONECHUNK);

        execution_status = t1s_hw_readreg(&st_readreg_infoinput, &st_readreg_data);
        if (execution_status == false) {
            printf("Reading CONFIG0 reg failed after writing (inside WriteReg)\n");
        } else {
            uint8_t payload_sizeconfiguredval;
            payload_sizeconfiguredval = (st_readreg_data.databuffer[0] & 0x00000007u);

            switch (payload_sizeconfiguredval) {
            case 3:
                g_maxpayloadsize = 8u;
                break;

            case 4:
                g_maxpayloadsize = 16u;
                break;

            case 5:
                g_maxpayloadsize = 32u;
                break;

            case 6:
            default:
                g_maxpayloadsize = 64u;
                break;
            }
            printf("CONFIG0 reg value is 0x%08x in WriteReg function\n", st_readreg_data.databuffer[0]);
        }
    } else if (commandheader.st_ctrl_head_foot_bits.MMS == 1) {
        // CheckIfFCSEnabled();
    } else {
    }

    return writestatus;
}

uint32_t write_register(uint8_t MMS, uint16_t Address, uint32_t data) {
    struct st_ctrl_cmd_reg st_writereg_input;
    bool execution_status = false;
    st_writereg_input.memorymap = MMS;
    st_writereg_input.length = 0;
    st_writereg_input.address = Address;
    st_writereg_input.databuffer[0] = data;

    execution_status = t1s_hw_writereg(&st_writereg_input);
    if (execution_status == true) {
        return st_writereg_input.databuffer[0];
    } else {
        printf("ERROR: Register Write failed at MMS %d, Address %4x\n", MMS, Address);
        return 0;
    }
}

uint32_t read_register(uint8_t MMS, uint16_t Address) {
    bool execution_status = false;
    struct st_ctrl_cmd_reg st_readreg_infoinput;
    struct st_ctrl_cmd_reg st_readreg_data;
    st_readreg_infoinput.memorymap = MMS;
    st_readreg_infoinput.length = 0u;
    st_readreg_infoinput.address = Address;

    execution_status = t1s_hw_readreg(&st_readreg_infoinput, &st_readreg_data);
    if (execution_status == true) {
        return st_readreg_data.databuffer[0];
    } else {
        printf("ERROR: Register Read failed at MMS %d, Address %4x\n", MMS, Address);
        return 0;
    }
}

int clear_status(void) {
    bool execution_status = false;
    struct st_ctrl_cmd_reg st_readreg_infoinput;
    struct st_ctrl_cmd_reg st_readreg_data;
    struct st_ctrl_cmd_reg st_writereg_input;

    // Reads Buffer Status register from MMS 0
    st_readreg_infoinput.memorymap = 0;
    st_readreg_infoinput.length = 0;
    st_readreg_infoinput.address = 0x0008;

    execution_status = t1s_hw_readreg(&st_readreg_infoinput, &st_readreg_data);
    if (execution_status == false) {
        printf("Reading STATUS0 register failed\n");
    } else {
        printf("STATUS0 reg value before clearing is 0x%08x\n", st_readreg_data.databuffer[0]);
    }

    if (st_readreg_data.databuffer[0] != 0x00000000) // If all values are not at default then set to default
    {
        // Clear STATUS0 register
        st_writereg_input.memorymap = 0;
        st_writereg_input.length = 0;
        st_writereg_input.address = 0x0008;
        st_writereg_input.databuffer[0] = 0xFFFFFFFF;
        execution_status = t1s_hw_writereg(&st_writereg_input);
        if (execution_status == false) {
            printf("ERROR: Writing into STATUS0 reg failed while clearing error\n");
        } else {
            printf("STATUS0 reg written with value 0x%08x successfully\n", st_writereg_input.databuffer[0]);
        }
    }

    // Reads Buffer Status register from MMS 0 after clearing
    execution_status = t1s_hw_readreg(&st_readreg_infoinput, &st_readreg_data);
    if (execution_status == false) {
        // ToDo Action to be taken if reading register fails
        printf("Reading STATUS0 register failed\n");
        return -SPI_E_UNKNOWN_ERROR;
    } else {
        printf("STATUS0 reg value after clearing is 0x%08x\n", st_readreg_data.databuffer[0]);
        return SPI_E_SUCCESS;
    }
}
