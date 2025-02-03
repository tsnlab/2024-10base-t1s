#ifndef DEVICE_DRIVER_LAN8650_REGISTER_H
#define DEVICE_DRIVER_LAN8650_REGISTER_H

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
    int16_t address;
};

#endif /* DEVICE_DRIVER_LAN8650_REGISTER_H */
