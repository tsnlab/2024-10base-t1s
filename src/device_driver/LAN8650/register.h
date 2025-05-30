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

#define DUMPREG_GENERAL 0x1
#define DUMPREG_RX 0x2
#define DUMPREG_TX 0x4
#define DUMPREG_ALL 0x7

/* Register Class: General */
#define TSN_SYSTEM_INFO_H 0x0278 /* Year [63:56] : Month [55:48] : Day [47:40] : Commit # [39:32] */
#define TSN_SYSTEM_INFO_L 0x027C /* RESERVED [31:0] */

#define TSN_MAC_ADDR_H 0x0280 /* RESERVED [63:48] : MAC Address [47:32] */
#define TSN_MAC_ADDR_L 0x0284 /* MAC Address [31:0] */

#define TSN_SYSCOUNT_H 0x2888 /* SYSTEM_COUNT [63:32] */
#define TSN_SYSCOUNT_L 0x288C /* SYSTEM_COUNT [31:0] */

#define TSN_SYSTEM_CONTROL_H 0x0290 /* RESERVED [63:32] */
#define TSN_SYSTEM_CONTROL_L 0x0294 /* RESERVED [31:1] : TSN ENABLE */
#define TSN_RX_CONTROL_H 0x0298 /* MAX_FRAME_LENGTH [63:48] : */
                                /* RESERVED [47:45] : */
                                /* MAX_FRAME_FIFO_DATA_COUNT [44:32] */
#define TSN_RX_CONTROL_L 0x029C /* RESERVED [31:25] : */
                                /* MAX_META_FIFO_DATA_COUNT [24:16] : */
                                /* RESERVED [15:8] : CLEAR DROP BYTE COUNT : */
                                /* CLEAR DROP FRAME COUNT : */
                                /* CLEAR RX BYTE COUNT : */
                                /* CLEAR RX FRAME COUNT : */
                                /* CHECK-SUM ENABLE : */
                                /* JUMBO DROP :  FCS OFFLOAD : */
                                /* VLAN OFFLOAD */
#define TSN_TX_CONTROL_H 0x02A0 /* RESERVED [63:32] */
#define TSN_TX_CONTROL_L 0x02A4 /* ETHERNET_INTER_FRAME_GAP [31:16] : */
                                /* RESERVED [15:10] : */
                                /* HOST_FIFO_FULL_LIMIT [9:0] */
#define PPS_PULSE_AT_H 0x02A8 /* PULSE_AT [63:32] */
#define PPS_PULSE_AT_L 0x02AC /* PULSE_AT [31:0] */
#define PPS_CYCLE_1S_H 0x02B0 /* CYCLE_1S [63:32] */
#define PPS_CYCLE_1S_L 0x02B4 /* CYCLE_1S [31:0] */

/* Register Class: Rx (Frame Decoder) */
#define FD_RX_TSTAMP_H 0x0000 /* RX_TSTAMP [63:32] */
#define FD_RX_TSTAMP_L 0x0004 /* RX_TSTAMP [31:0] */
#define FD_RX_FRAME_STATUS1_H 0x0008 /* CHECK_SUM [63:32] */
#define FD_RX_FRAME_STATUS1_L 0x000C /* VLAN_TAG [31:16] : FRAME_LENGTH [15:0]  */
#define FD_RX_FRAME_STATUS2_H 0x0010 /* RESERVED [63:32] */
#define FD_RX_FRAME_STATUS2_L 0x0014 /* RESERVED [31:5] : VLAN OFFLOAD FLAG : */
                                     /* FCS OFFLOAD FLAG : FCS ERROR FLAG : */
                                     /* JUMBO ERROR FLAG : FIFO FULL FLAG */
#define FD_TOTAL_RX_FRAME_CNT_H 0x0018 /* TOTAL_RX_FRAME_CNT [63:32] */
#define FD_TOTAL_RX_FRAME_CNT_L 0x001C /* TOTAL_RX_FRAME_CNT [31:0] */
#define FD_TOTAL_RX_BYTE_CNT_H 0x0020 /* TOTAL_RX_BYTE_CNT [63:32] */
#define FD_TOTAL_RX_BYTE_CNT_L 0x0024 /* TOTAL_RX_BYTE_CNT [31:0] */
#define FD_TOTAL_DROP_FRAME_CNT_H 0x0028 /* TOTAL_DROP_FRAME_CNT [63:32] */
#define FD_TOTAL_DROP_FRAME_CNT_L 0x002C /* TOTAL_DROP_FRAME_CNT [31:0] */
#define FD_TOTAL_DROP_BYTE_CNT_H 0x0030 /* TOTAL_DROP_BYTE_CNT [63:32] */
#define FD_TOTAL_DROP_BYTE_CNT_L 0x0034 /* TOTAL_DROP_BYTE_CNT [31:0] */
#define FD_FCS_DROP_FRAME_CNT_H 0x0038 /* FCS_DROP_FRAME_CNT [63:32] */
#define FD_FCS_DROP_FRAME_CNT_L 0x003C /* FCS_DROP_FRAME_CNT [31:0] */
#define FD_FCS_DROP_BYTE_CNT_H 0x0040 /* FCS_DROP_BYTE_CNT [63:32] */
#define FD_FCS_DROP_BYTE_CNT_L 0x0044 /* FCS_DROP_BYTE_CNT [31:0] */
#define FD_JUMBO_DROP_FRAME_CNT_H 0x0048 /* JUMBO_DROP_FRAME_CNT [63:32] */
#define FD_JUMBO_DROP_FRAME_CNT_L 0x004C /* JUMBO_DROP_FRAME_CNT [31:0] */
#define FD_JUMBO_DROP_BYTE_CNT_H 0x0050 /* JUMBO_DROP_BYTE_CNT [63:32] */
#define FD_JUMBO_DROP_BYTE_CNT_L 0x0054 /* JUMBO_DROP_BYTE_CNT [31:0] */
#define FD_FIFO_FULL_DROP_FRAME_CNT_H 0x0058 /* FIFO_FULL_DROP_FRAME_CNT [63:32] */
#define FD_FIFO_FULL_DROP_FRAME_CNT_L 0x005C /* FIFO_FULL_DROP_FRAME_CNT [31:0] */
#define FD_FIFO_FULL_DROP_BYTE_CNT_H 0x0060 /* FIFO_FULL_DROP_BYTE_CNT [63:32] */
#define FD_FIFO_FULL_DROP_BYTE_CNT_L 0x0064 /* FIFO_FULL_DROP_BYTE_CNT [31:0] */

/* Register Class: FIFO (META, FRAME) */
#define TSN_RX_FIFO_STATUS_H 0x00A8 /* RESERVED [63:32] */
#define TSN_RX_FIFO_STATUS_L 0x00AC /* RES [31:29] : */
                                    /* FRAME_FIFO_DATA_CNT [28:16] : */
                                    /* RESERVED [15:9] : */
                                    /* META_FIFO_DATA_CNT [8:0] */


/* Register Class: Tx (Frame Stacker) */
#define FS_GENERAL_STATUS_H 0x00B0 /* RESERVED [63:58] : */
                                   /* HOST_FIFO_RD_CNT [57:48] : */
                                   /* RESERVED [47:42] : */
                                   /* HOST_FIFO_WR_CNT [41:32] */
#define FS_GENERAL_STATUS_L 0x00B4 /* RESERVED [31:4] : XDMA AXIS TREADY : */
                                   /* HOST FIFO USER FULL */
                                   /* HOST FIFO EMPTY : HOST FIFO FULL */
#define FS_TOTAL_RX_FRAME_CNT_H 0x00B8 /* TOTAL_RX_FRAME_CNT [63:32] */
#define FS_TOTAL_RX_FRAME_CNT_L 0x00BC /* TOTAL_RX_FRAME_CNT [31:0] */
#define FS_TOTAL_RX_16BYTE_CNT_H 0x00C0 /* TOTAL_RX_16BYTE_CNT [63:32] */
#define FS_TOTAL_RX_16BYTE_CNT_L 0x00C4 /* TOTAL_RX_16BYTE_CNT [31:0] */
#define FS_TOTAL_BACK_PRESSURE_EVENT_CNT_H 0x00C8 /* TOTAL_BACK_PRESSURE_EVENT_CNT [63:32] */
#define FS_TOTAL_BACK_PRESSURE_EVENT_CNT_L 0x00CC /* TOTAL_BACK_PRESSURE_EVENT_CNT [31:0] */


/* Register Class: Tx (Frame Parser) */
#define FP_FRAME_TICK_FROM_H 0x00E0 /* TICK_FROM [63:32] */
#define FP_FRAME_TICK_FROM_L 0x00E4 /* TICK_FROM [31:0] */
#define FP_FRAME_TICK_TO_H 0x00E8 /* TICK_TO [63:32] */
#define FP_FRAME_TICK_TO_L 0x00EC /* TICK_TO [31:0] */
#define FP_FRAME_TICK_DELAY_FROM_H 0x00F0 /* TICK_DELAY_FROM [63:32] */
#define FP_FRAME_TICK_DELAY_FROM_L 0x00F4 /* TICK_DELAY_FROM [31:0] */
#define FP_FRAME_TICK_DELAY_TO_H 0x00F8 /* TICK_DELAY_TO [63:32] */
#define FP_FRAME_TICK_DELAY_TO_L 0x00FC /* TICK_DELAY_TO [31:0] */
#define FP_FRAME_STATUS1_H 0x0100 /* RESERVED [63:56] : ALLOC_ADDR [55:48] : */
                                  /* RESERVED [47:33] : FAIL_POLICY [32] */
#define FP_FRAME_STATUS1_L 0x0104 /* FRAME_LENGTH [31:16] : TIMESTAMP_ID [15:0] */
#define FP_FRAME_STATUS2_H 0x0108 /* RESERVED [63:52] : */
                                  /* META_AXIS_HSK_CNT [51:49] : */
                                  /* META_ADDR_OFFSET [48] : */
                                  /* RESERVED [47:39] : */
                                  /* FRAME_ADDR_OFFSET [38:32] */
#define FP_FRAME_STATUS2_L 0x010C /* FRAME_AXIS_HSK_CNT [31:16] : */
                                  /* RX_FRAME_16BYTE_CNT [15:0] */


/* Register Class: Tx (Frame Scheduler) */
#define FSCH_TX_FRAME_STATUS_H 0x0118 /* RESERVED [63:48] : */
                                      /* TX_FRAME_TSTAMP_ID [47:32] */
#define FSCH_TX_FRAME_STATUS_L 0x011C /* TX_FRAME_LENGTH [15:0] : */
                                      /* RESERVED [15:8] : */
                                      /* TX_FRAME_ADDRESS [7:0] */
#define FSCH_TOTAL_NEW_ENTRY_CNT_H 0x0120 /* TOTAL_NEW_ENTRY_CNT [63:32] */
#define FSCH_TOTAL_NEW_ENTRY_CNT_L 0x0124 /* TOTAL_NEW_ENTRY_CNT [31:0] */
#define FSCH_TOTAL_VALID_ENTRY_CNT_H 0x0128 /* TOTAL_VALID_ENTRY_CNT [63:32] */
#define FSCH_TOTAL_VALID_ENTRY_CNT_L 0x012C /* TOTAL_VALID_ENTRY_CNT [31:0] */
#define FSCH_TOTAL_DELAY_ENTRY_CNT_H 0x0130 /* TOTAL_DELAY_ENTRY_CNT [63:32] */
#define FSCH_TOTAL_DELAY_ENTRY_CNT_L 0x0134 /* TOTAL_DELAY_ENTRY_CNT [31:0] */
#define FSCH_TOTAL_DROP_ENTRY_CNT_H 0x0138 /* TOTAL_DROP_ENTRY_CNT [63:32] */
#define FSCH_TOTAL_DROP_ENTRY_CNT_L 0x013C /* TOTAL_DROP_ENTRY_CNT [31:0] */


/* Register Class: Tx (Frame Buffer) */
#define FBW_BUFFER_WRITE_STATUS1_H 0x0148 /* RESERVED [63:40] : */
                                          /* ADDR_FIFO_DATA_CNT [39:32] */
#define FBW_BUFFER_WRITE_STATUS1_L 0x014C /* RESERVED [31:11] : */
                                          /* BRAM_SEL [10:8] : ALLOC_ADDR [7:0] */
#define FBW_BUFFER_WRITE_STATUS2_H 0x0150 /* RESERVED [63:59] : */
                                          /* BRAM3_WR_ADDR_LAST [58:48] : */
                                          /* RESERVED [47:43] : */
                                          /* BRAM2_WR_ADDR_LAST [42:32] */
#define FBW_BUFFER_WRITE_STATUS2_L 0x0154 /* RESERVED [31:27] : */
                                          /* BRAM1_WR_ADDR_LAST [26:16] : */
                                          /* RESERVED [15:11] : */
                                          /* BRAM0_WR_ADDR_LAST [10:0] */
#define FBW_BUFFER_WRITE_STATUS3_H 0x0158 /* RESERVED [63:59] : */
                                          /* BRAM7_WR_ADDR_LAST [58:48] : */
                                          /* RESERVED [47:43] : */
                                          /* BRAM6_WR_ADDR_LAST [42:32] */
#define FBW_BUFFER_WRITE_STATUS3_L 0x015C /* RESERVED [31:27] : */
                                          /* BRAM5_WR_ADDR_LAST [26:16] */
                                          /* RESERVED [15:11] */
                                          /* BRAM4_WR_ADDR_LAST [10:0] */
#define FBW_ADDR_FIFO_CNT_H 0x0160 /* RESERVED [63:32] */
#define FBW_ADDR_FIFO_CNT_L 0x0164 /* RESERVED [31:8] : */
                                   /* ADDR_FIFO_DATA_CNT [7:0] */
#define FBR_BUFFER_READ_STATUS1_H 0x0168 /* RESERVED [63:40] : */
                                         /* DROP_FRAME_ADDR [39:32] */
#define FBR_BUFFER_READ_STATUS1_L 0x016C /* SELECTED_FRAME_16BYTE_CNT [31:16] : */
                                         /* RESERVED [15:11] : */
                                         /* BRAM_SEL [2:0] : */
                                         /* SELECTED_FRAME_ADDR [7:0] */
#define FBR_BUFFER_READ_STATUS2_H 0x0170 /* RESERVED [63:59] : */
                                         /* BRAM3_RD_ADDR_LAST [58:48] : */
                                         /* RESERVED [47:43] : */
                                         /* BRAM2_RD_ADDR_LAST [42:32] */
#define FBR_BUFFER_READ_STATUS2_L 0x0174 /* RESERVED [31:27] : */
                                         /* BRAM1_RD_ADDR_LAST [26:16] : */
                                         /* RESERVED [15:11] : */
                                         /* BRAM0_RD_ADDR_LAST [10:0] */
#define FBR_BUFFER_READ_STATUS3_H 0x0178 /* RESERVED [63:59] : */
                                         /* BRAM7_RD_ADDR_LAST [58:48] : */
                                         /* RESERVED [47:43] : */
                                         /* BRAM6_RD_ADDR_LAST [42:32] */
#define FBR_BUFFER_READ_STATUS3_L 0x017C /* RESERVED [31:27] : */
                                         /* BRAM5_RD_ADDR_LAST [26:16] : */
                                         /* RESERVED [15:11] : */
                                         /* BRAM4_RD_ADDR_LAST [10:0] */

/* Register Class: Tx (Frame Transfer FSM) */
#define FTRSF_TX_FRAME_STATUS_H 0x0188 /* TX_METADATA [63:32] */
#define FTRSF_TX_FRAME_STATUS_L 0x018C /* TX_FRAME_LENGTH [15:0] : */
                                       /* RESERVED [15:8] : */
                                       /* TX_FRAME_ADDRESS [7:0] */
#define FTRSF_GENERAL_STATUS_H 0x0190 /* RESERVED [63:35] : */
                                      /* TX_FRAME_FIFO_FULL [34] : */
                                      /* TX_META_FIFO_FULL [33] : */
                                      /* TX_STATUS [32] */
#define FTRSF_GENERAL_STATUS_L 0x0194 /* RESERVED [31:25] : */
                                      /* TX_FRAME_FIFO_DATA_COUNT [24:16] : */
                                      /* RESERVED [15:9] : */
                                      /* TX_META_FIFO_DATA_COUNT [8:0] */
#define FTRSF_TOTAL_TX_FRAME_CNT_H 0x0198 /* TOTAL_TX_FRAME_CNT [63:32] */
#define FTRSF_TOTAL_TX_FRAME_CNT_L 0x019C /* TOTAL_TX_FRAME_CNT [31:0] */
#define FTRSF_TOTAL_META_FIFO_FULL_CNT_H 0x01A0 /* TOTAL_META_FIFO_FULL_CNT [63:32] */
#define FTRSF_TOTAL_META_FIFO_FULL_CNT_L 0x01A4 /* TOTAL_META_FIFO_FULL_CNT [31:0] */
#define FTRSF_TOTAL_FRAME_FIFO_FULL_CNT_H 0x01A8 /* TOTAL_FRAME_FIFO_FULL_CNT [63:32] */
#define FTRSF_TOTAL_FRAME_FIFO_FULL_CNT_L 0x01AC /* TOTAL_FRAME_FIFO_FULL_CNT [31:0] */

/* Register Class: Tx (Frame Transmitter FSM) */
#define FT_TX_FRAME_STATUS1_H 0x01B8 /* TX_METADATA [63:32] */
#define FT_TX_FRAME_STATUS1_L 0x01BC /* TX_FRAME_16BYTE_LENGTH [31:16] : */
                                     /* TX_FRAME_LENGTH [15:0] */
#define FT_TX_FRAME_STATUS2_H 0x01C0 /* RESERVED [63:48] : */
                                     /* TX_FRAME_TSTAMP_ID [47:32] */
#define FT_TX_FRAME_STATUS2_L 0x01C4 /* TX_16BYTE_CNT [31:16] : */
                                     /* TX_BYTE_CNT [15:0] */
#define FT_TOTAL_TX_FRAME_CNT_H 0x01C8 /* TOTAL_TX_FRAME_CNT [63:32] */
#define FT_TOTAL_TX_FRAME_CNT_L 0x01CC /* TOTAL_TX_FRAME_CNT [31:0] */
#define FT_TOTAL_TX_BYTE_CNT_H 0x01D0 /* TOTAL_TX_BYTE_CNT [63:32] */
#define FT_TOTAL_TX_BYTE_CNT_L 0x01D4 /* TOTAL_TX_BYTE_CNT [31:0] */
#define FT_TX_TSTAMP1_H 0x01D8 /* TX_TIMESTAMP_1 [63:32] */
#define FT_TX_TSTAMP1_L 0x01DC /* TX_TIMESTAMP_1 [31:0] */
#define FT_TX_TSTAMP2_H 0x01E0 /* TX_TIMESTAMP_2 [63:32] */
#define FT_TX_TSTAMP2_L 0x01E4 /* TX_TIMESTAMP_2 [31:0] */
#define FT_TX_TSTAMP3_H 0x01E8 /* TX_TIMESTAMP_3 [63:32] */
#define FT_TX_TSTAMP3_L 0x01EC /* TX_TIMESTAMP_3 [31:0] */
#define FT_TX_TSTAMP4_H 0x01F0 /* TX_TIMESTAMP_4 [63:32] */
#define FT_TX_TSTAMP4_L 0x01F4 /* TX_TIMESTAMP_4 [31:0] */


#define REG_TSN_VERSION 0x0000
#define REG_TSN_CONFIG 0x0004
#define REG_TSN_CONTROL 0x0008
#define REG_SCRATCH 0x0010

#define REG_RX_PACKETS 0x0100
#define REG_RX_BYTES_HIGH 0x0110
#define REG_RX_BYTES_LOW 0x0114
#define REG_RX_DROP_PACKETS 0x0120
#define REG_RX_DROP_BYTES_HIGH 0x0130
#define REG_RX_DROP_BYTES_LOW 0x0134

#define REG_TX_PACKETS 0x0200
#define REG_TX_BYTES_HIGH 0x0210
#define REG_TX_BYTES_LOW 0x0214
#define REG_TX_DROP_PACKETS 0x0220
#define REG_TX_DROP_BYTES_HIGH 0x0230
#define REG_TX_DROP_BYTES_LOW 0x0234

#define REG_TX_TIMESTAMP_COUNT 0x0300
#define REG_TX_TIMESTAMP1_HIGH 0x0310
#define REG_TX_TIMESTAMP1_LOW 0x0314
#define REG_TX_TIMESTAMP2_HIGH 0x0320
#define REG_TX_TIMESTAMP2_LOW 0x0324
#define REG_TX_TIMESTAMP3_HIGH 0x0330
#define REG_TX_TIMESTAMP3_LOW 0x0334
#define REG_TX_TIMESTAMP4_HIGH 0x0340
#define REG_TX_TIMESTAMP4_LOW 0x0344
#define REG_SYS_COUNT_HIGH 0x0380
#define REG_SYS_COUNT_LOW 0x0384

#define REG_RX_INPUT_PACKET_COUNT 0x0400
#define REG_RX_OUTPUT_PACKET_COUNT 0x0404
#define REG_RX_BUFFER_FULL_DROP_PACKET_COUNT 0x0408
#define REG_TX_INPUT_PACKET_COUNT 0x0410
#define REG_TX_OUTPUT_PACKET_COUNT 0x0414
#define REG_TX_BUFFER_FULL_DROP_PACKET_COUNT 0x0418

#define REG_RPPB_FIFO_STATUS 0x0470
#define REG_RASB_FIFO_STATUS 0x0474
#define REG_TASB_FIFO_STATUS 0x0480
#define REG_TPPB_FIFO_STATUS 0x0484
#define REG_MRIB_DEBUG 0x04A0
#define REG_MTIB_DEBUG 0x04B0

#define REG_TEMAC_STATUS 0x0500
#define REG_TEMAC_RX_STAT 0x0510
#define REG_TEMAC_TX_STAT 0x0514

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

#endif /* DEVICE_DRIVER_LAN8650_REGISTER_H */
