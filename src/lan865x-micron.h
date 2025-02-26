#pragma once

#include <stdint.h>

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
#define MMS0_OA_ID 0x00
#define MMS0_OA_PHYID 0x01
#define MMS0_OA_STDCAP 0x02
#define MMS0_OA_RESET 0x03
#define MMS0_OA_CONFIG0 0x04
#define MMS0_OA_STATUS0 0x08
#define MMS0_OA_STATUS1 0x09
#define MMS0_OA_BUFSTS 0X0B
#define MMS0_OA_IMASK0 0x0C
#define MMS0_OA_IMASK1 0x0D
#define MMS0_TTSCAH 0x10
#define MMS0_TTSCAL 0x11
#define MMS0_TTSCBH 0x12
#define MMS0_TTSCBL 0x13
#define MMS0_TTSCCH 0x14
#define MMS0_TTSCCL 0x15
#define MMS0_BASIC_CONTROL 0xFF00
#define MMS0_BASIC_STATUS 0xFF01
#define MMS0_PHY_ID1 0xFF02
#define MMS0_PHY_ID2 0xFF03
#define MMS0_MMDCTRL 0xFF0D
#define MMS0_MMDAD 0xFF0E

/* MMS1, MAC Registers */
#define MMS1_MAC_NCR 0x00
#define MMS1_MAC_NCFGR 0x01
#define MMS1_MAC_HRB 0x20
#define MMS1_MAC_HRT 0x21
#define MMS1_MAC_SAB1 0x22
#define MMS1_MAC_SAT1 0x23
#define MMS1_MAC_SAB2 0x24
#define MMS1_MAC_SAT2 0x25
#define MMS1_MAC_SAB3 0x26
#define MMS1_MAC_SAT3 0x27
#define MMS1_MAC_SAB4 0x28
#define MMS1_MAC_SAT4 0x29
#define MMS1_MAC_TIDM1 0x2A
#define MMS1_MAC_TIDM2 0x2B
#define MMS1_MAC_TIDM3 0x2C
#define MMS1_MAC_TIDM4 0x2D
#define MMS1_MAC_SAMB1 0x32
#define MMS1_MAC_SAMT1 0x33
#define MMS1_MAC_TISUBN 0x6F
#define MMS1_MAC_TSH 0x70
#define MMS1_MAC_TSL 0x74
#define MMS1_MAC_TN 0x75
#define MMS1_MAC_TA 0x76
#define MMS1_MAC_TI 0x77
#define MMS1_BMGR_CTL 0x0200
#define MMS1_STATS0 0x0208
#define MMS1_STATS1 0x0209
#define MMS1_STATS2 0x020A
#define MMS1_STATS3 0x020B
#define MMS1_STATS4 0x020C
#define MMS1_STATS5 0x020D
#define MMS1_STATS6 0x020E
#define MMS1_STATS7 0x020F
#define MMS1_STATS8 0x0210
#define MMS1_STATS9 0x0211
#define MMS1_STATS10 0x0212
#define MMS1_STATS11 0x0213
#define MMS1_STATS12 0x0214

/* MMS2, PHY PCS Registers */
#define MMS2_T1SPCSCTL 0x08F3
#define MMS2_T1SPCSSTS 0x08F4
#define MMS2_T1SPCSDIAG1 0x08F5
#define MMS2_T1SPCSDIAG2 0x08F6

/* MMS3, PHY PMA/PMD Registers */
#define MMS3_T1PMAPMDEXTA 0x12
#define MMS3_T1PMAPMDCTL 0x0834
#define MMS3_T1SPMACTL 0x08F9
#define MMS3_T1SPMASTS 0x08FA
#define MMS3_T1STSTCTL 0x08FB

/* MMS4, PHY Vendor Specific Registers */
#define MMS4_CTRL1 0x10
#define MMS4_STS1 0x18
#define MMS4_STS2 0x19
#define MMS4_STS3 0x1A
#define MMS4_IMSK1 0x1C
#define MMS4_IMSK2 0x1D
#define MMS4_CTRCTRL 0x20
#define MMS4_TOCNTH 0x24
#define MMS4_TOCNTL 0x25
#define MMS4_BCNCNTH 0x26
#define MMS4_BCNCNTL 0x27
#define MMS4_MULTID0 0x30
#define MMS4_MULTID1 0x31
#define MMS4_MULTID2 0x32
#define MMS4_MULTID3 0x33
#define MMS4_PRSSTS 0x36
#define MMS4_PRTMGMT2 0x3D
#define MMS4_IWDTOH 0x3E
#define MMS4_IWDTOL 0x3F
#define MMS4_TXMCTL 0x40
#define MMS4_TXMPATH 0x41
#define MMS4_TXMPATL 0x42
#define MMS4_TXMMSKH 0x43
#define MMS4_TXMMSKL 0x44
#define MMS4_TXMLOC 0x45
#define MMS4_TXMDLY 0x49
#define MMS4_RXMCTL 0x50
#define MMS4_RXMPATH 0x51
#define MMS4_RXMPATL 0x52
#define MMS4_RXMMSKH 0x53
#define MMS4_RXMMSKL 0x54
#define MMS4_RXMLOC 0x55
#define MMS4_RXMDLY 0x59
#define MMS4_CBSSPTHH 0x60
#define MMS4_CBSSPTHL 0x61
#define MMS4_CBSSTTHH 0x62
#define MMS4_CBSSTTHL 0x63
#define MMS4_CBSSLPCTL 0x64
#define MMS4_CBSTPLMTH 0x65
#define MMS4_CBSTPLMTL 0x66
#define MMS4_CBSBTLMTH 0x67
#define MMS4_CBSBTLMTL 0x68
#define MMS4_CBSCRCTRH 0x69
#define MMS4_CBSCRCTRL 0x6A
#define MMS4_CBSCTRL 0x6B
#define MMS4_PLCASKPCTL 0x70
#define MMS4_PLCATOSKP 0x71
#define MMS4_ACMACTL 0x74
#define MMS4_SLPCTL0 0x80
#define MMS4_SLPCTL1 0x81
#define MMS4_CDCTL0 0x87
#define MMS4_SQICTL 0xA0
#define MMS4_SQISTS0 0xA1
#define MMS4_SQICFG0 0xAA
#define MMS4_SQICFG2 0xAC
#define MMS4_ANALOG5 0xD5
#define MMS4_INDIR_RD_ADDR 0xD8
#define MMS4_INDIR_RD_VAL 0xD9
#define MMS4_INDIR_RD_WIDTH 0xDA
#define MMS4_MIDVER 0xCA00
#define MMS4_PLCA_CTRL0 0xCA01
#define MMS4_PLCA_CTRL1 0xCA02
#define MMS4_PLCA_STS 0xCA03
#define MMS4_PLCA_TOTMR 0xCA04
#define MMS4_PLCA_BURST 0xCA05

#define MMS4_A_00AD 0x00AD
#define MMS4_A_00AE 0x00AE
#define MMS4_A_00AF 0x00AF
#define MMS4_A_00B0 0x00B0
#define MMS4_A_00B1 0x00B1
#define MMS4_A_00B2 0x00B2
#define MMS4_A_00B3 0x00B3
#define MMS4_A_00B4 0x00B4
#define MMS4_A_00B5 0x00B5
#define MMS4_A_00B6 0x00B6
#define MMS4_A_00B7 0x00B7
#define MMS4_A_00B8 0x00B8
#define MMS4_A_00B9 0x00B9
#define MMS4_A_00BA 0x00BA
#define MMS4_A_00BB 0x00BB

#define MMS4_A_00D0 0x00D0
#define MMS4_A_00E0 0x00E0
#define MMS4_A_0084 0x0084
#define MMS4_A_008A 0x008A
#define MMS4_A_00E9 0x00E9
#define MMS4_A_00F5 0x00F5
#define MMS4_A_00F4 0x00F4
#define MMS4_A_00F8 0x00F8
#define MMS4_A_00F9 0x00F9
#define MMS4_A_0091 0x0091
#define MMS4_A_0077 0x0077

/* MMS10, Miscellaneous Register Descriptions */
#define MMS10_QTXCFG 0x81
#define MMS10_QRXCFG 0x82
#define MMS10_PADCTRL 0x88
#define MMS10_CLKOCTL 0x89
#define MMS10_MISC 0x8C
#define MMS10_DEVID 0x94
#define MMS10_BUSPCS 0x96
#define MMS10_CFGPRTCTL 0x99
#define MMS10_ECCCTRL 0x0100
#define MMS10_ECCSTS 0x0101
#define MMS10_ECCFLTCTRL 0x0102
#define MMS10_EC0CTRL 0x0200
#define MMS10_EC1CTRL 0x0201
#define MMS10_EC2CTRL 0x0202
#define MMS10_EC3CTRL 0x0203
#define MMS10_ECRDSTS 0x0204
#define MMS10_ECTOT 0x0205
#define MMS10_ECCLKSH 0x0206
#define MMS10_ECCLKSL 0x0207
#define MMS10_ECCLKNS 0x0208
#define MMS10_ECRDTS0 0x0209
#define MMS10_ECRDTS1 0x020A
#define MMS10_ECRDTS2 0x020B
#define MMS10_ECRDTS3 0x020C
#define MMS10_ECRDTS4 0x020D
#define MMS10_ECRDTS5 0x020E
#define MMS10_ECRDTS6 0x020F
#define MMS10_ECRDTS7 0x0210
#define MMS10_ECRDTS8 0x0211
#define MMS10_ECRDTS9 0x0212
#define MMS10_ECRDTS10 0x0213
#define MMS10_ECRDTS11 0x0214
#define MMS10_ECRDTS12 0x0215
#define MMS10_ECRDTS13 0x0216
#define MMS10_ECRDTS14 0x0217
#define MMS10_ECRDTS15 0x0218
#define MMS10_PACYC 0x021F
#define MMS10_PACTRL 0x0220
#define MMS10_EG0STNS 0x0221
#define MMS10_EG0STSECL 0x0222
#define MMS10_EG0STSECH 0x0223
#define MMS10_EG0PW 0x0224
#define MMS10_EG0IT 0x0225
#define MMS10_EG0CTL 0x0226
#define MMS10_EG1STNS 0x0227
#define MMS10_EG1STSECL 0x0228
#define MMS10_EG1STSECH 0x0229
#define MMS10_EG1PW 0x022A
#define MMS10_EG1IT 0x022B
#define MMS10_EG1CTL 0x022C
#define MMS10_EG2STNS 0x022D
#define MMS10_EG2STSECL 0x022E
#define MMS10_EG2STSECH 0x022F
#define MMS10_EG2PW 0x0230
#define MMS10_EG2IT 0x0231
#define MMS10_EG2CTL 0x0232
#define MMS10_EG3STNS 0x0233
#define MMS10_EG3STSECL 0x0234
#define MMS10_EG3STSECH 0x0235
#define MMS10_EG3PW 0x0236
#define MMS10_EG3IT 0x0237
#define MMS10_EG3CTL 0x0238
#define MMS10_PPSCTL 0x0239
#define MMS10_SEVINTEN 0x023A
#define MMS10_SEVINTDIS 0x023B
#define MMS10_SEVIM 0x023C
#define MMS10_SEVSTS 0x023D

/**
 * The LAN8650/1 uses a 25.0 MHz timer clock source and requires that the timer increment by 40.0 ns for each clock
 * period. This is programmed by writing the value 0x00000028 to the TSU Timer Increment (MAC_TI) register.
 */
#define TIMER_INCREMENT 0x28

#define MAX_CONTROL_CMD_LEN (0x7F)
#define MAX_PAYLOAD_BYTE (64U) /* TODO: This is configurable so need to change based on configuration */
#define REG_SIZE (4)
#define MAX_REG_DATA_ONECONTROLCMD (MAX_CONTROL_CMD_LEN * REG_SIZE)
#define MAX_REG_DATA_ONECHUNK (MAX_PAYLOAD_BYTE / REG_SIZE)
#define MAX_DATA_DWORD_ONECHUNK (MAX_REG_DATA_ONECHUNK)
#define HEADER_SIZE (4)
#define FOOTER_SIZE (4)

#define MMS04_INDIR_WIDTH 0x2

/* Value1: -5 to 15 */
#define VALUE1_ADDR 0x04
#define VALUE2_ADDR 0x08
#define VALUE_MASK 0x1F
#define VALUE_SIGN_MASK 0x10

#define VALUE1_OFFSET1 9
#define VALUE1_OFFSET2 14
#define VALUE1_SHIFT1 10
#define VALUE1_SHIFT2 4
#define VALUE1_LOWEST_VAL 0x03

#define VALUE2_OFFSET 40
#define VALUE2_SHIFT 10

#define VALUE_OFFSET_MASK 0x3F

#define MAX_VALUE_5BITS 0x20

#define SQI_BYTE_WIDTH 8
#define SQI_4_BYTES_WIDTH 32

#define SQI_PARA3_OFFSET1 5
#define SQI_PARA3_OFFSET2 9
#define SQI_PARA4_OFFSET1 9
#define SQI_PARA4_OFFSET2 14
#define SQI_PARA5_OFFSET1 17
#define SQI_PARA5_OFFSET2 22

#define START_WORD_OFFSET_UNIT (4U)
#define END_BYTE_OFFSET (1U)

#define MMS4_PLCA_CTRL0_INIT_VAL 0x00008000
#define MMS1_MAC_NCFGR_INIT_VAL 0x000000C0
#define MMS1_MAC_NCR_INIT_VAL 0x0000000C

#define MMS4_A_00B0_V 0x0103
#define MMS4_A_00B1_V 0x0910
#define MMS4_A_00B2_V 0x1D26
#define MMS4_A_00B3_V 0x002A
#define MMS4_A_00B4_V 0x0103
#define MMS4_A_00B5_V 0x070D
#define MMS4_A_00B6_V 0x1720
#define MMS4_A_00B7_V 0x0027
#define MMS4_A_00B8_V 0x0509
#define MMS4_A_00B9_V 0x0E13
#define MMS4_A_00BA_V 0x1C25
#define MMS4_A_00BB_V 0x002B

#define MMS4_A_00D0_V 0x3F31
#define MMS4_A_00E0_V 0xC000
#define MMS4_A_00E9_V 0x9E50
#define MMS4_A_00F5_V 0x1CF8
#define MMS4_A_00F4_V 0xC020
#define MMS4_A_00F8_V 0xB900
#define MMS4_A_00F9_V 0x4E53
#define MMS4_A_0081_V 0x0080
#define MMS4_A_0091_V 0x9660
#define MMS4_A_0077_V 0x0028
#define MMS4_A_0043_V 0x00FF
#define MMS4_A_0044_V 0xFFFF
#define MMS4_A_0045_V 0x0000
#define MMS4_A_0053_V 0x00FF
#define MMS4_A_0054_V 0xFFFF
#define MMS4_A_0055_V 0x0000
#define MMS4_A_0040_V 0x0002
#define MMS4_A_0050_V 0x0002

#define MMS0_OA_STATUS0_RESETC_SHIFT 6
#define MMS0_OA_CONFIG0_SYNC_SHIFT 15
#define MMS0_OA_CONFIG0_FTSE_SHIFT 7
#define MMS0_OA_CONFIG0_FTSS_SHIFT 6
#define MMS0_OA_BUFSTS_TXC_SHIFT 8
#define MMS0_OA_BUFSTS_TXC_MASK 0xFF
#define MMS0_OA_BUFSTS_RBA_MASK 0xFF
#define MMS4_CDCTL0_CDEN_SHIFT 15

#define LAN865X_MMS_MASK 0x0F
#define LAN865X_NODE_ID_MASK 0xFF
#define LAN865X_NODE_COUNT_MASK 0xFF
#define LAN865X_NODE_COUNT_SHIFT 8

#define MAC_BOTTOM_REGISTER_MASK 0xffffffff
#define MAC_TOP_REGISTER_MASK 0xffff
#define MAC_BOTTOM_REGISTER_WIDTH 32
#define MAC_TOP_REGISTER_FLTTYP_SHIFT 16
#define MAC_TOP_REGISTER_FLTTYP_MASK 1
#define MAC_TOP_REGISTER_FLTBM_SHIFT 24
#define MAC_TOP_REGISTER_FLTBM_MASK 0x3F

#define LAN865X_PARITY_BIT_MASK 0x11111111
#define LAN865X_PARITY_SHIFT_BITS 28

#define BOARD_MAC_SPECIFIC_ID 1 /* 1 to 4 */

/**
 * DNC: Data, Not Control - Flag indicating the type of transaction, data or control.
 *    0 Control (register read/write)
 *    1 Data (Ethernet frame)
 */
enum { DNC_FLAG_CONTROL = 0, DNC_FLAG_DATA };

/**
 * WNR: Write, Not Read - This bit indicates the type of control access to perform.
 *    0 Control/Status register read
 *    1 Control/Status register write
 */
enum { WNR_BIT_READ = 0, WNR_BIT_WRITE };

/**
 * AID: Address Increment Disable - Normally, when this bit is 0, the address is post-incremented by one following
 * each read/write register access within the same control command. When this bit is 1, subsequent reads or
 * writes within the same control command will result in the same register address being accessed. This feature
 * is useful for reading and writing register FIFOs located at a single address.
 * 0 Register address will automatically be post-incremented following each read/write within the
 * same control command.
 * 1 Register address will not be post-incremented following each read/write within the same control
 * command, allowing successive access to the same register address.
 */
enum { AID_FALSE = 0, AID_TRUE };

struct ctrl_cmd_reg {
    uint8_t mms;
    uint8_t length;
    uint16_t addr;
    uint32_t buffer[MAX_CONTROL_CMD_LEN];
};

union ctrl_header {
    uint32_t ctrl_frame;
    struct {
        uint32_t p : 1;
        uint32_t len : 7;
        uint32_t addr : 16;
        uint32_t mms : 4;
        uint32_t aid : 1;
        uint32_t wnr : 1;
        uint32_t hdrb : 1;
        uint32_t dnc : 1;
    } ctrl_bits;
};

union data_header {
    uint32_t data_frame;
    struct {
        uint32_t p : 1;
        uint32_t rsvd3 : 5;
        uint32_t tsc : 2;
        uint32_t ebo : 6;
        uint32_t ev : 1;
        uint32_t rsvd2 : 1;
        uint32_t swo : 4;
        uint32_t sv : 1;
        uint32_t dv : 1;
        uint32_t vs : 2;
        uint32_t rsvd1 : 5;
        uint32_t norx : 1;
        uint32_t seq : 1;
        uint32_t dnc : 1;
    } data_bits;
};

union data_footer {
    uint32_t data_frame;
    struct {
        uint32_t p : 1;
        uint32_t txc : 5;
        uint32_t rtps : 1;
        uint32_t rtsa : 1;
        uint32_t ebo : 6;
        uint32_t ev : 1;
        uint32_t fd : 1;
        uint32_t swo : 4;
        uint32_t sv : 1;
        uint32_t dv : 1;
        uint32_t vs : 2;
        uint32_t rba : 5;
        uint32_t sync : 1;
        uint32_t hdrb : 1;
        uint32_t exst : 1;
    } footer_bits;
};

struct reg_info {
    char* desc;
    int32_t address;
};
