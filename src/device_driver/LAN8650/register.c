#include "register.h"

#include <arpa/inet.h>

#include "arch.h"
#include "spi.h"

uint8_t g_maxpayloadsize;

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
#define FD_RX_FRAME_STATUS1_L 0x000C /* VLAN_TAG [31:16] : FRAME_LENGTH [15:0]	*/
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

#if 0 /* Register Map of 1-Queue TSN IP 20250529 */








FP_FRAME_TICK_FROM_H
FP_FRAME_TICK_FROM_L
FP_FRAME_TICK_TO_H
FP_FRAME_TICK_TO_L
FP_FRAME_TICK_DELAY_FROM_H
FP_FRAME_TICK_DELAY_FROM_L
FP_FRAME_TICK_DELAY_TO_H
FP_FRAME_TICK_DELAY_TO_L
FP_FRAME_STATUS1_H
FP_FRAME_STATUS1_L
FP_FRAME_STATUS2_H
FP_FRAME_STATUS2_L


FSCH_TX_FRAME_STATUS_H
FSCH_TX_FRAME_STATUS_L
FSCH_TOTAL_NEW_ENTRY_CNT_H
FSCH_TOTAL_NEW_ENTRY_CNT_L
FSCH_TOTAL_VALID_ENTRY_CNT_H
FSCH_TOTAL_VALID_ENTRY_CNT_L
FSCH_TOTAL_DELAY_ENTRY_CNT_H
FSCH_TOTAL_DELAY_ENTRY_CNT_L
FSCH_TOTAL_DROP_ENTRY_CNT_H
FSCH_TOTAL_DROP_ENTRY_CNT_L


FBW_BUFFER_WRITE_STATUS1_H
FBW_BUFFER_WRITE_STATUS1_L
FBW_BUFFER_WRITE_STATUS2_H
FBW_BUFFER_WRITE_STATUS2_L
FBW_BUFFER_WRITE_STATUS3_H
FBW_BUFFER_WRITE_STATUS3_L
FBW_ADDR_FIFO_CNT_H
FBW_ADDR_FIFO_CNT_L
FBR_BUFFER_READ_STATUS1_H
FBR_BUFFER_READ_STATUS1_L
FBR_BUFFER_READ_STATUS2_H
FBR_BUFFER_READ_STATUS2_L
FBR_BUFFER_READ_STATUS3_H
FBR_BUFFER_READ_STATUS3_L


FTRSF_TX_FRAME_STATUS_H
FTRSF_TX_FRAME_STATUS_L
FTRSF_GENERAL_STATUS_H
FTRSF_GENERAL_STATUS_L
FTRSF_TOTAL_TX_FRAME_CNT_H
FTRSF_TOTAL_TX_FRAME_CNT_L
FTRSF_TOTAL_META_FIFO_FULL_CNT_H
FTRSF_TOTAL_META_FIFO_FULL_CNT_L
FTRSF_TOTAL_FRAME_FIFO_FULL_CNT_H
FTRSF_TOTAL_FRAME_FIFO_FULL_CNT_L


FT_TX_FRAME_STATUS1_H
FT_TX_FRAME_STATUS1_L
FT_TX_FRAME_STATUS2_H
FT_TX_FRAME_STATUS2_L
FT_TOTAL_TX_FRAME_CNT_H
FT_TOTAL_TX_FRAME_CNT_L
FT_TOTAL_TX_BYTE_CNT_H
FT_TOTAL_TX_BYTE_CNT_L
FT_TX_TSTAMP1_H
FT_TX_TSTAMP1_L
FT_TX_TSTAMP2_H
FT_TX_TSTAMP2_L
FT_TX_TSTAMP3_H
FT_TX_TSTAMP3_L
FT_TX_TSTAMP4_H
FT_TX_TSTAMP4_L


	Register Map of 1-Queue TSN IP
	No.	Class	Addr. Offset	Register	31	30	29	28	27	26	25	24	23	22	21	20	19	18	17	16	15	14	13	12	11	10	9	8	7	6	5	4	3	2	1	0	Note
			"0x0288
~
0x028C"	TSN_SYSCOUNT_H	SYSTEM_COUNT [63:32]																																82th 64-bit Register High  /  slv_reg162
				Reset Value	0x0000_0000
				TSN_SYSCOUNT_L	SYSTEM_COUNT [31:0]																																82th 64-bit Register Low   /  slv_reg163
				Reset Value	0x0000_0000
	4		"0x0290
~
0x0294"	TSN_SYSTEM_CONTROL_H	RESERVED [63:32]																																83th 64-bit Register High  /  slv_reg164
				Reset Value	0x0000_0000
				TSN_SYSTEM_CONTROL_L	RESERVED [31:1]																															TSN ENABLE	83th 64-bit Register Low   /  slv_reg165
				Reset Value	0x0000_0000
	5		"0x0298
~
0x029C"	TSN_RX_CONTROL_H	MAX_FRAME_LENGTH [63:48]																RESERVED [47:45]			MAX_FRAME_FIFO_DATA_COUNT [44:32]													84th 64-bit Register High  /  slv_reg166
				Reset Value	0x0000_0000
	6			TSN_RX_CONTROL_L	RESERVED [31:25]							"MAX_META_FIFO_
DATA_COUNT [24:16]"									RESERVED [15:8]								CLEAR DROP BYTE COUNT	CLEAR DROP FRAME COUNT	CLEAR RX BYTE COUNT	CLEAR RX FRAME COUNT	CHECK-SUM ENABLE	JUMBO DROP	FCS OFFLOAD	VLAN OFFLOAD	84th 64-bit Register Low   /  slv_reg167
				Reset Value	0x0000_0000
	7		"0x02A0
~
0x02A4"	TSN_TX_CONTROL_H	RESERVED [63:32]																																85th 64-bit Register High  /  slv_reg168
				Reset Value	0x0000_0000
	8			TSN_TX_CONTROL_L	ETHERNET_INTER_FRAME_GAP [31:16]																RESERVED [15:10]						HOST_FIFO_FULL_LIMIT [9:0]										85th 64-bit Register Low   /  slv_reg169
				Reset Value	0x0000_0000
	9		"0x02A8
~
0x02AC"	PPS_PULSE_AT_H	PULSE_AT [63:32]																																86th 64-bit Register High  /  slv_reg170
				Reset Value	0x0000_0000
	10			PPS_PULSE_AT_L	PULSE_AT [31:0]																																86th 64-bit Register Low   /  slv_reg171
				Reset Value	0x0000_0000
	11		"0x02B0
~
0x02B4"	PPS_CYCLE_1S_H	CYCLE_1S [63:32]																																87th 64-bit Register High  /  slv_reg172
				Reset Value	0x0000_0000
	13			PPS_CYCLE_1S_L	CYCLE_1S [31:0]																																87th 64-bit Register Low   /  slv_reg173
				Reset Value	0x0000_0000

	1	"Rx

(Frame
Decoder)"	"0x0000
~
0x0004"	FD_RX_TSTAMP_H	RX_TSTAMP [63:32]																																1th 64-bit Register High  /  slv_reg0
				Reset Value	0x0000_0000
	2			FD_RX_TSTAMP_L	RX_TSTAMP [31:0]																																1th 64-bit Register Low   /  slv_reg1
				Reset Value	0x0000_0000
	3		"0x0008
~
0x000C"	FD_RX_FRAME_STATUS1_H	CHECK_SUM [63:32]																																2th 64-bit Register High  /  slv_reg2
				Reset Value	0x0000_0000
	4			FD_RX_FRAME_STATUS1_L	VLAN_TAG [31:16]																FRAME_LENGTH [15:0]																2th 64-bit Register Low   /  slv_reg3
				Reset Value	0x0000_0000
	5		"0x0010
~
0x0014"	FD_RX_FRAME_STATUS2_H	RESERVED [63:32]																																3th 64-bit Register High  /  slv_reg4
				Reset Value	-
	6			FD_RX_FRAME_STATUS2_L	RESERVED [31:5]																											VLAN OFFLOAD FLAG	FCS OFFLOAD FLAG	FCS ERROR FLAG	JUMBO ERROR FLAG	FIFO FULL FLAG	3th 64-bit Register Low   /  slv_reg5
				Reset Value	-																											0	0	0	0	0
	7		"0x0018
~
0x001C"	FD_TOTAL_RX_FRAME_CNT_H	TOTAL_RX_FRAME_CNT [63:32]																																4th 64-bit Register High  /  slv_reg6
				Reset Value	0x0000_0000
	8			FD_TOTAL_RX_FRAME_CNT_L	TOTAL_RX_FRAME_CNT [31:0]																																4th 64-bit Register Low   /  slv_reg7
				Reset Value	0x0000_0000
	9		"0x0020
~
0x0024"	FD_TOTAL_RX_BYTE_CNT_H	TOTAL_RX_BYTE_CNT [63:32]																																5th 64-bit Register High  /  slv_reg8
				Reset Value	0x0000_0000
	10			FD_TOTAL_RX_BYTE_CNT_L	TOTAL_RX_BYTE_CNT [31:0]																																5th 64-bit Register Low   /  slv_reg9
				Reset Value	0x0000_0000
	11		"0x0028
~
0x002C"	FD_TOTAL_DROP_FRAME_CNT_H	TOTAL_DROP_FRAME_CNT [63:32]																																6th 64-bit Register High  /  slv_reg10
				Reset Value	0x0000_0000
	12			FD_TOTAL_DROP_FRAME_CNT_L	TOTAL_DROP_FRAME_CNT [31:0]																																6th 64-bit Register Low   /  slv_reg11
				Reset Value	0x0000_0000
	13		"0x0030
~
0x0034"	FD_TOTAL_DROP_BYTE_CNT_H	TOTAL_DROP_BYTE_CNT [63:32]																																7th 64-bit Register High  /  slv_reg12
				Reset Value	0x0000_0000
	14			FD_TOTAL_DROP_BYTE_CNT_L	TOTAL_DROP_BYTE_CNT [31:0]																																7th 64-bit Register Low   /  slv_reg13
				Reset Value	0x0000_0000
	15		"0x0038
~
0x003C"	FD_FCS_DROP_FRAME_CNT_H	FCS_DROP_FRAME_CNT [63:32]																																8th 64-bit Register High  /  slv_reg14
				Reset Value	0x0000_0000
	16			FD_FCS_DROP_FRAME_CNT_L	FCS_DROP_FRAME_CNT [31:0]																																8th 64-bit Register Low   /  slv_reg15
				Reset Value	0x0000_0000
	17		"0x0040
~
0x0044"	FD_FCS_DROP_BYTE_CNT_H	FCS_DROP_BYTE_CNT [63:32]																																9th 64-bit Register High  /  slv_reg16
				Reset Value	0x0000_0000
	18			FD_FCS_DROP_BYTE_CNT_L	FCS_DROP_BYTE_CNT [31:0]																																9th 64-bit Register Low   /  slv_reg17
				Reset Value	0x0000_0000
	19		"0x0048
~
0x004C"	FD_JUMBO_DROP_FRAME_CNT_H	JUMBO_DROP_FRAME_CNT [63:32]																																10th 64-bit Register High  /  slv_reg18
				Reset Value	0x0000_0000
	20			FD_JUMBO_DROP_FRAME_CNT_L	JUMBO_DROP_FRAME_CNT [31:0]																																10th 64-bit Register Low   /  slv_reg19
				Reset Value	0x0000_0000
	21		"0x0050
~
0x0054"	FD_JUMBO_DROP_BYTE_CNT_H	JUMBO_DROP_BYTE_CNT [63:32]																																11th 64-bit Register High  /  slv_reg20
				Reset Value	0x0000_0000
	22			FD_JUMBO_DROP_BYTE_CNT_L	JUMBO_DROP_BYTE_CNT [31:0]																																11th 64-bit Register Low   /  slv_reg21
				Reset Value	0x0000_0000
	23		"0x0058
~
0x005C"	FD_FIFO_FULL_DROP_FRAME_CNT_H	FIFO_FULL_DROP_FRAME_CNT [63:32]																																12th 64-bit Register High  /  slv_reg22
				Reset Value	0x0000_0000
	24			FD_FIFO_FULL_DROP_FRAME_CNT_L	FIFO_FULL_DROP_FRAME_CNT [31:0]																																12th 64-bit Register Low   /  slv_reg23
				Reset Value	0x0000_0000
	25		"0x0060
~
0x0064"	FD_FIFO_FULL_DROP_BYTE_CNT_H	FIFO_FULL_DROP_BYTE_CNT [63:32]																																13th 64-bit Register High  /  slv_reg24
				Reset Value	0x0000_0000
	26			FD_FIFO_FULL_DROP_BYTE_CNT_L	FIFO_FULL_DROP_BYTE_CNT [31:0]																																13th 64-bit Register Low   /  slv_reg25
				Reset Value	0x0000_0000
	27		"0x0068
~
0x006C"																																		14th 64-bit Register High  /  slv_reg26
				Reset Value
	28																																				14th 64-bit Register Low   /  slv_reg27
				Reset Value
	29		"0x0070
~
0x0074"																																		15th 64-bit Register High  /  slv_reg28
				Reset Value
	30																																				15th 64-bit Register Low   /  slv_reg29
				Reset Value
	31		"0x0078
~
0x007C"																																		16th 64-bit Register High  /  slv_reg30
				Reset Value
	32																																				16th 64-bit Register Low   /  slv_reg31
				Reset Value
	33	"Rx

(Metadata
Generator)"	"0x0080
~
0x0084"																																		17th 64-bit Register High  /  slv_reg32
				Reset Value
	34																																				17th 64-bit Register Low   /  slv_reg33
				Reset Value
	35		"0x0088
~
0x008C"																																		18th 64-bit Register High  /  slv_reg34
				Reset Value
	36																																				18th 64-bit Register Low   /  slv_reg35
				Reset Value
	37	"Rx

(Rx Frame
Transmitter)"	"0x0090
~
0x0094"																																		19th 64-bit Register High  /  slv_reg36
				Reset Value
	38																																				19th 64-bit Register Low   /  slv_reg37
				Reset Value
	39		"0x0098
~
0x009C"																																		20th 64-bit Register High  /  slv_reg38
				Reset Value
	40																																				20th 64-bit Register Low   /  slv_reg39
				Reset Value
	41		"0x00A0
~
0x00A4"																																		21th 64-bit Register High  /  slv_reg40
				Reset Value
	42																																				21th 64-bit Register Low   /  slv_reg41
				Reset Value
	42	"FIFO

(META,
FRAME)"	"0x00A8
~
0x00AC"	TSN_RX_FIFO_STATUS_H	RESERVED [63:32]																																22th 64-bit Register High  /  slv_reg42
				Reset Value	0x0000_0000
	42			TSN_RX_FIFO_STATUS_L	RES [31:29]			FRAME_FIFO_DATA_CNT [28:16]													RESERVED [15:9]							META_FIFO_DATA_CNT [8:0]									22th 64-bit Register Low   /  slv_reg43
				Reset Value	0x0000_0000



	1	"Tx

(Frame
Stacker)"	"0x00B0
~
0x00B4"	FS_GENERAL_STATUS_H	RESERVED [63:58]						HOST_FIFO_RD_CNT [57:48]										RESERVED [47:42]						HOST_FIFO_WR_CNT [41:32]										23th 64-bit Register High  /  slv_reg44
				Reset Value	0x0000_0000
	1			FS_GENERAL_STATUS_L	RESERVED [31:4]																												XDMA AXIS TREADY	HOST FIFO USER FULL	HOST FIFO EMPTY	HOST FIFO FULL	23th 64-bit Register Low   /  slv_reg45
				Reset Value	0x0000_0000
	1		"0x00B8
~
0x00BC"	FS_TOTAL_RX_FRAME_CNT_H	TOTAL_RX_FRAME_CNT [63:32]																																24th 64-bit Register High  /  slv_reg46
				Reset Value	0x0000_0000
	2			FS_TOTAL_RX_FRAME_CNT_L	TOTAL_RX_FRAME_CNT [31:0]																																24th 64-bit Register Low   /  slv_reg47
				Reset Value	0x0000_0000
	3		"0x00C0
~
0x00C4"	FS_TOTAL_RX_16BYTE_CNT_H	TOTAL_RX_16BYTE_CNT [63:32]																																25th 64-bit Register High  /  slv_reg48
				Reset Value	0x0000_0000
	4			FS_TOTAL_RX_16BYTE_CNT_L	TOTAL_RX_16BYTE_CNT [31:0]																																25th 64-bit Register Low   /  slv_reg49
				Reset Value	0x0000_0000
	5		"0x00C8
~
0x00CC"	FS_TOTAL_BACK_PRESSURE_EVENT_CNT_H	TOTAL_BACK_PRESSURE_EVENT_CNT [63:32]																																26th 64-bit Register High  /  slv_reg50
				Reset Value	0x0000_0000
	6			FS_TOTAL_BACK_PRESSURE_EVENT_CNT_L	TOTAL_BACK_PRESSURE_EVENT_CNT [31:0]																																26th 64-bit Register Low   /  slv_reg51
				Reset Value	0x0000_0000
	9		"0x00D0
~
0x00D4"																																		27th 64-bit Register High  /  slv_reg52
				Reset Value
	10																																				27th 64-bit Register Low   /  slv_reg53
				Reset Value
	11		"0x00D8
~
0x00DC"																																		28th 64-bit Register High  /  slv_reg54
				Reset Value
	12																																				28th 64-bit Register Low   /  slv_reg55
				Reset Value
	13	"Tx

(Frame
Parser)"	"0x00E0
~
0x00E4"	FP_FRAME_TICK_FROM_H	TICK_FROM [63:32]																																29th 64-bit Register High  /  slv_reg56
				Reset Value	0x0000_0000
	14			FP_FRAME_TICK_FROM_L	TICK_FROM [31:0]																																29th 64-bit Register Low   /  slv_reg57
				Reset Value	0x0000_0000
	15		"0x00E8
~
0x00EC"	FP_FRAME_TICK_TO_H	TICK_TO [63:32]																																30th 64-bit Register High  /  slv_reg58
				Reset Value	0x0000_0000
	16			FP_FRAME_TICK_TO_L	TICK_TO [31:0]																																30th 64-bit Register Low   /  slv_reg59
				Reset Value	0x0000_0000
	17		"0x00F0
~
0x00F4"	FP_FRAME_TICK_DELAY_FROM_H	TICK_DELAY_FROM [63:32]																																31th 64-bit Register High  /  slv_reg60
				Reset Value	0x0000_0000
	18			FP_FRAME_TICK_DELAY_FROM_L	TICK_DELAY_FROM [31:0]																																31th 64-bit Register Low   /  slv_reg61
				Reset Value	0x0000_0000
	19		"0x00F8
~
0x00FC"	FP_FRAME_TICK_DELAY_TO_H	TICK_DELAY_TO [63:32]																																32th 64-bit Register High  /  slv_reg62
				Reset Value	0x0000_0000
	20			FP_FRAME_TICK_DELAY_TO_L	TICK_DELAY_TO [31:0]																																32th 64-bit Register Low   /  slv_reg63
				Reset Value	0x0000_0000
	21		"0x0100
~
0x0104"	FP_FRAME_STATUS1_H	RESERVED [63:56]								ALLOC_ADDR [55:48]								RESERVED [47:33]															FAIL_POLICY [32]	33th 64-bit Register High  /  slv_reg64
				Reset Value	0x0000_0000
	22			FP_FRAME_STATUS1_L	FRAME_LENGTH [31:16]																TIMESTAMP_ID [15:0]																33th 64-bit Register Low   /  slv_reg65
				Reset Value	0x0000_0000
	23		"0x0108
~
0x010C"	FP_FRAME_STATUS2_H	RESERVED [63:52]												META_AXIS_HSK_CNT [51:49]			META_ADDR_OFFSET [48]	RESERVED [47:39]									"FRAME_ADDR_
OFFSET [38:32]"							34th 64-bit Register High  /  slv_reg66
				Reset Value	0x0000_0000
	24			FP_FRAME_STATUS2_L	FRAME_AXIS_HSK_CNT [31:16]																RX_FRAME_16BYTE_CNT [15:0]																34th 64-bit Register Low   /  slv_reg67
				Reset Value	0x0000_0000
	25		"0x0110
~
0x0114"																																		35th 64-bit Register High  /  slv_reg68
				Reset Value	0x0000_0000
	26																																				35th 64-bit Register Low   /  slv_reg69
				Reset Value	0x0000_0000
	41	"Tx

(Frame
Scheduler)"	"0x0118
~
0x011C"	FSCH_TX_FRAME_STATUS_H	RESERVED [63:48]																TX_FRAME_TSTAMP_ID [47:32]																36th 64-bit Register High  /  slv_reg70
				Reset Value	0x0000_0000
	42			FSCH_TX_FRAME_STATUS_L	TX_FRAME_LENGTH [15:0]																RESERVED [15:8]								TX_FRAME_ADDRESS [7:0]								36th 64-bit Register Low   /  slv_reg71
				Reset Value	0x0000_0000
	43		"0x0120
~
0x0124"	FSCH_TOTAL_NEW_ENTRY_CNT_H	TOTAL_NEW_ENTRY_CNT [63:32]																																37th 64-bit Register High  /  slv_reg72
				Reset Value	0x0000_0000
	44			FSCH_TOTAL_NEW_ENTRY_CNT_L	TOTAL_NEW_ENTRY_CNT [31:0]																																37th 64-bit Register Low   /  slv_reg73
				Reset Value	0x0000_0000
	45		"0x0128
~
0x012C"	FSCH_TOTAL_VALID_ENTRY_CNT_H	TOTAL_VALID_ENTRY_CNT [63:32]																																38th 64-bit Register High  /  slv_reg74
				Reset Value	0x0000_0000
	46			FSCH_TOTAL_VALID_ENTRY_CNT_L	TOTAL_VALID_ENTRY_CNT [31:0]																																38th 64-bit Register Low   /  slv_reg75
				Reset Value	0x0000_0000
	47		"0x0130
~
0x0134"	FSCH_TOTAL_DELAY_ENTRY_CNT_H	TOTAL_DELAY_ENTRY_CNT [63:32]																																39th 64-bit Register High  /  slv_reg76
				Reset Value	0x0000_0000
	48			FSCH_TOTAL_DELAY_ENTRY_CNT_L	TOTAL_DELAY_ENTRY_CNT [31:0]																																39th 64-bit Register Low   /  slv_reg77
				Reset Value	0x0000_0000
	49		"0x0138
~
0x013C"	FSCH_TOTAL_DROP_ENTRY_CNT_H	TOTAL_DROP_ENTRY_CNT [63:32]																																40th 64-bit Register High  /  slv_reg78
				Reset Value	0x0000_0000
	50			FSCH_TOTAL_DROP_ENTRY_CNT_L	TOTAL_DROP_ENTRY_CNT [31:0]																																40th 64-bit Register Low   /  slv_reg79
				Reset Value	0x0000_0000
	49		"0x0140
~
0x0144"																																		41th 64-bit Register High  /  slv_reg80
				Reset Value
	50																																				41th 64-bit Register Low   /  slv_reg81
				Reset Value
	27	"Tx

(Frame
Buffer)"	"0x0148
~
0x014C"	FBW_BUFFER_WRITE_STATUS1_H	RESERVED [63:40]																								ADDR_FIFO_DATA_CNT [39:32]								42th 64-bit Register High  /  slv_reg82
				Reset Value	0x0000_0000
	28			FBW_BUFFER_WRITE_STATUS1_L	RESERVED [31:11]																					BRAM_SEL [10:8]			ALLOC_ADDR [7:0]								42th 64-bit Register Low   /  slv_reg83
				Reset Value	0x0000_0000
	29		"0x0150
~
0x0154"	FBW_BUFFER_WRITE_STATUS2_H	RESERVED [63:59]					BRAM3_WR_ADDR_LAST [58:48]											RESERVED [47:43]					BRAM2_WR_ADDR_LAST [42:32]											43th 64-bit Register High  /  slv_reg84
				Reset Value	0x0000_0000
	30			FBW_BUFFER_WRITE_STATUS2_L	RESERVED [31:27]					BRAM1_WR_ADDR_LAST [26:16]											RESERVED [15:11]					BRAM0_WR_ADDR_LAST [10:0]											43th 64-bit Register Low   /  slv_reg85
				Reset Value	0x0000_0000
	31		"0x0158
~
0x015C"	FBW_BUFFER_WRITE_STATUS3_H	RESERVED [63:59]					BRAM7_WR_ADDR_LAST [58:48]											RESERVED [47:43]					BRAM6_WR_ADDR_LAST [42:32]											44th 64-bit Register High  /  slv_reg86
				Reset Value	0x0000_0000
	32			FBW_BUFFER_WRITE_STATUS3_L	RESERVED [31:27]					BRAM5_WR_ADDR_LAST [26:16]											RESERVED [15:11]					BRAM4_WR_ADDR_LAST [10:0]											44th 64-bit Register Low   /  slv_reg87
				Reset Value	0x0000_0000
			"0x0160
~
0x0164"	FBW_ADDR_FIFO_CNT_H	RESERVED [63:32]																																45th 64-bit Register High  /  slv_reg88
				Reset Value	0x0000_0000
				FBW_ADDR_FIFO_CNT_L	RESERVED [31:8]																								ADDR_FIFO_DATA_CNT [7:0]								45th 64-bit Register Low   /  slv_reg89
				Reset Value	0x0000_0000
	33		"0x0168
~
0x016C"	FBR_BUFFER_READ_STATUS1_H	RESERVED [63:40]																								DROP_FRAME_ADDR [39:32]								46th 64-bit Register High  /  slv_reg90
				Reset Value	0x0000_0000
	34			FBR_BUFFER_READ_STATUS1_L	SELECTED_FRAME_16BYTE_CNT [31:16]																RESERVED [15:11]					BRAM_SEL [2:0]			SELECTED_FRAME_ADDR [7:0]								46th 64-bit Register Low   /  slv_reg91
				Reset Value	0x0000_0000
	35		"0x0170
~
0x0174"	FBR_BUFFER_READ_STATUS2_H	RESERVED [63:59]					BRAM3_RD_ADDR_LAST [58:48]											RESERVED [47:43]					BRAM2_RD_ADDR_LAST [42:32]											47th 64-bit Register High  /  slv_reg92
				Reset Value	0x0000_0000
	36			FBR_BUFFER_READ_STATUS2_L	RESERVED [31:27]					BRAM1_RD_ADDR_LAST [26:16]											RESERVED [15:11]					BRAM0_RD_ADDR_LAST [10:0]											47th 64-bit Register Low   /  slv_reg93
				Reset Value	0x0000_0000
	37		"0x0178
~
0x017C"	FBR_BUFFER_READ_STATUS3_H	RESERVED [63:59]					BRAM7_RD_ADDR_LAST [58:48]											RESERVED [47:43]					BRAM6_RD_ADDR_LAST [42:32]											48th 64-bit Register High  /  slv_reg94
				Reset Value	0x0000_0000
	38			FBR_BUFFER_READ_STATUS3_L	RESERVED [31:27]					BRAM5_RD_ADDR_LAST [26:16]											RESERVED [15:11]					BRAM4_RD_ADDR_LAST [10:0]											48th 64-bit Register Low   /  slv_reg95
				Reset Value	0x0000_0000
	39		"0x0180
~
0x0184"																																		49th 64-bit Register High  /  slv_reg96
				Reset Value	0x0000_0000
	40																																				49th 64-bit Register Low   /  slv_reg97
				Reset Value	0x0000_0000
	49	"Tx

(Frame
Transfer
FSM)"	"0x0188
~
0x018C"	FTRSF_TX_FRAME_STATUS_H	TX_METADATA [63:32]																																50th 64-bit Register High  /  slv_reg98
				Reset Value	0x0000_0000
	50			FTRSF_TX_FRAME_STATUS_L	TX_FRAME_LENGTH [15:0]																RESERVED [15:8]								TX_FRAME_ADDRESS [7:0]								50th 64-bit Register Low   /  slv_reg99
				Reset Value	0x0000_0000
	49		"0x0190
~
0x0194"	FTRSF_GENERAL_STATUS_H	RESERVED [63:35]																													TX_FRAME_FIFO_FULL [34]	TX_META_FIFO_FULL [33]	TX_STATUS [32]	51th 64-bit Register High  /  slv_reg100
				Reset Value	0x0000_0000
	50			FTRSF_GENERAL_STATUS_L	RESERVED [31:25]							"TX_FRAME_FIFO_
DATA_COUNT [24:16]"									RESERVED [15:9]							TX_META_FIFO_DATA_COUNT [8:0]									51th 64-bit Register Low   /  slv_reg101
				Reset Value	0x0000_0000
	49		"0x0198
~
0x019C"	FTRSF_TOTAL_TX_FRAME_CNT_H	TOTAL_TX_FRAME_CNT [63:32]																																52th 64-bit Register High  /  slv_reg102
				Reset Value	0x0000_0000
	50			FTRSF_TOTAL_TX_FRAME_CNT_L	TOTAL_TX_FRAME_CNT [31:0]																																52th 64-bit Register Low   /  slv_reg103
				Reset Value	0x0000_0000
	49		"0x01A0
~
0x01A4"	FTRSF_TOTAL_META_FIFO_FULL_CNT_H	TOTAL_META_FIFO_FULL_CNT [63:32]																																53th 64-bit Register High  /  slv_reg104
				Reset Value	0x0000_0000
	50			FTRSF_TOTAL_META_FIFO_FULL_CNT_L	TOTAL_META_FIFO_FULL_CNT [31:0]																																53th 64-bit Register Low   /  slv_reg105
				Reset Value	0x0000_0000
	49		"0x01A8
~
0x01AC"	FTRSF_TOTAL_FRAME_FIFO_FULL_CNT_H	TOTAL_FRAME_FIFO_FULL_CNT [63:32]																																54th 64-bit Register High  /  slv_reg106
				Reset Value	0x0000_0000
	50			FTRSF_TOTAL_FRAME_FIFO_FULL_CNT_L	TOTAL_FRAME_FIFO_FULL_CNT [31:0]																																54th 64-bit Register Low   /  slv_reg107
				Reset Value	0x0000_0000
	49		"0x01B0
~
0x01B4"																																		55th 64-bit Register High  /  slv_reg108
				Reset Value
	50																																				55th 64-bit Register Low   /  slv_reg109
				Reset Value
	49	"Tx

(Frame
Transmitter
FSM)"	"0x01B8
~
0x01BC"	FT_TX_FRAME_STATUS1_H	TX_METADATA [63:32]																																56th 64-bit Register High  /  slv_reg110
				Reset Value	0x0000_0000
	50			FT_TX_FRAME_STATUS1_L	TX_FRAME_16BYTE_LENGTH [31:16]																TX_FRAME_LENGTH [15:0]																56th 64-bit Register Low   /  slv_reg111
				Reset Value	0x0000_0000
	49		"0x01C0
~
0x01C4"	FT_TX_FRAME_STATUS2_H	RESERVED [63:48]																TX_FRAME_TSTAMP_ID [47:32]																57th 64-bit Register High  /  slv_reg112
				Reset Value	0x0000_0000
	50			FT_TX_FRAME_STATUS2_L	TX_16BYTE_CNT [31:16]																TX_BYTE_CNT [15:0]																57th 64-bit Register Low   /  slv_reg113
				Reset Value	0x0000_0000
	49		"0x01C8
~
0x01CC"	FT_TOTAL_TX_FRAME_CNT_H	TOTAL_TX_FRAME_CNT [63:32]																																58th 64-bit Register High  /  slv_reg114
				Reset Value	0x0000_0000
	50			FT_TOTAL_TX_FRAME_CNT_L	TOTAL_TX_FRAME_CNT [31:0]																																58th 64-bit Register Low   /  slv_reg115
				Reset Value	0x0000_0000
	49		"0x01D0
~
0x01D4"	FT_TOTAL_TX_BYTE_CNT_H	TOTAL_TX_BYTE_CNT [63:32]																																59th 64-bit Register High  /  slv_reg116
				Reset Value	0x0000_0000
	50			FT_TOTAL_TX_BYTE_CNT_L	TOTAL_TX_BYTE_CNT [31:0]																																59th 64-bit Register Low   /  slv_reg117
				Reset Value	0x0000_0000
	49		"0x01D8
~
0x01DC"	FT_TX_TSTAMP1_H	TX_TIMESTAMP_1 [63:32]																																60th 64-bit Register High  /  slv_reg118
				Reset Value	0x0000_0000
	50			FT_TX_TSTAMP1_L	TX_TIMESTAMP_1 [31:0]																																60th 64-bit Register Low   /  slv_reg119
				Reset Value	0x0000_0000
	49		"0x01E0
~
0x01E4"	FT_TX_TSTAMP2_H	TX_TIMESTAMP_2 [63:32]																																61th 64-bit Register High  /  slv_reg120
				Reset Value	0x0000_0000
	50			FT_TX_TSTAMP2_L	TX_TIMESTAMP_2 [31:0]																																61th 64-bit Register Low   /  slv_reg121
				Reset Value	0x0000_0000
	49		"0x01E8
~
0x01EC"	FT_TX_TSTAMP3_H	TX_TIMESTAMP_3 [63:32]																																62th 64-bit Register High  /  slv_reg122
				Reset Value	0x0000_0000
	50			FT_TX_TSTAMP3_L	TX_TIMESTAMP_3 [31:0]																																62th 64-bit Register Low   /  slv_reg123
				Reset Value	0x0000_0000
	49		"0x01F0
~
0x01F4"	FT_TX_TSTAMP4_H	TX_TIMESTAMP_4 [63:32]																																63th 64-bit Register High  /  slv_reg124
				Reset Value	0x0000_0000
	50			FT_TX_TSTAMP4_L	TX_TIMESTAMP_4 [31:0]																																63th 64-bit Register Low   /  slv_reg125
				Reset Value	0x0000_0000
	49		"0x01F8
~
0x01FC"																																		64th 64-bit Register High  /  slv_reg126
				Reset Value
	50																																				64th 64-bit Register Low   /  slv_reg127
				Reset Value





















































































































































































































































































































































































































































































































































































































































































































































































































































#endif

struct reginfo reg_general[] = {{"TSN version", REG_TSN_VERSION},   
#if 0
                                {"TSN Configuration", REG_TSN_CONFIG},
                                {"TSN Control", REG_TSN_CONTROL},   {"@sys count", REG_SYS_COUNT_HIGH},
                                {"TEMAC status", REG_TEMAC_STATUS}, 
#endif
                                {"", -1}};

struct reginfo reg_rx[] = {{"rx packets", REG_RX_PACKETS},
                           {"@rx bytes", REG_RX_BYTES_HIGH},
                           {"rx drop packets", REG_RX_DROP_PACKETS},
                           {"@rx drop bytes", REG_RX_DROP_BYTES_HIGH},
                           {"rx input packet counter", REG_RX_INPUT_PACKET_COUNT},
                           {"rx output packet counter", REG_RX_OUTPUT_PACKET_COUNT},
                           {"rx buffer full drop packet count", REG_RX_BUFFER_FULL_DROP_PACKET_COUNT},
                           {"RPPB FIFO status", REG_RPPB_FIFO_STATUS},
                           {"RASB FIFO status", REG_RASB_FIFO_STATUS},
                           {"MRIB debug", REG_MRIB_DEBUG},
                           {"TEMAC rx statistics", REG_TEMAC_RX_STAT},
                           {"", -1}};

struct reginfo reg_tx[] = {{"tx packets", REG_TX_PACKETS},
                           {"@tx bytes", REG_TX_BYTES_HIGH},
                           {"tx drop packets", REG_TX_DROP_PACKETS},
                           {"@tx drop bytes", REG_TX_DROP_BYTES_HIGH},
                           {"tx timestamp count", REG_TX_TIMESTAMP_COUNT},
                           {"@tx timestamp 1", REG_TX_TIMESTAMP1_HIGH},
                           {"@tx timestamp 2", REG_TX_TIMESTAMP2_HIGH},
                           {"@tx timestamp 3", REG_TX_TIMESTAMP3_HIGH},
                           {"@tx timestamp 4", REG_TX_TIMESTAMP4_HIGH},
                           {"tx input packet counter", REG_TX_INPUT_PACKET_COUNT},
                           {"tx output packet counter", REG_TX_OUTPUT_PACKET_COUNT},
                           {"tx buffer full drop packet count", REG_TX_BUFFER_FULL_DROP_PACKET_COUNT},
                           {"TASB FIFO status", REG_TASB_FIFO_STATUS},
                           {"TPPB FIFO status", REG_TPPB_FIFO_STATUS},
                           {"MTIB debug", REG_MTIB_DEBUG},
                           {"TEMAC tx statistics", REG_TEMAC_TX_STAT},
                           {"", -1}};

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
#if 0
    if (mode == PLCA_MODE_COORDINATOR) {
//        write_register(MMS4, PLCA_CTRL1, 0x00000800); // Coordinator(node 0), 2 nodes
//        write_register(MMS1, MAC_SAB1, 0x11111111);   // Configure MAC Address (Temporary)
//        write_register(MMS1, MAC_SAT1, 0x00001111);   // Configure MAC Address (Temporary)
    } else if (mode == PLCA_MODE_FOLLOWER) {
//        write_register(MMS4, PLCA_CTRL1, 0x00000801); // Follower, node 1
//        write_register(MMS1, MAC_SAB1, 0x22222222);   // Configure MAC Address (Temporary)
//        write_register(MMS1, MAC_SAT1, 0x00002222);   // Configure MAC Address (Temporary)
    } else {
        printf("Invalid mode: %d\n", mode);
        return false;
    }
#endif
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
        // printf("Parity Error READMACPHYReg header value : 0x%08x\n", commandheader_echoed.ctrl_frame_head);
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

#define RX_FRAME_FIFO_BASE 0x1000
#define TX_FRAME_FIFO_BASE 0x2000
#define REG_READ_FIFO_BASE 0x3000
#define REG_WRITE_FIFO_BASE 0x4000
#define METADATA_FIFO_BASE 0xF000

#define SPI_CMD_WRITE 0x01
#define SPI_CMD_READ 0x02

#define SPI_READ_CMD_LENGTH 7
#define SPI_WRITE_CMD_LENGTH 7
#define SPI_READ_DATA_OFFSET 3

static bool mpw_hw_writereg(struct mpw_ctrl_cmd_reg* p_regData) {
    uint8_t numberof_bytestosend = 0;
    bool writestatus = true;
    bool execution_status = true;
    uint8_t txbuffer[MAX_PAYLOAD_BYTE + HEADER_SIZE] = {0};
    uint8_t rxbuffer[MAX_PAYLOAD_BYTE + HEADER_SIZE] = {0};

    txbuffer[0] = p_regData->cmd;
    txbuffer[1] = (uint8_t)((p_regData->address & 0xFF00) >> 8);
    txbuffer[2] = (uint8_t)(p_regData->address & 0xFF);

    memcpy(&txbuffer[3], &p_regData->databuffer[0], sizeof(uint32_t));

    numberof_bytestosend = SPI_WRITE_CMD_LENGTH;

    printf("%s\n", __func__);
    for(int id = 0; id < SPI_WRITE_CMD_LENGTH; id++) {
        printf("txbuffer[%02d] : 0x%02x\n", id, txbuffer[id] & 0xFF);
    }

    spi_transfer(rxbuffer, txbuffer, numberof_bytestosend);

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

uint32_t write_register_mpw(uint8_t MMS, uint16_t address, uint32_t data) {
    struct mpw_ctrl_cmd_reg writereg_input;
    bool execution_status = false;

    printf("%s - MMS: %d, address: 0x%04x, data: 0x%08x\n", __func__, MMS, address, data);

    writereg_input.cmd = SPI_CMD_WRITE;
    writereg_input.length = 0;
    // writereg_input.address = REG_WRITE_FIFO_BASE | (address & 0xFFF);
    writereg_input.address = RX_FRAME_FIFO_BASE| (address & 0xFFF);
    writereg_input.databuffer[0] = ntohl(data);

    execution_status = mpw_hw_writereg(&writereg_input);
    if (execution_status == true) {
        return writereg_input.databuffer[0];
    } else {
        printf("ERROR: Register Write failed at MMS %d, Address %4x\n", MMS, address);
        return 0;
    }
}

static bool mpw_hw_readreg(struct mpw_ctrl_cmd_reg* p_regInfoInput, struct mpw_ctrl_cmd_reg* p_readRegData) {
    uint8_t txbuffer[MAX_REG_DATA_ONECONTROLCMD + HEADER_SIZE + REG_SIZE] = {0};
    uint8_t rxbuffer[MAX_REG_DATA_ONECONTROLCMD + HEADER_SIZE + REG_SIZE] = {0};
    uint16_t numberof_bytestosend = 0;
    uint32_t reg_val;

    txbuffer[0] = p_regInfoInput->cmd;                                /* Command */
    txbuffer[1] = (uint8_t)((p_regInfoInput->address & 0xFF00) >> 8); /* Target Address[15:8] */
    txbuffer[2] = (uint8_t)(p_regInfoInput->address & 0xFF);          /* Target Address[7:0] */

    numberof_bytestosend = 10; // SPI_READ_CMD_LENGTH; /* Command(1) + Target Address(2) + Register Value(4) */

    printf("%s\n", __func__);
    for(int id=0; id< 10; id++) {
        printf("txbuffer[%2d]: 0x%02x\n", id, txbuffer[id] & 0xff);
    }

    for(int id=0; id<10; id++) {
        printf("rxbuffer[%2d]: 0x%02x\n", id, rxbuffer[id] & 0xff);
    }

    spi_transfer(rxbuffer, txbuffer, numberof_bytestosend);

    for(int id=0; id<10; id++) {
        printf("rxbuffer[%2d]: 0x%02x\n", id, rxbuffer[id] & 0xff);
    }

    return true;

    memcpy((uint8_t*)&reg_val, &rxbuffer[SPI_READ_DATA_OFFSET], sizeof(uint32_t));

    p_readRegData->databuffer[0] = ntohl(reg_val);

    return true;
}

uint32_t read_register(uint8_t mms, uint16_t address) {
    bool execution_status = false;
    struct mpw_ctrl_cmd_reg readreg_infoinput;
    struct mpw_ctrl_cmd_reg readreg_data;
    readreg_infoinput.cmd = SPI_CMD_READ;
    readreg_infoinput.length = 0;
    // readreg_infoinput.address = REG_READ_FIFO_BASE | (address & 0xFFF);
    readreg_infoinput.address = RX_FRAME_FIFO_BASE | (address & 0xFFF);

    execution_status = mpw_hw_readreg(&readreg_infoinput, &readreg_data);
    if (execution_status == true) {
        return readreg_data.databuffer[0];
    } else {
        // printf("ERROR: Register Read failed at MMS %d, Address %4x\n", mms, address);
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
        if (reginfo[i].desc[0] == '@') {
            uint64_t ll = read_register(mms, reginfo[i].address);
            ll = (ll << 32) | read_register(mms, reginfo[i].address + 4);
            printf("address: 0x%04x - value: 0x%016x - %s\n", reginfo[i].address, ll, &(reginfo[i].desc[1]));
        } else {
            printf("address: 0x%04x - value: 0x%08x - %s\n", reginfo[i].address,
                   read_register(mms, (uint16_t)reginfo[i].address), reginfo[i].desc);
        }
    }
}

/* read all register values in memory map selector */
static int read_register_in_mms(uint8_t mms) {
    switch (mms) {
    case MMS0: /* General Registers */
        dump_reginfo(mms, reg_general);
        break;
    case MMS1: /* Rx Registers */
        dump_reginfo(mms, reg_rx);
        break;
    case MMS2: /* Tx  Registers */
        dump_reginfo(mms, reg_tx);
        break;
    default:
        printf("%s - Unknown memory map selector(0x%02x)\n", __func__, mms);
        return -1;
    }

    return 0;
}

static int read_register_in_register_group(uint8_t reg_grp) {
    switch (reg_grp) {
    case MMS0: /* General Registers */
        dump_reginfo(reg_grp, reg_general);
        break;
    case MMS1: /* Rx Registers */
        dump_reginfo(reg_grp, reg_rx);
        break;
    case MMS2: /* Tx  Registers */
        dump_reginfo(reg_grp, reg_tx);
        break;
    default:
        printf("%s - Unknown Register Group(0x%02x)\n", __func__, reg_grp);
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

static int set_mpw_register_value(uint8_t mms, struct reginfo* reginfo, int32_t addr, uint32_t data) {

    write_register_mpw(mms, (uint16_t)addr, data);
    return 0;
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

static int write_register_in_register_group(uint8_t reg_grp, int32_t addr, uint32_t data) {
    return set_mpw_register_value(reg_grp, reg_general, addr, data);
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

int api_read_register_in_register_group(int reg_grp) {
    return read_register_in_register_group((uint8_t)reg_grp);
}

int api_write_register_in_mms(int mms, int addr, uint32_t data) {
    return write_register_in_mms((uint8_t)mms, (int32_t)addr, (uint32_t)data);
}

int api_write_register_in_register_group(int reg_grp, int addr, uint32_t data) {
    return write_register_in_register_group((uint8_t)reg_grp, (int32_t)addr, (uint32_t)data);
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
