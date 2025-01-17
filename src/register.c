#include "register.h"

#include <arpa/inet.h>

#include "arch.h"
#include "spi.h"

uint8_t g_maxpayloadsize;

/* This function is to set the MACPHY register as guided by AN_LAN865x-Configuration
 * In addition to configuring the PHY transceiver in the device, the following configuration
 * configures the MAC to :
 * 1. Set time stamping at the end of the Start of Frame Delimiter (SFD)
 * 2. Set the Timer increment register to 40 ns to be used as a 25MHz internal clock
 */
static void set_macphy_register() {
    uint8_t value1 = read_register(0x04, 0x1F);
    int8_t offset1;
    if ((value1 & 0x10) != 0) {
        offset1 = (int8_t)((uint8_t)value1 - 0x20);
    } else {
        offset1 = (int8_t)value1;
    }

    uint8_t value2 = read_register(0x08, 0x1F);
    int8_t offset2;
    if ((value2 & 0x10) != 0) {
        offset2 = (int8_t)((uint8_t)value2 - 0x20);
    } else {
        offset2 = (int8_t)value2;
    }

    uint16_t cfgparam1 = (uint16_t)(((9 + offset1) & 0x3F) << 10) | (uint16_t)(((14 + offset1) & 0x3F) << 4) | 0x03;
    uint16_t cfgparam2 = (uint16_t)(((40 + offset2) & 0x3F) << 10);

    write_register(MMS4, 0x00D0, 0x3F31);
    write_register(MMS4, 0x00E0, 0xC000);
    write_register(MMS4, 0x0084, cfgparam1);
    write_register(MMS4, 0x008A, cfgparam2);
    write_register(MMS4, 0x00E9, 0x9E50);
    write_register(MMS4, 0x00F5, 0x1CF8);
    write_register(MMS4, 0x00F4, 0xC020);
    write_register(MMS4, 0x00F8, 0xB900);
    write_register(MMS4, 0x00F9, 0x4E53);
    write_register(MMS4, 0x0081, 0x0080);
    write_register(MMS4, 0x0091, 0x9660);
    write_register(MMS4, 0x0077, 0x0028);
    write_register(MMS4, 0x0043, 0x00FF);
    write_register(MMS4, 0x0044, 0xFFFF);
    write_register(MMS4, 0x0045, 0x0000);
    write_register(MMS4, 0x0053, 0x00FF);
    write_register(MMS4, 0x0054, 0xFFFF);
    write_register(MMS4, 0x0055, 0x0000);
    write_register(MMS4, 0x0040, 0x0002);
    write_register(MMS4, 0x0050, 0x0002);
}

/* This function is to set the SQI(Signal Quality Indicator) register as guided by AN_LAN865x-Configuration
 * SQI should be defined in order to use this function
 * See Datasheet for more details
 * This function is not tested.
 */
static void set_sqi_register() {
    uint8_t value1 = read_register(0x04, 0x1F);
    int8_t offset1;
    if ((value1 & 0x10) != 0) {
        offset1 = (int8_t)((uint8_t)value1 - 0x20);
    } else {
        offset1 = (int8_t)value1;
    }

    uint16_t cfgparam3 = (uint16_t)(((5 + offset1) & 0x3F) << 8) | (uint16_t)((9 + offset1) & 0x3F);
    uint16_t cfgparam4 = (uint16_t)(((9 + offset1) & 0x3F) << 8) | (uint16_t)((14 + offset1) & 0x3F);
    uint16_t cfgparam5 = (uint16_t)(((17 + offset1) & 0x3F) << 8) | (uint16_t)((22 + offset1) & 0x3F);

    write_register(MMS4, 0x00AD, cfgparam3);
    write_register(MMS4, 0x00AE, cfgparam4);
    write_register(MMS4, 0x00AF, cfgparam5);
    write_register(MMS4, 0x00B0, 0x0103);
    write_register(MMS4, 0x00B1, 0x0910);
    write_register(MMS4, 0x00B2, 0x1D26);
    write_register(MMS4, 0x00B3, 0x002A);
    write_register(MMS4, 0x00B4, 0x0103);
    write_register(MMS4, 0x00B5, 0x070D);
    write_register(MMS4, 0x00B6, 0x1720);
    write_register(MMS4, 0x00B7, 0x0027);
    write_register(MMS4, 0x00B8, 0x0509);
    write_register(MMS4, 0x00B9, 0x0E13);
    write_register(MMS4, 0x00BA, 0x1C25);
    write_register(MMS4, 0x00BB, 0x002B);
}

bool set_register(int mode) {
    uint32_t regval;
    regval = read_register(MMS4, CDCTL0);
    write_register(MMS4, CDCTL0, regval | (1 << 15)); // Initial logic (disable collision detection)

    // PLCA Configuration based on mode
    // TODO: This process is temporary and assumes that there are only two nodes.
    // TODO: Should be changed to get node info. from the command line.
    if (mode == PLCA_MODE_COORDINATOR) {
        write_register(MMS4, PLCA_CTRL1, 0x00000200); // Coordinator(node 0), 2 nodes
        write_register(MMS1, MAC_SAB1, 0xBEEFBEEF);   // Configure MAC Address (Temporary)
        write_register(MMS1, MAC_SAT1, 0x0000BEEF);   // Configure MAC Address (Temporary)
    } else if (mode == PLCA_MODE_FOLLOWER) {
        write_register(MMS4, PLCA_CTRL1, 0x00000801); // Follower, node 1
        write_register(MMS1, MAC_SAB1, 0xCAFECAFE);   // Configure MAC Address (Temporary)
        write_register(MMS1, MAC_SAT1, 0x0000CAFE);   // Configure MAC Address (Temporary)
    } else {
        printf("Invalid mode: %d\n", mode);
        return false;
    }
    set_macphy_register(); // AN_LAN865x-Configuration
#ifdef SQI
    set_sqi_register();
#endif

    write_register(MMS4, PLCA_CTRL1, 0x00008000); // Enable PLCA
    write_register(MMS1, MAC_NCFGR, 0x000000C0);  // Enable unicast, multicast
    write_register(MMS1, MAC_NCR, 0x0000000C);    // Enable MACPHY TX, RX
    write_register(MMS0, OA_STATUS0, 0x00000040); // Clear RESETC
    write_register(MMS0, OA_CONFIG0, 0x00008006); // SYNC bit SET (last configuration)

    return true;
}

static bool t1s_hw_readreg(struct ctrl_cmd_reg* p_regInfoInput, struct ctrl_cmd_reg* p_readRegData) {
    const uint8_t ignored_echoedbytes = HEADER_SIZE;
    bool readstatus = false;
    uint8_t txbuffer[MAX_REG_DATA_ONECONTROLCMD + HEADER_SIZE + REG_SIZE] = {0};
    uint8_t rxbuffer[MAX_REG_DATA_ONECONTROLCMD + HEADER_SIZE + REG_SIZE] = {0};
    uint16_t numberof_bytestosend = 0;
    union ctrl_header commandheader;
    union ctrl_header commandheader_echoed;
    uint32_t converted_commandheader;

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

    converted_commandheader = htonl(commandheader.ctrl_frame_head);
    memcpy(txbuffer, &converted_commandheader, HEADER_SIZE);

    numberof_bytestosend =
        HEADER_SIZE + ((commandheader.ctrl_head_bits.len + 1) * REG_SIZE) +
        ignored_echoedbytes; // Added extra 4 bytes because first 4 bytes during reception shall be ignored
    spi_transfer((uint8_t*)&rxbuffer[0], (uint8_t*)&txbuffer[0], numberof_bytestosend);

    memcpy((uint8_t*)&commandheader_echoed.ctrl_frame_head, &rxbuffer[ignored_echoedbytes], HEADER_SIZE);
    commandheader_echoed.ctrl_frame_head = ntohl(commandheader_echoed.ctrl_frame_head);

    if (commandheader_echoed.ctrl_head_bits.hdrb != 1) // if MACPHY received header with parity error then it will be 1
    {
        if (commandheader.ctrl_head_bits.len == 0) {
            memcpy((uint8_t*)&p_readRegData->databuffer[0], &(rxbuffer[ignored_echoedbytes + HEADER_SIZE]), REG_SIZE);
            p_readRegData->databuffer[0] = ntohl(p_readRegData->databuffer[0]);
        } else {
            for (int regCount = 0; regCount <= commandheader.ctrl_head_bits.len; regCount++) {
                memcpy((uint8_t*)&p_readRegData->databuffer[regCount],
                       &rxbuffer[ignored_echoedbytes + HEADER_SIZE + (REG_SIZE * regCount)], REG_SIZE);
                p_readRegData->databuffer[regCount] = ntohl(p_readRegData->databuffer[regCount]);
            }
        }
        readstatus = true;
    } else {
        // TODO: Error handling if MACPHY received with header parity error
        printf("Parity Error READMACPHYReg header value : 0x%08x\n", commandheader_echoed.ctrl_frame_head);
    }

    return readstatus;
}

static bool t1s_hw_writereg(struct ctrl_cmd_reg* p_regData) {
    uint8_t numberof_bytestosend = 0;
    uint8_t numberof_registerstosend = 0;
    bool writestatus = true;
    bool execution_status = false;
    const uint8_t ignored_echoedbytes = HEADER_SIZE;
    uint8_t txbuffer[MAX_PAYLOAD_BYTE + HEADER_SIZE] = {0};
    uint8_t rxbuffer[MAX_PAYLOAD_BYTE + HEADER_SIZE] = {0};
    union ctrl_header commandheader;
    union ctrl_header commandheader_echoed;
    uint32_t converted_commandheader;

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

    converted_commandheader = htonl(commandheader.ctrl_frame_head);
    memcpy(txbuffer, &converted_commandheader, HEADER_SIZE);

    numberof_registerstosend = commandheader.ctrl_head_bits.len + 1;
    for (uint8_t controlRegIndex = 0; controlRegIndex < numberof_registerstosend; controlRegIndex++) {
        uint32_t data = htonl(p_regData->databuffer[controlRegIndex]);
        memcpy(&txbuffer[HEADER_SIZE + controlRegIndex * REG_SIZE], &data, sizeof(uint32_t));
    }

    numberof_bytestosend =
        HEADER_SIZE + ((commandheader.ctrl_head_bits.len + 1) * REG_SIZE) +
        ignored_echoedbytes; // Added extra 4 bytes because last 4 bytes of payload will be ignored by MACPHY
    spi_transfer((uint8_t*)&rxbuffer[0], (uint8_t*)&txbuffer[0], numberof_bytestosend);

    memcpy((uint8_t*)&commandheader_echoed.ctrl_frame_head, &rxbuffer[ignored_echoedbytes], HEADER_SIZE);
    commandheader_echoed.ctrl_frame_head = ntohl(commandheader_echoed.ctrl_frame_head);
    // TODO: check this logic and modify it if needed
    if (commandheader.ctrl_head_bits.mms == 0) {
        struct ctrl_cmd_reg readreg_infoinput;
        struct ctrl_cmd_reg readreg_data;

        // Reads CONFIG0 register from MMS 0
        readreg_infoinput.memorymap = MMS0;
        readreg_infoinput.length = 0;
        readreg_infoinput.address = OA_CONFIG0;
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
    readreg_infoinput.memorymap = MMS0;
    readreg_infoinput.length = 0;
    readreg_infoinput.address = OA_STATUS0;

    execution_status = t1s_hw_readreg(&readreg_infoinput, &readreg_data);
    if (execution_status == false) {
        printf("Reading STATUS0 register failed\n");
    } else {
        printf("STATUS0 reg value before clearing is 0x%08x\n", readreg_data.databuffer[0]);
    }

    if (readreg_data.databuffer[0] != 0x00000000) // If all values are not at default then set to default
    {
        // Clear STATUS0 register
        writereg_input.memorymap = MMS0;
        writereg_input.length = 0;
        writereg_input.address = OA_STATUS0;
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
