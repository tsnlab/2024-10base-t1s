#include "register.h"

#include <arpa/inet.h>

#include "arch.h"
#include "spi.h"

uint8_t g_maxpayloadsize;

struct reginfo reg_open_alliance[] = {{"Identification Register", OA_ID},
                                      {"PHY Identification Register", OA_PHYID},
                                      {"Standard Capabilities", OA_STDCAP},
                                      {"Reset Control and Status Register", OA_RESET},
                                      {"Configuration 0 Register", OA_CONFIG0},
                                      {"Status 0 Register", OA_STATUS0},
                                      {"Status 1 Register", OA_STATUS1},
                                      {"Buffer Status Register", OA_BUFSTS},
                                      {"Interrupt Mask 0 Register", OA_IMASK0},
                                      {"Interrupt Mask 1 Register", OA_IMASK1},
                                      {"Transmit Timestamp Capture A (High)", TTSCAH},
                                      {"Transmit Timestamp Capture A (Low)", TTSCAL},
                                      {"Transmit Timestamp Capture B (High)", TTSCBH},
                                      {"Transmit Timestamp Capture B (Low)", TTSCBL},
                                      {"Transmit Timestamp Capture C (High)", TTSCCH},
                                      {"Transmit Timestamp Capture C (Low)", TTSCCL},
                                      {"Basic Control", BASIC_CONTROL},
                                      {"Basic Status", BASIC_STATUS},
                                      {"PHY Identifier 1 Register", PHY_ID1},
                                      {"PHY Identifier 2 Register", PHY_ID2},
                                      {"MMD Access Control Register", MMDCTRL},
                                      {"MMD Access Address/Data Register", MMDAD},
                                      {"", -1}};

struct reginfo reg_mac[] = {{"Network Control Register", MAC_NCR},
                            {"Network Configuration Register", MAC_NCFGR},
                            {"Hash Register Bottom", MAC_HRB},
                            {"Hash Register Top", MAC_HRT},
                            {"Specific Address 1 Bottom", MAC_SAB1},
                            {"Specific Address 1 Top", MAC_SAT1},
                            {"Specific Address 2 Bottom", MAC_SAB2},
                            {"Specific Address 2 Top", MAC_SAT2},
                            {"Specific Address 3 Bottom", MAC_SAB3},
                            {"Specific Address 3 Top", MAC_SAT3},
                            {"Specific Address 4 Bottom", MAC_SAB4},
                            {"Specific Address 4 Top", MAC_SAT4},
                            {"MAC Type ID Match 1", MAC_TIDM1},
                            {"MAC Type ID Match 2", MAC_TIDM2},
                            {"MAC Type ID Match 3", MAC_TIDM3},
                            {"MAC Type ID Match 4", MAC_TIDM4},
                            {"Specific Address Match 1 Bottom", MAC_SAMB1},
                            {"Specific Address Match 1 Top", MAC_SAMT1},
                            {"Timer Increment Sub-Nanoseconds", MAC_TISUBN},
                            {"Timestamp Seconds High", MAC_TSH},
                            {"Timestamp Seconds Low", MAC_TSL},
                            {"Timestamp Nanoseconds", MAC_TN},
                            {"TSU Timer Adjust", MAC_TA},
                            {"TSU Timer Increment", MAC_TI},
                            {"Buffer Manager Control", BMGR_CTL},
                            {"Statistics 0", STATS0},
                            {"Statistics 1", STATS1},
                            {"Statistics 2", STATS2},
                            {"Statistics 3", STATS3},
                            {"Statistics 4", STATS4},
                            {"Statistics 5", STATS5},
                            {"Statistics 6", STATS6},
                            {"Statistics 7", STATS7},
                            {"Statistics 8", STATS8},
                            {"Statistics 9", STATS9},
                            {"Statistics 10", STATS10},
                            {"Statistics 11", STATS11},
                            {"Statistics 12", STATS12},
                            {"", -1}};

struct reginfo reg_phy_pcs[] = {{"10BASE-T1S PCS Control", T1SPCSCTL},
                                {"10BASE-T1S PCS Status", T1SPCSSTS},
                                {"10BASE-T1S PCS Diagnostic 1", T1SPCSDIAG1},
                                {"10BASE-T1S PCS Diagnostic 2", T1SPCSDIAG2},
                                {"", -1}};

struct reginfo reg_phy_pma_pmd[] = {{"BASE-T1 PMA/PMD Extended Ability", T1PMAPMDEXTA},
                                    {"BASE-T1 PMA/PMD Control", T1PMAPMDCTL},
                                    {"10BASE-T1S PMA Control", T1SPMACTL},
                                    {"10BASE-T1S PMA Status", T1SPMASTS},
                                    {"10BASE-T1S Test Mode Control", T1STSTCTL},
                                    {"", -1}};

struct reginfo reg_phy_vendor_specific[] = {{"Control 1 Register", CTRL1},
                                            {"Status 1 Register", STS1},
                                            {"Status 2 Register", STS2},
                                            {"Status 3 Register", STS3},
                                            {"Interrupt Mask 1 Register", IMSK1},
                                            {"Interrupt Mask 2 Register", IMSK2},
                                            {"Counter Control Register", CTRCTRL},
                                            {"Transmit Opportunity Count (High)", TOCNTH},
                                            {"Transmit Opportunity Count (Low)", TOCNTL},
                                            {"BEACON Count (High)", BCNCNTH},
                                            {"BEACON Count (Low)", BCNCNTL},
                                            {"PLCA Multiple ID 0 Register", MULTID0},
                                            {"PLCA Multiple ID 1 Register", MULTID1},
                                            {"PLCA Multiple ID 2 Register", MULTID2},
                                            {"PLCA Multiple ID 3 Register", MULTID3},
                                            {"PLCA Reconciliation Sublayer Status", PRSSTS},
                                            {"Port Management 2", PRTMGMT2},
                                            {"Inactivity Watchdog Timeout (High)", IWDTOH},
                                            {"Inactivity Watchdog Timeout (Low)", IWDTOL},
                                            {"Transmit Match Control Register", TXMCTL},
                                            {"Transmit Match Pattern (High) Register", TXMPATH},
                                            {"Transmit Match Pattern (Low) Register", TXMPATL},
                                            {"Transmit Match Mask (High) Register", TXMMSKH},
                                            {"Transmit Match Mask (Low) Register", TXMMSKL},
                                            {"Transmit Match Location Register", TXMLOC},
                                            {"Transmit Matched Packet Delay Register", TXMDLY},
                                            {"Receive Match Control Register", RXMCTL},
                                            {"Receive Match Pattern (High) Register", RXMPATH},
                                            {"Receive Match Pattern (Low) Register", RXMPATL},
                                            {"Receive Match Mask (High) Register", RXMMSKH},
                                            {"Receive Match Mask (Low) Register", RXMMSKL},
                                            {"Receive Match Location Register", RXMLOC},
                                            {"Receive Matched Packet Delay Register", RXMDLY},
                                            {"Credit Based Shaper Stop Threshold (High) Register", CBSSPTHH},
                                            {"Credit Based Shaper Stop Threshold (Low) Register", CBSSPTHL},
                                            {"Credit Based Shaper Start Threshold (High) Register", CBSSTTHH},
                                            {"Credit Based Shaper Start Threshold (Low) Register", CBSSTTHL},
                                            {"Credit Based Shaper Slope Control Register", CBSSLPCTL},
                                            {"Credit Based Shaper Top Limit (High) Register", CBSTPLMTH},
                                            {"Credit Based Shaper Top Limit (Low) Register", CBSTPLMTL},
                                            {"Credit Based Shaper Bottom Limit (High) Register", CBSBTLMTH},
                                            {"Credit Based Shaper Bottom Limit (Low) Register", CBSBTLMTL},
                                            {"Credit Based Shaper Credit Counter (High) Register", CBSCRCTRH},
                                            {"Credit Based Shaper Credit Counter (Low) Register", CBSCRCTRL},
                                            {"Credit Based Shaper Control Register", CBSCTRL},
                                            {"PLCA Skip Control Register", PLCASKPCTL},
                                            {"PLCA Transmit Opportunity Skip Register", PLCATOSKP},
                                            {"Application Controlled Media Access Control Register", ACMACTL},
                                            {"Sleep Control 0 Register", SLPCTL0},
                                            {"Sleep Control 1 Register", SLPCTL1},
                                            {"Collision Detector Control 0 Register", CDCTL0},
                                            {"SQI Control Register", SQICTL},
                                            {"SQI Status 0 Register", SQISTS0},
                                            {"SQI Configuration 0 Register", SQICFG0},
                                            {"SQI Configuration 2 Register", SQICFG2},
                                            {"Analog Control 5", ANALOG5},
                                            {"OPEN Alliance Map ID and Version Register", MIDVER},
                                            {"PLCA Control 0 Register", PLCA_CTRL0},
                                            {"PLCA Control 1 Register", PLCA_CTRL1},
                                            {"PLCA Status Register", PLCA_STS},
                                            {"PLCA Transmit Opportunity Timer Register", PLCA_TOTMR},
                                            {"PLCA Burst Mode Register", PLCA_BURST},
                                            {"", -1}};

struct reginfo reg_miscellaneous[] = {{"Queue Transmit Configuration", QTXCFG},
                                      {"Queue Receive Configuration", QRXCFG},
                                      {"Pad Control", PADCTRL},
                                      {"Clock Output Control", CLKOCTL},
                                      {"Miscellaneous", MISC},
                                      {"Device Identification", DEVID},
                                      {"Bus Parity Control and Status", BUSPCS},
                                      {"Configuration Protection Control", CFGPRTCTL},
                                      {"SRAM Error Correction Code Control", ECCCTRL},
                                      {"SRAM Error Correction Code Status", ECCSTS},
                                      {"SRAM Error Correction Code Fault Injection Control", ECCFLTCTRL},
                                      {"Event Capture 0 Control", EC0CTRL},
                                      {"Event Capture 1 Control", EC1CTRL},
                                      {"Event Capture 2 Control", EC2CTRL},
                                      {"Event Capture 3 Control", EC3CTRL},
                                      {"Event Capture Read Status Register", ECRDSTS},
                                      {"Event Capture Total Counts Register", ECTOT},
                                      {"Event Capture Clock Seconds High Register", ECCLKSH},
                                      {"Event Capture Clock Seconds Low Register", ECCLKSL},
                                      {"Event Capture Clock Nanoseconds Register", ECCLKNS},
                                      {"Event Capture Read Timestamp Register 0", ECRDTS0},
                                      {"Event Capture Read Timestamp Register 1", ECRDTS1},
                                      {"Event Capture Read Timestamp Register 2", ECRDTS2},
                                      {"Event Capture Read Timestamp Register 3", ECRDTS3},
                                      {"Event Capture Read Timestamp Register 4", ECRDTS4},
                                      {"Event Capture Read Timestamp Register 5", ECRDTS5},
                                      {"Event Capture Read Timestamp Register 6", ECRDTS6},
                                      {"Event Capture Read Timestamp Register 7", ECRDTS7},
                                      {"Event Capture Read Timestamp Register 8", ECRDTS8},
                                      {"Event Capture Read Timestamp Register 9", ECRDTS9},
                                      {"Event Capture Read Timestamp Register 10", ECRDTS10},
                                      {"Event Capture Read Timestamp Register 11", ECRDTS11},
                                      {"Event Capture Read Timestamp Register 12", ECRDTS12},
                                      {"Event Capture Read Timestamp Register 13", ECRDTS13},
                                      {"Event Capture Read Timestamp Register 14", ECRDTS14},
                                      {"Event Capture Read Timestamp Register 15", ECRDTS15},
                                      {"Phase Adjuster Cycles Register", PACYC},
                                      {"Phase Adjuster Control Register", PACTRL},
                                      {"Event 0 Start Time Nanoseconds Register", EG0STNS},
                                      {"Event 0 Start Time Seconds Low Register", EG0STSECL},
                                      {"Event 0 Start Time Seconds High Register", EG0STSECH},
                                      {"Event 0 Pulse Width Register", EG0PW},
                                      {"Event 0 Idle Time Register", EG0IT},
                                      {"Event Generator 0 Control Register", EG0CTL},
                                      {"Event 1 Start Time Nanoseconds Register", EG1STNS},
                                      {"Event 1 Start Time Seconds Low Register", EG1STSECL},
                                      {"Event 1 Start Time Seconds High Register", EG1STSECH},
                                      {"Event 1 Pulse Width Register", EG1PW},
                                      {"Event 1 Idle Time Register", EG1IT},
                                      {"Event Generator 1 Control Register", EG1CTL},
                                      {"Event 2 Start Time Nanoseconds Register", EG2STNS},
                                      {"Event 2 Start Time Seconds Low Register", EG2STSECL},
                                      {"Event 2 Start Time Seconds High Register", EG2STSECH},
                                      {"Event 2 Pulse Width Register", EG2PW},
                                      {"Event 2 Idle Time Register", EG2IT},
                                      {"Event Generator 2 Control Register", EG2CTL},
                                      {"Event 3 Start Time Nanoseconds Register", EG3STNS},
                                      {"Event 3 Start Time Seconds Low Register", EG3STSECL},
                                      {"Event 3 Start Time Seconds High Register", EG3STSECH},
                                      {"Event 3 Pulse Width Register", EG3PW},
                                      {"Event 3 Idle Time Register", EG3IT},
                                      {"Event Generator 3 Control Register", EG3CTL},
                                      {"One Pulse-per-Second Control Register", PPSCTL},
                                      {"Synchronization Event Interrupt Enable Register", SEVINTEN},
                                      {"Synchronization Event Interrupt Disable Register", SEVINTDIS},
                                      {"Synchronization Event Interrupt Mask Status Register", SEVIM},
                                      {"Synchronization Event Status Register", SEVSTS},
                                      {"", -1}};

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

    /* Read OA_STATUS0 */
    regval = read_register(MMS0, OA_STATUS0);

    /* Write 1 to RESETC bit of OA_STATUS0 */
    regval |= (1 << 6);
    write_register(MMS0, OA_STATUS0, regval);

    regval = read_register(MMS4, CDCTL0);
    write_register(MMS4, CDCTL0, regval | (1 << 15)); // Initial logic (disable collision detection)

    // PLCA Configuration based on mode
    // TODO: This process is temporary and assumes that there are only two nodes.
    // TODO: Should be changed to get node info. from the command line.
    if (mode == PLCA_MODE_COORDINATOR) {
        write_register(MMS4, PLCA_CTRL1, 0x00000800); // Coordinator(node 0), 2 nodes
        write_register(MMS1, MAC_SAB1, 0x11111111);   // Configure MAC Address (Temporary)
        write_register(MMS1, MAC_SAT1, 0x00001111);   // Configure MAC Address (Temporary)
    } else if (mode == PLCA_MODE_FOLLOWER) {
        write_register(MMS4, PLCA_CTRL1, 0x00000801); // Follower, node 1
       write_register(MMS1, MAC_SAB1, 0x22222222);   // Configure MAC Address (Temporary)
       write_register(MMS1, MAC_SAT1, 0x00002222);   // Configure MAC Address (Temporary)
    } else {
        printf("Invalid mode: %d\n", mode);
        return false;
    }
    set_macphy_register(); // AN_LAN865x-Configuration
#ifdef SQI
    set_sqi_register();
#endif

    write_register(MMS4, PLCA_CTRL0, 0x00008000); // Enable PLCA
    write_register(MMS1, MAC_NCFGR, 0x000000C0);  // Enable unicast, multicast
    write_register(MMS1, MAC_NCR, 0x0000000C);    // Enable MACPHY TX, RX
#if 0
    write_register(MMS0, OA_STATUS0, 0x00000040); // Clear RESETC
    write_register(MMS0, OA_CONFIG0, 0x00008006); // SYNC bit SET (last configuration)
#endif

    /* Read OA_CONFIG0 */
    regval = read_register(MMS0, OA_CONFIG0);

    /* Set SYNC bit of OA_CONFIG0 */
    regval |= (1 << 15);
    write_register(MMS0, OA_CONFIG0, regval);

    /* Read OA_STATUS0 */
    regval = read_register(MMS0, OA_STATUS0);

    /* Clear RESETC bit of OA_STATUS0 */
    regval &= ~(1UL << 6);
    write_register(MMS0, OA_STATUS0, regval);

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
    spi_transfer(rxbuffer, txbuffer, numberof_bytestosend);

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
    spi_transfer(rxbuffer, txbuffer, numberof_bytestosend);

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

uint32_t read_register(uint8_t mms, uint16_t address) {
    bool execution_status = false;
    struct ctrl_cmd_reg readreg_infoinput;
    struct ctrl_cmd_reg readreg_data;
    readreg_infoinput.memorymap = mms;
    readreg_infoinput.length = 0;
    readreg_infoinput.address = address;

    execution_status = t1s_hw_readreg(&readreg_infoinput, &readreg_data);
    if (execution_status == true) {
        return readreg_data.databuffer[0];
    } else {
        printf("ERROR: Register Read failed at MMS %d, Address %4x\n", mms, address);
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

static void dump_reginfo(uint8_t mms, struct reginfo* reginfo) {

    for (int i = 0; reginfo[i].address >= 0; i++) {
        printf("address: 0x%04x - value: 0x%08x - %s\n", reginfo[i].address,
               read_register(mms, (uint16_t)reginfo[i].address), reginfo[i].desc);
    }
}

/* read all register values in memory map selector */
static int read_register_in_mms(uint8_t mms) {
    switch (mms) {
    case MMS0: /* Open Alliance 10BASE-T1x MAC-PHY Standard Registers */
        dump_reginfo(mms, reg_open_alliance);
        break;
    case MMS1: /* MAC Registers */
        dump_reginfo(mms, reg_mac);
        break;
    case MMS2: /* PHY PCS Registers */
        dump_reginfo(mms, reg_phy_pcs);
        break;
    case MMS3: /* PHY PMA/PMD Registers */
        dump_reginfo(mms, reg_phy_pma_pmd);
        break;
    case MMS4: /* PHY Vendor Specific Registers */
        dump_reginfo(mms, reg_phy_vendor_specific);
        break;
    case MMS10: /* Miscellaneous Register Descriptions */
        dump_reginfo(mms, reg_miscellaneous);
        break;
    default:
        printf("%s - Unknown memory map selector(0x%02x)\n", __func__, mms);
        return -1;
    }

    return 0;
}

static int set_register_value(uint8_t mms, struct reginfo* reginfo, int32_t addr, uint32_t data) {

    uint32_t pre_val;
    for (int i = 0; reginfo[i].address >= 0; i++) {
        if (reginfo[i].address == addr) {
            pre_val = read_register(mms, (uint16_t)reginfo[i].address);
            write_register(mms, (uint16_t)addr, data);
            printf("address: 0x%04x - pre-value: 0x%08x - cur-value: 0x%08x - %s\n", reginfo[i].address, pre_val,
                   read_register(mms, (uint16_t)reginfo[i].address), reginfo[i].desc);
            return 0;
        }
    }
    return -1;
}

/* read all register values in memory map selector */
static int write_register_in_mms(uint8_t mms, int32_t addr, uint32_t data) {
    switch (mms) {
    case MMS0: /* Open Alliance 10BASE-T1x MAC-PHY Standard Registers */
        return set_register_value(mms, reg_open_alliance, addr, data);
    case MMS1: /* MAC Registers */
        return set_register_value(mms, reg_mac, addr, data);
    case MMS2: /* PHY PCS Registers */
        return set_register_value(mms, reg_phy_pcs, addr, data);
    case MMS3: /* PHY PMA/PMD Registers */
        return set_register_value(mms, reg_phy_pma_pmd, addr, data);
    case MMS4: /* PHY Vendor Specific Registers */
        return set_register_value(mms, reg_phy_vendor_specific, addr, data);
    case MMS10: /* Miscellaneous Register Descriptions */
        return set_register_value(mms, reg_miscellaneous, addr, data);
    default:
        printf("%s - Unknown memory map selector(0x%02x)\n", __func__, mms);
        return -1;
    }
}

static int configure_plca_to_mac_phy() {
    uint32_t regval;

    /* Initial logic (disable collision detection) */
    regval = read_register(MMS4, CDCTL0);
    write_register(MMS4, CDCTL0, regval | (1 << 15));

    /* AN_LAN865x-Configuration */
    set_macphy_register();
#ifdef SQI
    set_sqi_register();
#endif

    write_register(MMS4, PLCA_CTRL0, 0x00008000); /* Enable PLCA */
    write_register(MMS1, MAC_NCFGR, 0x000000C0);  /* Enable unicast, multicast */
    write_register(MMS1, MAC_NCR, 0x0000000C);    /* Enable MACPHY TX, RX */
    write_register(MMS0, OA_STATUS0, 0x00000040); /* Clear RESETC */
    write_register(MMS0, OA_CONFIG0, 0x00008006); /* SYNC bit SET (last configuration) */

    return 0;
}

#define BOARD_MAC_SPECIFIC_ID 1 /* 1 to 4 */

static int set_mac_address(uint64_t mac, int id, int filter_mask, int filter_type) {
    uint32_t bottom;
    uint32_t top;

    bottom = (uint32_t)(mac & 0xffffffff);
    top = (uint32_t)(((mac >> 32) & 0xffff) + ((filter_type & 0x1) << 16) + ((filter_mask & 0x3F) << 24));

    switch (id) {
    case 1:
        write_register(MMS1, MAC_SAB1, bottom);
        write_register(MMS1, MAC_SAT1, top);
        break;
    case 2:
        write_register(MMS1, MAC_SAB2, bottom);
        write_register(MMS1, MAC_SAT2, top);
        break;
    case 3:
        write_register(MMS1, MAC_SAB3, bottom);
        write_register(MMS1, MAC_SAT3, top);
        break;
    case 4:
        write_register(MMS1, MAC_SAB4, bottom);
        write_register(MMS1, MAC_SAT4, top);
        break;
    }
    return 0;
}

static uint64_t get_mac_address(int id) {
    uint32_t bottom = 0;
    uint32_t top = 0;
    uint64_t mac;

    switch (id) {
    case 1:
        bottom = read_register(MMS1, MAC_SAB1);
        top = read_register(MMS1, MAC_SAT1);
        break;
    case 2:
        bottom = read_register(MMS1, MAC_SAB2);
        top = read_register(MMS1, MAC_SAT2);
        break;
    case 3:
        bottom = read_register(MMS1, MAC_SAB3);
        top = read_register(MMS1, MAC_SAT3);
        break;
    case 4:
        bottom = read_register(MMS1, MAC_SAB4);
        top = read_register(MMS1, MAC_SAT4);
        break;
    }

    mac = top & 0xffff;
    mac = (mac << 32) | bottom;

    return mac;
}

static int set_node_config(int node_id, int node_cnt) {
    uint32_t regval;

    regval = (uint32_t)((node_cnt & 0xFF) << 8) + (node_id & 0xFF);
    write_register(MMS4, PLCA_CTRL1, regval); /* PLCA Control 1 Register */

    return 0;
}

int api_read_register_in_mms(int mms) {
    return read_register_in_mms((uint8_t)mms);
}

int api_write_register_in_mms(int mms, int addr, uint32_t data) {
    return write_register_in_mms((uint8_t)mms, (int32_t)addr, (uint32_t)data);
}

uint64_t api_get_mac_address() {
    return get_mac_address((int)BOARD_MAC_SPECIFIC_ID);
}

int api_configure_plca_to_mac_phy() {
    return configure_plca_to_mac_phy();
}

int api_config_mac_address(uint64_t mac) {
    return set_mac_address(mac, BOARD_MAC_SPECIFIC_ID, 0, 0);
}

int api_config_node(int node_id, int node_cnt) {
    return set_node_config(node_id, node_cnt);
}
