#pragma once

#include <stdint.h>

#include <linux/ioctl.h>
/* Command definition */
#define LAN865X_MAGIC 'L'                                            /* Driver's unique magic number */
#define LAN865X_READ_REG _IOR(LAN865X_MAGIC, 1, struct lan865x_reg)  /* Read command */
#define LAN865X_WRITE_REG _IOW(LAN865X_MAGIC, 2, struct lan865x_reg) /* Write command */

/* register access structure */
struct lan865x_reg {
    uint32_t addr;  /* register address */
    uint32_t value; /* register value */
};

/* MMS(Memory Map Selector) */
enum {
    MMS0 = 0x00,  /* Open Alliance 10BASE-T1x MAC-PHY Standard Registers */
    MMS1 = 0x01,  /* MAC Registers */
    MMS2 = 0x02,  /* PHY PCS Registers */
    MMS3 = 0x03,  /* PHY PMA/PMD Registers */
    MMS4 = 0x04,  /* PHY Vendor Specific Registers */
    MMS10 = 0x0A, /* Miscellaneous Register Descriptions */
};

/* MMS0, Open Alliance 10BASE-T1x MAC-PHY Standard Registers */
enum {
    OA_ID = 0x00,
    OA_PHYID = 0x01,
    OA_STDCAP = 0x02,
    OA_RESET = 0x03,
    OA_CONFIG0 = 0x04,
    OA_STATUS0 = 0x08,
    OA_STATUS1 = 0x09,
    OA_BUFSTS = 0X0B,
    OA_IMASK0 = 0x0C,
    OA_IMASK1 = 0x0D,
    TTSCAH = 0x10,
    TTSCAL = 0x11,
    TTSCBH = 0x12,
    TTSCBL = 0x13,
    TTSCCH = 0x14,
    TTSCCL = 0x15,
    BASIC_CONTROL = 0xFF00,
    BASIC_STATUS = 0xFF01,
    PHY_ID1 = 0xFF02,
    PHY_ID2 = 0xFF03,
    MMDCTRL = 0xFF0D,
    MMDAD = 0xFF0E
};

/* MMS1, MAC Registers */
enum {
    MAC_NCR = 0x00,
    MAC_NCFGR = 0x01,
    MAC_HRB = 0x20,
    MAC_HRT = 0x21,
    MAC_SAB1 = 0x22,
    MAC_SAT1 = 0x23,
    MAC_SAB2 = 0x24,
    MAC_SAT2 = 0x25,
    MAC_SAB3 = 0x26,
    MAC_SAT3 = 0x27,
    MAC_SAB4 = 0x28,
    MAC_SAT4 = 0x29,
    MAC_TIDM1 = 0x2A,
    MAC_TIDM2 = 0x2B,
    MAC_TIDM3 = 0x2C,
    MAC_TIDM4 = 0x2D,
    MAC_SAMB1 = 0x32,
    MAC_SAMT1 = 0x33,
    MAC_TISUBN = 0x6F,
    MAC_TSH = 0x70,
    MAC_TSL = 0x74,
    MAC_TN = 0x75,
    MAC_TA = 0x76,
    MAC_TI = 0x77,
    BMGR_CTL = 0x0200,
    STATS0 = 0x0208,
    STATS1 = 0x0209,
    STATS2 = 0x020A,
    STATS3 = 0x020B,
    STATS4 = 0x020C,
    STATS5 = 0x020D,
    STATS6 = 0x020E,
    STATS7 = 0x020F,
    STATS8 = 0x0210,
    STATS9 = 0x0211,
    STATS10 = 0x0212,
    STATS11 = 0x0213,
    STATS12 = 0x0214
};

/* MMS2, PHY PCS Registers */
enum { T1SPCSCTL = 0x08F3, T1SPCSSTS = 0x08F4, T1SPCSDIAG1 = 0x08F5, T1SPCSDIAG2 = 0x08F6 };

/* MMS3, PHY PMA/PMD Registers */
enum { T1PMAPMDEXTA = 0x12, T1PMAPMDCTL = 0x0834, T1SPMACTL = 0x08F9, T1SPMASTS = 0x08FA, T1STSTCTL = 0x08FB };

/* MMS4, PHY Vendor Specific Registers */
enum {
    CTRL1 = 0x10,
    STS1 = 0x18,
    STS2 = 0x19,
    STS3 = 0x1A,
    IMSK1 = 0x1C,
    IMSK2 = 0x1D,
    CTRCTRL = 0x20,
    TOCNTH = 0x24,
    TOCNTL = 0x25,
    BCNCNTH = 0x26,
    BCNCNTL = 0x27,
    MULTID0 = 0x30,
    MULTID1 = 0x31,
    MULTID2 = 0x32,
    MULTID3 = 0x33,
    PRSSTS = 0x36,
    PRTMGMT2 = 0x3D,
    IWDTOH = 0x3E,
    IWDTOL = 0x3F,
    TXMCTL = 0x40,
    TXMPATH = 0x41,
    TXMPATL = 0x42,
    TXMMSKH = 0x43,
    TXMMSKL = 0x44,
    TXMLOC = 0x45,
    TXMDLY = 0x49,
    RXMCTL = 0x50,
    RXMPATH = 0x51,
    RXMPATL = 0x52,
    RXMMSKH = 0x53,
    RXMMSKL = 0x54,
    RXMLOC = 0x55,
    RXMDLY = 0x59,
    CBSSPTHH = 0x60,
    CBSSPTHL = 0x61,
    CBSSTTHH = 0x62,
    CBSSTTHL = 0x63,
    CBSSLPCTL = 0x64,
    CBSTPLMTH = 0x65,
    CBSTPLMTL = 0x66,
    CBSBTLMTH = 0x67,
    CBSBTLMTL = 0x68,
    CBSCRCTRH = 0x69,
    CBSCRCTRL = 0x6A,
    CBSCTRL = 0x6B,
    PLCASKPCTL = 0x70,
    PLCATOSKP = 0x71,
    ACMACTL = 0x74,
    SLPCTL0 = 0x80,
    SLPCTL1 = 0x81,
    CDCTL0 = 0x87,
    SQICTL = 0xA0,
    SQISTS0 = 0xA1,
    SQICFG0 = 0xAA,
    SQICFG2 = 0xAC,
    ANALOG5 = 0xD5,
    MIDVER = 0xCA00,
    PLCA_CTRL0 = 0xCA01,
    PLCA_CTRL1 = 0xCA02,
    PLCA_STS = 0xCA03,
    PLCA_TOTMR = 0xCA04,
    PLCA_BURST = 0xCA05
};

/* MMS10, Miscellaneous Register Descriptions */
enum {
    QTXCFG = 0x81,
    QRXCFG = 0x82,
    PADCTRL = 0x88,
    CLKOCTL = 0x89,
    MISC = 0x8C,
    DEVID = 0x94,
    BUSPCS = 0x96,
    CFGPRTCTL = 0x99,
    ECCCTRL = 0x0100,
    ECCSTS = 0x0101,
    ECCFLTCTRL = 0x0102,
    EC0CTRL = 0x0200,
    EC1CTRL = 0x0201,
    EC2CTRL = 0x0202,
    EC3CTRL = 0x0203,
    ECRDSTS = 0x0204,
    ECTOT = 0x0205,
    ECCLKSH = 0x0206,
    ECCLKSL = 0x0207,
    ECCLKNS = 0x0208,
    ECRDTS0 = 0x0209,
    ECRDTS1 = 0x020A,
    ECRDTS2 = 0x020B,
    ECRDTS3 = 0x020C,
    ECRDTS4 = 0x020D,
    ECRDTS5 = 0x020E,
    ECRDTS6 = 0x020F,
    ECRDTS7 = 0x0210,
    ECRDTS8 = 0x0211,
    ECRDTS9 = 0x0212,
    ECRDTS10 = 0x0213,
    ECRDTS11 = 0x0214,
    ECRDTS12 = 0x0215,
    ECRDTS13 = 0x0216,
    ECRDTS14 = 0x0217,
    ECRDTS15 = 0x0218,
    PACYC = 0x021F,
    PACTRL = 0x0220,
    EG0STNS = 0x0221,
    EG0STSECL = 0x0222,
    EG0STSECH = 0x0223,
    EG0PW = 0x0224,
    EG0IT = 0x0225,
    EG0CTL = 0x0226,
    EG1STNS = 0x0227,
    EG1STSECL = 0x0228,
    EG1STSECH = 0x0229,
    EG1PW = 0x022A,
    EG1IT = 0x022B,
    EG1CTL = 0x022C,
    EG2STNS = 0x022D,
    EG2STSECL = 0x022E,
    EG2STSECH = 0x022F,
    EG2PW = 0x0230,
    EG2IT = 0x0231,
    EG2CTL = 0x0232,
    EG3STNS = 0x0233,
    EG3STSECL = 0x0234,
    EG3STSECH = 0x0235,
    EG3PW = 0x0236,
    EG3IT = 0x0237,
    EG3CTL = 0x0238,
    PPSCTL = 0X0239,
    SEVINTEN = 0x023A,
    SEVINTDIS = 0x023B,
    SEVIM = 0x023C,
    SEVSTS = 0x023D
};

struct reginfo {
    char* desc;
    int32_t address;
};

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

#define ADDR_WIDTH 16
#define ADDR_MASK 0xffff
#define VALUE_MASK 0xffff

#define HEX_BASE 16

/* operation type */
enum oper_type {
    READ_OPERATION = 0,
    WRITE_OPERATION,
    MMS_OPERATION,
    MAX_OPERATION,
};
