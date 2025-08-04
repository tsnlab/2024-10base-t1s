#include "lan865x_arch.h"

#include <linux/io.h>


uint32_t read32(void* addr) {
    return ioread32(addr);
}

void write32(u32 val, void* addr) {
    iowrite32(val, addr);
}

sysclock_t lan865x_get_sys_clock(struct lan865x_priv* priv) {
	struct oa_tc6* tc6 = priv->tc6;
    
	u32 sec_h;
	u32 sec, nsec;
    u64 tmp_sec, clock;

    if (oa_tc6_read_register(tc6, MMS1_MAC_TSH, &sec_h)) return -ENODEV;
    if (oa_tc6_read_register(tc6, MMS1_MAC_TSL, &sec)) return -ENODEV;
    if (oa_tc6_read_register(tc6, MMS1_MAC_TN, &nsec)) return -ENODEV;

	tmp_sec = (u64)sec * NS_IN_1S;
	nsec = nsec & 0x3FFFFFFF;

    clock = tmp_sec + nsec;

    return clock;
}

int lan865x_set_sys_clock(struct lan865x_priv* priv, u64 timestamp) {
	struct oa_tc6* tc6 = priv->tc6;

	u32 sec_h = 0x00000000 & 0xFFFFFFFF;
	u32 sec = (u32)(timestamp / NS_IN_1S) & 0xFFFFFFFF;
	u32 nsec = (u32)((timestamp % NS_IN_1S) & 0x3FFFFFFF);

	lan865x_debug("%s: sec_h = %u, sec = %u, nsec = %u\n", __func__, sec_h, sec, nsec);

    if (oa_tc6_write_register(tc6, MMS1_MAC_TSH, sec_h)) return -ENODEV;
    if (oa_tc6_write_register(tc6, MMS1_MAC_TSL, sec)) return -ENODEV;
    if (oa_tc6_write_register(tc6, MMS1_MAC_TN, nsec)) return -ENODEV;

    return 0;
}

void lan865x_set_sys_clock_nanocount(struct lan865x_priv* priv, u8 nano_count) {
	struct oa_tc6* tc6 = priv->tc6;
	u32 reg_val = 0;

    /* Bits 7:0 = CNS (nanosecond increment value) */
    reg_val |= (nano_count & 0xFF);

	/* Set MAC_TI(TSU Timer Increment) register */
    oa_tc6_write_register(tc6, MMS1_MAC_TI, reg_val);
}

void lan865x_add_sys_clock(struct lan865x_priv* priv, u32 add_offset) {
	struct oa_tc6* tc6 = priv->tc6;
	u32 reg_val = 0;

	add_offset = (add_offset >> 2); // 32bit -> 30bit

    /* Bits 29:0 = ITDT */
    reg_val |= (add_offset & 0x3FFFFFFF);

    /* Bit 31 = 0 (add) */
    reg_val &= ~(1U << 31);

	/* Set MAC_TA(TSU Timer Adjust) register */
    oa_tc6_write_register(tc6, MMS1_MAC_TA, reg_val);
}

void lan865x_sub_sys_clock(struct lan865x_priv* priv, u32 sub_offset) {
	struct oa_tc6* tc6 = priv->tc6;
	u32 reg_val = 0;

    /* Bits 29:0 = ITDT */
    reg_val |= (sub_offset & 0x3FFFFFFF);

    /* Bit 31 = 1 (subtract) */
    reg_val |= (1U << 31);

	/* Set MAC_TA(TSU Timer Adjust) register */
    oa_tc6_write_register(tc6, MMS1_MAC_TA, reg_val);
}

timestamp_t lan865x_read_tx_timestamp(struct lan865x_priv* priv, int tx_id) {
	struct oa_tc6* tc6 = priv->tc6;
    u32 ts_h = 0, ts_l = 0;
	u64 tmp_sec = 0;
    u64 timestamp = 0;

    switch (tx_id) {
    case LAN865X_TIMESTAMP_ID_A:
        oa_tc6_read_register(tc6, MMS0_TTSCAH, &ts_h);
        oa_tc6_read_register(tc6, MMS0_TTSCAL, &ts_l);
        break;
    case LAN865X_TIMESTAMP_ID_B:
        oa_tc6_read_register(tc6, MMS0_TTSCBH, &ts_h);
        oa_tc6_read_register(tc6, MMS0_TTSCBL, &ts_l);
        break;
    case LAN865X_TIMESTAMP_ID_C:
        oa_tc6_read_register(tc6, MMS0_TTSCCH, &ts_h);
        oa_tc6_read_register(tc6, MMS0_TTSCCL, &ts_l);
        break;
    default:
        return timestamp;
    }

	tmp_sec = (u64)ts_h * NS_IN_1S;
	ts_l = ts_l & 0xFFFFFFFF;

    timestamp = tmp_sec + ts_l;
    return timestamp;
}

void lan865x_update_tx_packets(struct lan865x_priv* priv) {
	struct oa_tc6* tc6 = priv->tc6;
    u32 tx_count = 0, total_tx_count = 0;

    /* This register gets cleared after read */
    oa_tc6_read_register(tc6, MMS0_STATS12, &tx_count);
    oa_tc6_read_register(tc6, MMS0_STATS11, &total_tx_count);

    priv->total_tx_count += tx_count;
    if ((total_tx_count - tx_count) > 0) {
        priv->total_tx_drop_count += (total_tx_count - tx_count);
    }
}

u64 lan865x_get_tx_packets(struct lan865x_priv* priv) {
    lan865x_update_tx_packets(priv);

    return priv->total_tx_count;
}

u64 lan865x_get_tx_drop_packets(struct lan865x_priv* priv) {
    lan865x_update_tx_packets(priv);

    return priv->total_tx_drop_count;
}
