#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <10baset1s/lan865x.h>
#include <10baset1s/rpi_spi.h>
#include <10baset1s/xbaset1s_arch.h>
#include <arpa/inet.h>

#include "lan865x-micron.h"

struct reg_info reg_open_alliance[] = {{"Identification Register", MMS0_OA_ID},
                                       {"PHY Identification Register", MMS0_OA_PHYID},
                                       {"Standard Capabilities", MMS0_OA_STDCAP},
                                       {"Reset Control and Status Register", MMS0_OA_RESET},
                                       {"Configuration 0 Register", MMS0_OA_CONFIG0},
                                       {"Status 0 Register", MMS0_OA_STATUS0},
                                       {"Status 1 Register", MMS0_OA_STATUS1},
                                       {"Buffer Status Register", MMS0_OA_BUFSTS},
                                       {"Interrupt Mask 0 Register", MMS0_OA_IMASK0},
                                       {"Interrupt Mask 1 Register", MMS0_OA_IMASK1},
                                       {"Transmit Timestamp Capture A (High)", MMS0_TTSCAH},
                                       {"Transmit Timestamp Capture A (Low)", MMS0_TTSCAL},
                                       {"Transmit Timestamp Capture B (High)", MMS0_TTSCBH},
                                       {"Transmit Timestamp Capture B (Low)", MMS0_TTSCBL},
                                       {"Transmit Timestamp Capture C (High)", MMS0_TTSCCH},
                                       {"Transmit Timestamp Capture C (Low)", MMS0_TTSCCL},
                                       {"Basic Control", MMS0_BASIC_CONTROL},
                                       {"Basic Status", MMS0_BASIC_STATUS},
                                       {"PHY Identifier 1 Register", MMS0_PHY_ID1},
                                       {"PHY Identifier 2 Register", MMS0_PHY_ID2},
                                       {"MMD Access Control Register", MMS0_MMDCTRL},
                                       {"MMD Access Address/Data Register", MMS0_MMDAD},
                                       {"", -1}};

struct reg_info reg_mac[] = {{"Network Control Register", MMS1_MAC_NCR},
                             {"Network Configuration Register", MMS1_MAC_NCFGR},
                             {"Hash Register Bottom", MMS1_MAC_HRB},
                             {"Hash Register Top", MMS1_MAC_HRT},
                             {"Specific Address 1 Bottom", MMS1_MAC_SAB1},
                             {"Specific Address 1 Top", MMS1_MAC_SAT1},
                             {"Specific Address 2 Bottom", MMS1_MAC_SAB2},
                             {"Specific Address 2 Top", MMS1_MAC_SAT2},
                             {"Specific Address 3 Bottom", MMS1_MAC_SAB3},
                             {"Specific Address 3 Top", MMS1_MAC_SAT3},
                             {"Specific Address 4 Bottom", MMS1_MAC_SAB4},
                             {"Specific Address 4 Top", MMS1_MAC_SAT4},
                             {"MAC Type ID Match 1", MMS1_MAC_TIDM1},
                             {"MAC Type ID Match 2", MMS1_MAC_TIDM2},
                             {"MAC Type ID Match 3", MMS1_MAC_TIDM3},
                             {"MAC Type ID Match 4", MMS1_MAC_TIDM4},
                             {"Specific Address Match 1 Bottom", MMS1_MAC_SAMB1},
                             {"Specific Address Match 1 Top", MMS1_MAC_SAMT1},
                             {"Timer Increment Sub-Nanoseconds", MMS1_MAC_TISUBN},
                             {"Timestamp Seconds High", MMS1_MAC_TSH},
                             {"Timestamp Seconds Low", MMS1_MAC_TSL},
                             {"Timestamp Nanoseconds", MMS1_MAC_TN},
                             {"TSU Timer Adjust", MMS1_MAC_TA},
                             {"TSU Timer Increment", MMS1_MAC_TI},
                             {"Buffer Manager Control", MMS1_BMGR_CTL},
                             {"Statistics 0", MMS1_STATS0},
                             {"Statistics 1", MMS1_STATS1},
                             {"Statistics 2", MMS1_STATS2},
                             {"Statistics 3", MMS1_STATS3},
                             {"Statistics 4", MMS1_STATS4},
                             {"Statistics 5", MMS1_STATS5},
                             {"Statistics 6", MMS1_STATS6},
                             {"Statistics 7", MMS1_STATS7},
                             {"Statistics 8", MMS1_STATS8},
                             {"Statistics 9", MMS1_STATS9},
                             {"Statistics 10", MMS1_STATS10},
                             {"Statistics 11", MMS1_STATS11},
                             {"Statistics 12", MMS1_STATS12},
                             {"", -1}};

struct reg_info reg_phy_pcs[] = {{"10BASE-T1S PCS Control", MMS2_T1SPCSCTL},
                                 {"10BASE-T1S PCS Status", MMS2_T1SPCSSTS},
                                 {"10BASE-T1S PCS Diagnostic 1", MMS2_T1SPCSDIAG1},
                                 {"10BASE-T1S PCS Diagnostic 2", MMS2_T1SPCSDIAG2},
                                 {"", -1}};

struct reg_info reg_phy_pma_pmd[] = {{"BASE-T1 PMA/PMD Extended Ability", MMS3_T1PMAPMDEXTA},
                                     {"BASE-T1 PMA/PMD Control", MMS3_T1PMAPMDCTL},
                                     {"10BASE-T1S PMA Control", MMS3_T1SPMACTL},
                                     {"10BASE-T1S PMA Status", MMS3_T1SPMASTS},
                                     {"10BASE-T1S Test Mode Control", MMS3_T1STSTCTL},
                                     {"", -1}};

struct reg_info reg_phy_vendor_specific[] = {{"Control 1 Register", MMS4_CTRL1},
                                             {"Status 1 Register", MMS4_STS1},
                                             {"Status 2 Register", MMS4_STS2},
                                             {"Status 3 Register", MMS4_STS3},
                                             {"Interrupt Mask 1 Register", MMS4_IMSK1},
                                             {"Interrupt Mask 2 Register", MMS4_IMSK2},
                                             {"Counter Control Register", MMS4_CTRCTRL},
                                             {"Transmit Opportunity Count (High)", MMS4_TOCNTH},
                                             {"Transmit Opportunity Count (Low)", MMS4_TOCNTL},
                                             {"BEACON Count (High)", MMS4_BCNCNTH},
                                             {"BEACON Count (Low)", MMS4_BCNCNTL},
                                             {"PLCA Multiple ID 0 Register", MMS4_MULTID0},
                                             {"PLCA Multiple ID 1 Register", MMS4_MULTID1},
                                             {"PLCA Multiple ID 2 Register", MMS4_MULTID2},
                                             {"PLCA Multiple ID 3 Register", MMS4_MULTID3},
                                             {"PLCA Reconciliation Sublayer Status", MMS4_PRSSTS},
                                             {"Port Management 2", MMS4_PRTMGMT2},
                                             {"Inactivity Watchdog Timeout (High)", MMS4_IWDTOH},
                                             {"Inactivity Watchdog Timeout (Low)", MMS4_IWDTOL},
                                             {"Transmit Match Control Register", MMS4_TXMCTL},
                                             {"Transmit Match Pattern (High) Register", MMS4_TXMPATH},
                                             {"Transmit Match Pattern (Low) Register", MMS4_TXMPATL},
                                             {"Transmit Match Mask (High) Register", MMS4_TXMMSKH},
                                             {"Transmit Match Mask (Low) Register", MMS4_TXMMSKL},
                                             {"Transmit Match Location Register", MMS4_TXMLOC},
                                             {"Transmit Matched Packet Delay Register", MMS4_TXMDLY},
                                             {"Receive Match Control Register", MMS4_RXMCTL},
                                             {"Receive Match Pattern (High) Register", MMS4_RXMPATH},
                                             {"Receive Match Pattern (Low) Register", MMS4_RXMPATL},
                                             {"Receive Match Mask (High) Register", MMS4_RXMMSKH},
                                             {"Receive Match Mask (Low) Register", MMS4_RXMMSKL},
                                             {"Receive Match Location Register", MMS4_RXMLOC},
                                             {"Receive Matched Packet Delay Register", MMS4_RXMDLY},
                                             {"Credit Based Shaper Stop Threshold (High) Register", MMS4_CBSSPTHH},
                                             {"Credit Based Shaper Stop Threshold (Low) Register", MMS4_CBSSPTHL},
                                             {"Credit Based Shaper Start Threshold (High) Register", MMS4_CBSSTTHH},
                                             {"Credit Based Shaper Start Threshold (Low) Register", MMS4_CBSSTTHL},
                                             {"Credit Based Shaper Slope Control Register", MMS4_CBSSLPCTL},
                                             {"Credit Based Shaper Top Limit (High) Register", MMS4_CBSTPLMTH},
                                             {"Credit Based Shaper Top Limit (Low) Register", MMS4_CBSTPLMTL},
                                             {"Credit Based Shaper Bottom Limit (High) Register", MMS4_CBSBTLMTH},
                                             {"Credit Based Shaper Bottom Limit (Low) Register", MMS4_CBSBTLMTL},
                                             {"Credit Based Shaper Credit Counter (High) Register", MMS4_CBSCRCTRH},
                                             {"Credit Based Shaper Credit Counter (Low) Register", MMS4_CBSCRCTRL},
                                             {"Credit Based Shaper Control Register", MMS4_CBSCTRL},
                                             {"PLCA Skip Control Register", MMS4_PLCASKPCTL},
                                             {"PLCA Transmit Opportunity Skip Register", MMS4_PLCATOSKP},
                                             {"Application Controlled Media Access Control Register", MMS4_ACMACTL},
                                             {"Sleep Control 0 Register", MMS4_SLPCTL0},
                                             {"Sleep Control 1 Register", MMS4_SLPCTL1},
                                             {"Collision Detector Control 0 Register", MMS4_CDCTL0},
                                             {"SQI Control Register", MMS4_SQICTL},
                                             {"SQI Status 0 Register", MMS4_SQISTS0},
                                             {"SQI Configuration 0 Register", MMS4_SQICFG0},
                                             {"SQI Configuration 2 Register", MMS4_SQICFG2},
                                             {"Analog Control 5", MMS4_ANALOG5},
                                             {"Analog Control 5", MMS4_ANALOG5},
                                             {"OPEN Alliance Map ID and Version Register", MMS4_MIDVER},
                                             {"PLCA Control 0 Register", MMS4_PLCA_CTRL0},
                                             {"PLCA Control 1 Register", MMS4_PLCA_CTRL1},
                                             {"PLCA Status Register", MMS4_PLCA_STS},
                                             {"PLCA Transmit Opportunity Timer Register", MMS4_PLCA_TOTMR},
                                             {"PLCA Burst Mode Register", MMS4_PLCA_BURST},
                                             {"", -1}};

struct reg_info reg_miscellaneous[] = {{"Queue Transmit Configuration", MMS10_QTXCFG},
                                       {"Queue Receive Configuration", MMS10_QRXCFG},
                                       {"Pad Control", MMS10_PADCTRL},
                                       {"Clock Output Control", MMS10_CLKOCTL},
                                       {"Miscellaneous", MMS10_MISC},
                                       {"Device Identification", MMS10_DEVID},
                                       {"Bus Parity Control and Status", MMS10_BUSPCS},
                                       {"Configuration Protection Control", MMS10_CFGPRTCTL},
                                       {"SRAM Error Correction Code Control", MMS10_ECCCTRL},
                                       {"SRAM Error Correction Code Status", MMS10_ECCSTS},
                                       {"SRAM Error Correction Code Fault Injection Control", MMS10_ECCFLTCTRL},
                                       {"Event Capture 0 Control", MMS10_EC0CTRL},
                                       {"Event Capture 1 Control", MMS10_EC1CTRL},
                                       {"Event Capture 2 Control", MMS10_EC2CTRL},
                                       {"Event Capture 3 Control", MMS10_EC3CTRL},
                                       {"Event Capture Read Status Register", MMS10_ECRDSTS},
                                       {"Event Capture Total Counts Register", MMS10_ECTOT},
                                       {"Event Capture Clock Seconds High Register", MMS10_ECCLKSH},
                                       {"Event Capture Clock Seconds Low Register", MMS10_ECCLKSL},
                                       {"Event Capture Clock Nanoseconds Register", MMS10_ECCLKNS},
                                       {"Event Capture Read Timestamp Register 0", MMS10_ECRDTS0},
                                       {"Event Capture Read Timestamp Register 1", MMS10_ECRDTS1},
                                       {"Event Capture Read Timestamp Register 2", MMS10_ECRDTS2},
                                       {"Event Capture Read Timestamp Register 3", MMS10_ECRDTS3},
                                       {"Event Capture Read Timestamp Register 4", MMS10_ECRDTS4},
                                       {"Event Capture Read Timestamp Register 5", MMS10_ECRDTS5},
                                       {"Event Capture Read Timestamp Register 6", MMS10_ECRDTS6},
                                       {"Event Capture Read Timestamp Register 7", MMS10_ECRDTS7},
                                       {"Event Capture Read Timestamp Register 8", MMS10_ECRDTS8},
                                       {"Event Capture Read Timestamp Register 9", MMS10_ECRDTS9},
                                       {"Event Capture Read Timestamp Register 10", MMS10_ECRDTS10},
                                       {"Event Capture Read Timestamp Register 11", MMS10_ECRDTS11},
                                       {"Event Capture Read Timestamp Register 12", MMS10_ECRDTS12},
                                       {"Event Capture Read Timestamp Register 13", MMS10_ECRDTS13},
                                       {"Event Capture Read Timestamp Register 14", MMS10_ECRDTS14},
                                       {"Event Capture Read Timestamp Register 15", MMS10_ECRDTS15},
                                       {"Phase Adjuster Cycles Register", MMS10_PACYC},
                                       {"Phase Adjuster Control Register", MMS10_PACTRL},
                                       {"Event 0 Start Time Nanoseconds Register", MMS10_EG0STNS},
                                       {"Event 0 Start Time Seconds Low Register", MMS10_EG0STSECL},
                                       {"Event 0 Start Time Seconds High Register", MMS10_EG0STSECH},
                                       {"Event 0 Pulse Width Register", MMS10_EG0PW},
                                       {"Event 0 Idle Time Register", MMS10_EG0IT},
                                       {"Event Generator 0 Control Register", MMS10_EG0CTL},
                                       {"Event 1 Start Time Nanoseconds Register", MMS10_EG1STNS},
                                       {"Event 1 Start Time Seconds Low Register", MMS10_EG1STSECL},
                                       {"Event 1 Start Time Seconds High Register", MMS10_EG1STSECH},
                                       {"Event 1 Pulse Width Register", MMS10_EG1PW},
                                       {"Event 1 Idle Time Register", MMS10_EG1IT},
                                       {"Event Generator 1 Control Register", MMS10_EG1CTL},
                                       {"Event 2 Start Time Nanoseconds Register", MMS10_EG2STNS},
                                       {"Event 2 Start Time Seconds Low Register", MMS10_EG2STSECL},
                                       {"Event 2 Start Time Seconds High Register", MMS10_EG2STSECH},
                                       {"Event 2 Pulse Width Register", MMS10_EG2PW},
                                       {"Event 2 Idle Time Register", MMS10_EG2IT},
                                       {"Event Generator 2 Control Register", MMS10_EG2CTL},
                                       {"Event 3 Start Time Nanoseconds Register", MMS10_EG3STNS},
                                       {"Event 3 Start Time Seconds Low Register", MMS10_EG3STSECL},
                                       {"Event 3 Start Time Seconds High Register", MMS10_EG3STSECH},
                                       {"Event 3 Pulse Width Register", MMS10_EG3PW},
                                       {"Event 3 Idle Time Register", MMS10_EG3IT},
                                       {"Event Generator 3 Control Register", MMS10_EG3CTL},
                                       {"One Pulse-per-Second Control Register", MMS10_PPSCTL},
                                       {"Synchronization Event Interrupt Enable Register", MMS10_SEVINTEN},
                                       {"Synchronization Event Interrupt Disable Register", MMS10_SEVINTDIS},
                                       {"Synchronization Event Interrupt Mask Status Register", MMS10_SEVIM},
                                       {"Synchronization Event Status Register", MMS10_SEVSTS},
                                       {"", -1}};

uint8_t get_parity(uint32_t header) {
    header ^= header >> 1;
    header ^= header >> 2;
    header = ((header & LAN865X_PARITY_BIT_MASK) * LAN865X_PARITY_BIT_MASK);
    return ((header >> LAN865X_PARITY_SHIFT_BITS) & 1);
}

static int lan865x_t1s_read_reg(unsigned int handle, struct ctrl_cmd_reg* read_info, struct ctrl_cmd_reg* read_data) {
    uint8_t ignored_echoedbytes = HEADER_SIZE;
    uint8_t tx_buf[MAX_REG_DATA_ONECONTROLCMD + HEADER_SIZE + REG_SIZE] = {0};
    uint8_t rx_buf[MAX_REG_DATA_ONECONTROLCMD + HEADER_SIZE + REG_SIZE] = {0};
    uint32_t numberof_bytestosend = 0;
    union ctrl_header cmd_hdr;
    union ctrl_header echo_cmd_hdr;
    uint32_t cvrt_cmd_hdr;

    cmd_hdr.ctrl_frame = 0;
    echo_cmd_hdr.ctrl_frame = 0;

    cmd_hdr.ctrl_bits.dnc = DNC_FLAG_CONTROL;
    cmd_hdr.ctrl_bits.hdrb = 0;           /* Header Bad */
    cmd_hdr.ctrl_bits.wnr = WNR_BIT_READ; /* Write, Not Read */
    if (read_info->length != 0) {
        cmd_hdr.ctrl_bits.aid = AID_FALSE; /* Address Increment Disable */
    } else {
        cmd_hdr.ctrl_bits.aid = AID_TRUE; /* Address Increment Disable */
    }
    cmd_hdr.ctrl_bits.mms = (uint32_t)read_info->mms;
    cmd_hdr.ctrl_bits.addr = (uint32_t)read_info->addr;
    cmd_hdr.ctrl_bits.len = (uint32_t)(read_info->length & MAX_CONTROL_CMD_LEN);
    cmd_hdr.ctrl_bits.p = ((get_parity(cmd_hdr.ctrl_frame) == 0) ? 1 : 0);

    cvrt_cmd_hdr = htonl(cmd_hdr.ctrl_frame);
    memcpy(tx_buf, &cvrt_cmd_hdr, HEADER_SIZE);

    numberof_bytestosend = HEADER_SIZE + ((cmd_hdr.ctrl_bits.len + 1U) * REG_SIZE) +
                           ignored_echoedbytes; /* Added extra 4 bytes because first 4 bytes during
                                                    reception shall be ignored */
    spi_transfer(handle, rx_buf, tx_buf, numberof_bytestosend);

    memcpy((uint8_t*)&echo_cmd_hdr.ctrl_frame, &rx_buf[ignored_echoedbytes], HEADER_SIZE);
    echo_cmd_hdr.ctrl_frame = ntohl(echo_cmd_hdr.ctrl_frame);

    /**
     * HDRB: Header Bad - Indication from the LAN8650/1 to the SPI host that the MAC-PHY received a transaction header
     * with an invalid parity. When sent to the LAN8650/1 by the SPI host, the value of this bit is ignored by the
     * LAN8650/1.
     */
    if (echo_cmd_hdr.ctrl_bits.hdrb != (uint32_t)0) { /* An invalid parity */
        printf("Parity Error READMACPHYReg header value : 0x%08x\n", echo_cmd_hdr.ctrl_frame);
        return -RET_FAIL;
    }

    /* A valid parity */
    if (cmd_hdr.ctrl_bits.len == (uint32_t)0) {
        memcpy((uint8_t*)&read_data->buffer[0], &(rx_buf[ignored_echoedbytes + HEADER_SIZE]), REG_SIZE);
        read_data->buffer[0] = ntohl(read_data->buffer[0]);
    } else {
        for (uint32_t count = 0; count <= cmd_hdr.ctrl_bits.len; count++) {
            memcpy((uint8_t*)&read_data->buffer[REG_SIZE * count],
                   &rx_buf[ignored_echoedbytes + HEADER_SIZE + (REG_SIZE * count)], REG_SIZE);
            read_data->buffer[REG_SIZE * count] = ntohl(read_data->buffer[REG_SIZE * count]);
        }
    }

    return RET_SUCCESS;
}

uint32_t read_register(unsigned int handle, uint8_t mms, uint16_t addr) {
    int ret;
    struct ctrl_cmd_reg read_info;
    struct ctrl_cmd_reg read_data;

    read_info.mms = mms;
    read_info.length = 0;
    read_info.addr = addr;

    ret = lan865x_t1s_read_reg(handle, &read_info, &read_data);
    if (ret) {
        printf("ERROR: Register Read failed at MMS %d, Address %4x\n", mms, addr);
        return 0;
    }

    return read_data.buffer[0];
}

static int lan865x_t1s_write_reg(unsigned int handle, struct ctrl_cmd_reg* write_info) {
    uint32_t numberof_bytestosend = 0;
    uint8_t ignored_echoedbytes = HEADER_SIZE;
    uint8_t tx_buf[MAX_PAYLOAD_BYTE + HEADER_SIZE] = {0};
    uint8_t rx_buf[MAX_PAYLOAD_BYTE + HEADER_SIZE] = {0};
    union ctrl_header cmd_header;
    union ctrl_header echo_cmd_header;
    uint32_t cvrt_cmd_header;

    cmd_header.ctrl_frame = echo_cmd_header.ctrl_frame = 0;

    cmd_header.ctrl_bits.dnc = DNC_FLAG_CONTROL;
    cmd_header.ctrl_bits.hdrb = 0;            /* Header Bad */
    cmd_header.ctrl_bits.wnr = WNR_BIT_WRITE; /* Write, Not Read */
    if (write_info->length != 0) {
        cmd_header.ctrl_bits.aid = AID_FALSE; /* Address Increment Disable: False */
    } else {
        cmd_header.ctrl_bits.aid = AID_TRUE; /* Address Increment Disable: True */
    }
    cmd_header.ctrl_bits.mms = (uint32_t)(write_info->mms & LAN865X_MMS_MASK);
    cmd_header.ctrl_bits.addr = (uint32_t)write_info->addr;
    cmd_header.ctrl_bits.len = (uint32_t)(write_info->length & MAX_CONTROL_CMD_LEN);
    cmd_header.ctrl_bits.p = ((get_parity(cmd_header.ctrl_frame) == 0) ? 1 : 0);

    cvrt_cmd_header = htonl(cmd_header.ctrl_frame);
    memcpy(tx_buf, &cvrt_cmd_header, HEADER_SIZE);

    for (uint32_t reg_idx = 0; reg_idx < (cmd_header.ctrl_bits.len + 1U); reg_idx++) {
        uint32_t data = htonl(write_info->buffer[reg_idx * REG_SIZE]);
        memcpy(&tx_buf[HEADER_SIZE + reg_idx * REG_SIZE], &data, sizeof(uint32_t));
    }

    numberof_bytestosend =
        HEADER_SIZE + (cmd_header.ctrl_bits.len + 1U) * REG_SIZE +
        ignored_echoedbytes; /* Added extra 4 bytes because last 4 bytes of payload will be ignored by MACPHY */
    spi_transfer(handle, rx_buf, tx_buf, numberof_bytestosend);

    return RET_SUCCESS;
}

int32_t write_register(unsigned int handle, uint8_t mms, uint16_t addr, uint32_t val) {
    struct ctrl_cmd_reg write_info;
    int ret;

    write_info.mms = mms;
    write_info.length = 0;
    write_info.addr = addr;
    write_info.buffer[0] = val;

    ret = lan865x_t1s_write_reg(handle, &write_info);
    if (ret == RET_SUCCESS) {
        return RET_SUCCESS;
    }

    printf("ERROR: Register Write failed at MMS %d, Address %4x, Value %4x\n", mms, addr, val);
    return -RET_FAIL;
}

uint8_t indirect_read(unsigned int handle, uint8_t addr, uint8_t mask) {

    write_register(handle, MMS4, MMS4_INDIR_RD_ADDR, addr);
    write_register(handle, MMS4, MMS4_INDIR_RD_WIDTH, MMS04_INDIR_WIDTH);
    return (read_register(handle, MMS4, MMS4_INDIR_RD_VAL) & mask);
}

/* This function is to set the MACPHY register as guided by AN_LAN865x-Configuration
 * In addition to configuring the PHY transceiver in the device, the following configuration
 * configures the MAC to :
 * 1. Set time stamping at the end of the Start of Frame Delimiter (SFD)
 * 2. Set the Timer increment register to 40 ns to be used as a 25MHz internal clock
 */
static void set_macphy_register(unsigned int handle) {
    uint8_t value1 = indirect_read(handle, VALUE1_ADDR, VALUE_MASK);
    int8_t offset1;
    if ((value1 & VALUE_SIGN_MASK) != 0) {
        offset1 = (int8_t)((uint8_t)value1 - MAX_VALUE_5BITS);
    } else {
        offset1 = (int8_t)value1;
    }

    uint8_t value2 = indirect_read(handle, VALUE2_ADDR, VALUE_MASK);
    int8_t offset2;
    if ((value2 & VALUE_SIGN_MASK) != 0) {
        offset2 = (int8_t)((uint8_t)value2 - MAX_VALUE_5BITS);
    } else {
        offset2 = (int8_t)value2;
    }

    uint16_t cfgparam1 = (uint16_t)(((VALUE1_OFFSET1 + offset1) & VALUE_OFFSET_MASK) << VALUE1_SHIFT1) |
                         (uint16_t)(((VALUE1_OFFSET2 + offset1) & VALUE_OFFSET_MASK) << VALUE1_SHIFT2) |
                         VALUE1_LOWEST_VAL;
    uint16_t cfgparam2 = (uint16_t)(((VALUE2_OFFSET + offset2) & VALUE_OFFSET_MASK) << VALUE2_SHIFT);

    write_register(handle, MMS4, MMS4_A_00D0, MMS4_A_00D0_V);
    write_register(handle, MMS4, MMS4_A_00E0, MMS4_A_00E0_V);
    write_register(handle, MMS4, MMS4_A_0084, cfgparam1);
    write_register(handle, MMS4, MMS4_A_008A, cfgparam2);
    write_register(handle, MMS4, MMS4_A_00E9, MMS4_A_00E9_V);
    write_register(handle, MMS4, MMS4_A_00F5, MMS4_A_00F5_V);
    write_register(handle, MMS4, MMS4_A_00F4, MMS4_A_00F4_V);
    write_register(handle, MMS4, MMS4_A_00F8, MMS4_A_00F8_V);
    write_register(handle, MMS4, MMS4_A_00F9, MMS4_A_00F9_V);
    write_register(handle, MMS4, MMS4_SLPCTL1, MMS4_A_0081_V);
    write_register(handle, MMS4, MMS4_A_0091, MMS4_A_0091_V);
    write_register(handle, MMS4, MMS4_A_0077, MMS4_A_0077_V);
    write_register(handle, MMS4, MMS4_TXMMSKH, MMS4_A_0043_V);
    write_register(handle, MMS4, MMS4_TXMMSKL, MMS4_A_0044_V);
    write_register(handle, MMS4, MMS4_TXMLOC, MMS4_A_0045_V);
    write_register(handle, MMS4, MMS4_RXMMSKH, MMS4_A_0053_V);
    write_register(handle, MMS4, MMS4_RXMMSKL, MMS4_A_0054_V);
    write_register(handle, MMS4, MMS4_RXMLOC, MMS4_A_0055_V);
    write_register(handle, MMS4, MMS4_TXMCTL, MMS4_A_0040_V);
    write_register(handle, MMS4, MMS4_RXMCTL, MMS4_A_0050_V);
}

#ifdef SQI
/* This function is to set the SQI(Signal Quality Indicator) register as guided by AN_LAN865x-Configuration
 * SQI should be defined in order to use this function
 * See Datasheet for more details
 * This function is not tested.
 */
static void set_sqi_register(unsigned int handle) {
    uint8_t value1 = read_register(handle, VALUE1_ADDR, VALUE_MASK);
    int8_t offset1;
    if ((value1 & VALUE_SIGN_MASK) != 0) {
        offset1 = (int8_t)((uint8_t)value1 - MAX_VALUE_5BITS);
    } else {
        offset1 = (int8_t)value1;
    }

    uint16_t cfgparam3 = (uint16_t)(((SQI_PARA3_OFFSET1 + offset1) & VALUE_OFFSET_MASK) << SQI_BYTE_WIDTH) |
                         (uint16_t)((SQI_PARA3_OFFSET2 + offset1) & VALUE_OFFSET_MASK);
    uint16_t cfgparam4 = (uint16_t)(((SQI_PARA4_OFFSET1 + offset1) & VALUE_OFFSET_MASK) << SQI_BYTE_WIDTH) |
                         (uint16_t)((SQI_PARA4_OFFSET2 + offset1) & VALUE_OFFSET_MASK);
    uint16_t cfgparam5 = (uint16_t)(((SQI_PARA5_OFFSET1 + offset1) & VALUE_OFFSET_MASK) << SQI_BYTE_WIDTH) |
                         (uint16_t)((SQI_PARA5_OFFSET2 + offset1) & VALUE_OFFSET_MASK);

    write_register(handle, MMS4, MMS4_A_00AD, cfgparam3);
    write_register(handle, MMS4, MMS4_A_00AE, cfgparam4);
    write_register(handle, MMS4, MMS4_A_00AF, cfgparam5);
    write_register(handle, MMS4, MMS4_A_00B0, MMS4_A_00B0_V);
    write_register(handle, MMS4, MMS4_A_00B1, MMS4_A_00B1_V);
    write_register(handle, MMS4, MMS4_A_00B2, MMS4_A_00B2_V);
    write_register(handle, MMS4, MMS4_A_00B3, MMS4_A_00B3_V);
    write_register(handle, MMS4, MMS4_A_00B4, MMS4_A_00B4_V);
    write_register(handle, MMS4, MMS4_A_00B5, MMS4_A_00B5_V);
    write_register(handle, MMS4, MMS4_A_00B6, MMS4_A_00B6_V);
    write_register(handle, MMS4, MMS4_A_00B7, MMS4_A_00B7_V);
    write_register(handle, MMS4, MMS4_A_00B8, MMS4_A_00B8_V);
    write_register(handle, MMS4, MMS4_A_00B9, MMS4_A_00B9_V);
    write_register(handle, MMS4, MMS4_A_00BA, MMS4_A_00BA_V);
    write_register(handle, MMS4, MMS4_A_00BB, MMS4_A_00BB_V);
}
#endif

void init_lan865x(unsigned int handle) {
    uint32_t regval;

    /* Read OA_STATUS0 */
    regval = read_register(handle, MMS0, MMS0_OA_STATUS0);

    /* Write 1 to RESETC bit of OA_STATUS0 */
    regval |= (1 << MMS0_OA_STATUS0_RESETC_SHIFT);
    write_register(handle, MMS0, MMS0_OA_STATUS0, regval);

    regval = read_register(handle, MMS4, MMS4_CDCTL0);
    write_register(handle, MMS4, MMS4_CDCTL0,
                   regval | (1 << MMS4_CDCTL0_CDEN_SHIFT)); /* Initial logic (disable collision detection) */

    set_macphy_register(handle); /* AN_LAN865x-Configuration */

#ifdef SQI
    set_sqi_register(handle);
#endif

    write_register(handle, MMS4, MMS4_PLCA_CTRL0, MMS4_PLCA_CTRL0_INIT_VAL); /* Enable PLCA */
    write_register(handle, MMS1, MMS1_MAC_NCFGR, MMS1_MAC_NCFGR_INIT_VAL);   /* Enable unicast, multicast */
    write_register(handle, MMS1, MMS1_MAC_NCR, MMS1_MAC_NCR_INIT_VAL);       /* Enable MACPHY TX, RX */

#ifdef FRAME_TIMESTAMP_ENABLE
    write_register(handle, MMS1, MMS1_MAC_TI, TIMER_INCREMENT); /* Enable MACPHY TX, RX */
#endif

    /* Read OA_CONFIG0 */
    regval = read_register(handle, MMS0, MMS0_OA_CONFIG0);

    /* Set SYNC bit of OA_CONFIG0 */
    regval |= (1 << MMS0_OA_CONFIG0_SYNC_SHIFT);
#ifdef FRAME_TIMESTAMP_ENABLE
    /* Set FTSE Frame Timestamp Enable bit of OA_CONFIG0 */
    regval |= (1 << MMS0_OA_CONFIG0_FTSE_SHIFT);

    /* Set FTSS Frame Timestamp Select bit of OA_CONFIG0 */
    regval |= (1 << MMS0_OA_CONFIG0_FTSS_SHIFT);
#endif
    write_register(handle, MMS0, MMS0_OA_CONFIG0, regval);

    /* Read OA_STATUS0 */
    regval = read_register(handle, MMS0, MMS0_OA_STATUS0);

    /* Clear RESETC bit of OA_STATUS0 */
    regval &= ~(1UL << MMS0_OA_STATUS0_RESETC_SHIFT);
    write_register(handle, MMS0, MMS0_OA_STATUS0, regval);
}

static void dump_reg_info(unsigned int handle, uint8_t mms, struct reg_info* regs) {

    for (int i = 0; regs[i].address >= 0; i++) {
        printf("address: 0x%04x - value: 0x%08x - %s\n", regs[i].address,
               read_register(handle, mms, (uint16_t)regs[i].address), regs[i].desc);
    }
}

int read_all_registers_in_mms(unsigned int handle, uint8_t mms) {

    switch (mms) {
    case MMS0: /* Open Alliance 10BASE-T1x MAC-PHY Standard Registers */
        dump_reg_info(handle, mms, reg_open_alliance);
        break;
    case MMS1: /* MAC Registers */
        dump_reg_info(handle, mms, reg_mac);
        break;
    case MMS2: /* PHY PCS Registers */
        dump_reg_info(handle, mms, reg_phy_pcs);
        break;
    case MMS3: /* PHY PMA/PMD Registers */
        dump_reg_info(handle, mms, reg_phy_pma_pmd);
        break;
    case MMS4: /* PHY Vendor Specific Registers */
        dump_reg_info(handle, mms, reg_phy_vendor_specific);
        break;
    case MMS10: /* Miscellaneous Register Descriptions */
        dump_reg_info(handle, mms, reg_miscellaneous);
        break;
    default:
        printf("%s - Unknown memory map selector(0x%02x)\n", __func__, mms);
        return -RET_FAIL;
    }

    return RET_SUCCESS;
}

static int set_register_value(unsigned int handle, uint8_t mms, struct reg_info* regs, int32_t addr, uint32_t data) {

    uint32_t pre_val;
    for (int i = 0; regs[i].address >= 0; i++) {
        if (regs[i].address == addr) {
            pre_val = read_register(handle, mms, (uint16_t)regs[i].address);
            write_register(handle, mms, (uint16_t)addr, data);
            printf("address: 0x%04x - pre-value: 0x%08x - cur-value: 0x%08x - %s\n", regs[i].address, pre_val,
                   read_register(handle, mms, (uint16_t)regs[i].address), regs[i].desc);
            return RET_SUCCESS;
        }
    }
    return -RET_FAIL;
}

int write_register_in_mms(unsigned int handle, uint8_t mms, int32_t addr, uint32_t data) {
    switch (mms) {
    case MMS0: /* Open Alliance 10BASE-T1x MAC-PHY Standard Registers */
        return set_register_value(handle, mms, reg_open_alliance, addr, data);
    case MMS1: /* MAC Registers */
        return set_register_value(handle, mms, reg_mac, addr, data);
    case MMS2: /* PHY PCS Registers */
        return set_register_value(handle, mms, reg_phy_pcs, addr, data);
    case MMS3: /* PHY PMA/PMD Registers */
        return set_register_value(handle, mms, reg_phy_pma_pmd, addr, data);
    case MMS4: /* PHY Vendor Specific Registers */
        return set_register_value(handle, mms, reg_phy_vendor_specific, addr, data);
    case MMS10: /* Miscellaneous Register Descriptions */
        return set_register_value(handle, mms, reg_miscellaneous, addr, data);
    default:
        printf("%s - Unknown memory map selector(0x%02x)\n", __func__, mms);
        return -RET_FAIL;
    }
}

static uint64_t get_mac_address(unsigned int handle, int id) {
    uint32_t bottom = 0;
    uint32_t top = 0;
    uint64_t mac;

    switch (id) {
    case 1:
        bottom = read_register(handle, MMS1, MMS1_MAC_SAB1);
        top = read_register(handle, MMS1, MMS1_MAC_SAT1);
        break;
    case 2:
        bottom = read_register(handle, MMS1, MMS1_MAC_SAB2);
        top = read_register(handle, MMS1, MMS1_MAC_SAT2);
        break;
    case 3:
        bottom = read_register(handle, MMS1, MMS1_MAC_SAB3);
        top = read_register(handle, MMS1, MMS1_MAC_SAT3);
        break;
    case 4:
        bottom = read_register(handle, MMS1, MMS1_MAC_SAB4);
        top = read_register(handle, MMS1, MMS1_MAC_SAT4);
        break;
    }

    mac = top & MAC_TOP_REGISTER_MASK;
    mac = (mac << SQI_4_BYTES_WIDTH) | bottom;

    return mac;
}

uint64_t get_mac_address_on_chip(unsigned int handle) {
    return get_mac_address(handle, (int)BOARD_MAC_SPECIFIC_ID);
}

static int set_mac_address(unsigned int handle, uint64_t mac, int id, int filter_mask, int filter_type) {
    uint32_t bottom;
    uint32_t top;

    bottom = (uint32_t)(mac & MAC_BOTTOM_REGISTER_MASK);
    top = (uint32_t)(((mac >> MAC_BOTTOM_REGISTER_WIDTH) & MAC_TOP_REGISTER_MASK) +
                     ((filter_type & MAC_TOP_REGISTER_FLTTYP_MASK) << MAC_TOP_REGISTER_FLTTYP_SHIFT) +
                     ((filter_mask & MAC_TOP_REGISTER_FLTBM_MASK) << MAC_TOP_REGISTER_FLTBM_SHIFT));

    switch (id) {
    case 1:
        write_register(handle, MMS1, MMS1_MAC_SAB1, bottom);
        return write_register(handle, MMS1, MMS1_MAC_SAT1, top);
    case 2:
        write_register(handle, MMS1, MMS1_MAC_SAB2, bottom);
        return write_register(handle, MMS1, MMS1_MAC_SAT2, top);
    case 3:
        write_register(handle, MMS1, MMS1_MAC_SAB3, bottom);
        return write_register(handle, MMS1, MMS1_MAC_SAT3, top);
    case 4:
        write_register(handle, MMS1, MMS1_MAC_SAB4, bottom);
        return write_register(handle, MMS1, MMS1_MAC_SAT4, top);
    }
    return -RET_FAIL;
}

int set_mac_address_to_chip(unsigned int handle, uint64_t mac) {
    return set_mac_address(handle, mac, BOARD_MAC_SPECIFIC_ID, 0, 0);
}

int set_node_config(unsigned int handle, int node_id, int node_cnt) {
    uint32_t reg_val;

    reg_val =
        (uint32_t)((node_cnt & LAN865X_NODE_COUNT_MASK) << LAN865X_NODE_COUNT_SHIFT) + (node_id & LAN865X_NODE_ID_MASK);
    return write_register(handle, MMS4, MMS4_PLCA_CTRL1, reg_val); /* PLCA Control 1 Register */
}

#ifdef FRAME_TIMESTAMP_ENABLE
int transmit_timestamp_capture_available(unsigned int handle, uint32_t mask) {
    uint32_t reg_val;

    for (int try_cnt = 0; try_cnt < TIMESTAMP_CAPTURE_AVAILABLE_CHECK_COUNT; try_cnt++) {
        reg_val = read_register(handle, MMS0, MMS0_OA_STATUS0);
        if (reg_val & mask) {
            return RET_SUCCESS;
        }
    }
    return RET_FAIL;
}

int get_timestamp(unsigned int handle, int reg, struct timestamp_format* timestamp) {
    uint32_t reg_low;
    uint16_t addr;

    switch (reg & TRANSMIT_TIMESTAMP_CAPTURE_MASK) {
    case TTSC_A:
#if 1
        if (transmit_timestamp_capture_available(handle, TRANSMIT_TIMESTAMP_CAPTURE_AVAILABLE_MASK_A)) {
            printf("Not Available - Transmit Timestamp Capture\n");
//            return ERR_NOT_AVAILAVLE;
        }
#endif
        addr = MMS0_TTSCAL;
        break;
    case TTSC_B:
#if 1
        if (transmit_timestamp_capture_available(handle, TRANSMIT_TIMESTAMP_CAPTURE_AVAILABLE_MASK_B)) {
            printf("Not Available - Transmit Timestamp Capture\n");
//            return ERR_NOT_AVAILAVLE;
        }
#endif
        addr = MMS0_TTSCBL;
        break;
    case TTSC_C:
#if 1
        if (transmit_timestamp_capture_available(handle, TRANSMIT_TIMESTAMP_CAPTURE_AVAILABLE_MASK_C)) {
            printf("Not Available - Transmit Timestamp Capture\n");
//            return ERR_NOT_AVAILAVLE;
        }
#endif
        addr = MMS0_TTSCCL;
        break;
    default:
        printf("%s - %d - Unknown parameter(%d)\n", __func__, __LINE__, reg);
        return ERR_UNKNOWN_PARAMETER;
    }

    reg_low = read_register(handle, MMS0, addr);

    timestamp->nano.nanoseconds = reg_low & NANOSECONDS_MASK;
    timestamp->seconds = read_register(handle, MMS0, addr - 1);

    return RET_SUCCESS;
}

void print_timestamp_info(struct timestamp_format timestamp) {
#if 0
    uint64_t* ts;

    ts = (uint64_t*)&timestamp;

    printf("Timestamp: 0x%016lx\n", *ts);
#endif
    printf("Timestamp: %d.%09d\n", timestamp.seconds, timestamp.nano.nanoseconds);
}

#endif
