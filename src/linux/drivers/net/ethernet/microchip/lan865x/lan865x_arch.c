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

    if (oa_tc6_read_register(tc6, MMS1_MAC_TN, &nsec))
        return -ENODEV;
    if (oa_tc6_read_register(tc6, MMS1_MAC_TSL, &sec))
        return -ENODEV;
    if (oa_tc6_read_register(tc6, MMS1_MAC_TSH, &sec_h))
        return -ENODEV;

    tmp_sec = (u64)sec * NS_IN_1S;
    nsec = nsec & 0x3FFFFFFF;

    clock = tmp_sec + nsec;

    return clock;
}

int lan865x_set_sys_clock(struct lan865x_priv* priv, u64 timestamp) {
    struct oa_tc6* tc6 = priv->tc6;

    u64 sec = timestamp / NS_IN_1S;
    u32 sec_h = (u32)(sec >> 32) & 0x0000FFFF;
    u32 sec_l = (u32)(sec & 0xFFFFFFFF);
    u32 nsec = (u32)(timestamp % NS_IN_1S) & 0x3FFFFFFF; // 30bit. Maybe not needed to mask

    (void)sec_l;

    LAN865X_DEBUG("%s: sec_h = %u, sec = %u, nsec = %u\n", __func__, sec_h, sec, nsec);

    // Reverse order for lower the error
    if (oa_tc6_write_register(tc6, MMS1_MAC_TN, nsec))
        return -ENODEV;
    if (oa_tc6_write_register(tc6, MMS1_MAC_TSL, sec))
        return -ENODEV;
    if (oa_tc6_write_register(tc6, MMS1_MAC_TSH, sec_h))
        return -ENODEV;

    return 0;
}

void lan865x_set_sys_clock_ti(struct lan865x_priv* priv, u64 subnano_b24) {
    struct oa_tc6* tc6 = priv->tc6;
    u32 reg_ti_val;
    u32 reg_tisubn_val;

    // XXX: Write bigger unit first, smaller later

    // 8bit 000000xx
    u32 nano = subnano_b24 >> 24;
    reg_ti_val = (nano & 0xFF);
    oa_tc6_write_register(tc6, MMS1_MAC_TI, reg_ti_val);

    // 24bit 00123456 -> 56001234
    u32 subnano = subnano_b24 & 0x00FFFFFF;
    reg_tisubn_val = ((subnano & 0xffff00) >> 8) | ((subnano & 0x0000FF) << 24);

    // Set MAC_TI(TSU Timer Increment) register
    oa_tc6_write_register(tc6, MMS1_MAC_TISUBN, reg_tisubn_val);
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

    static const u16 reg_hi[] = {
        [LAN865X_TIMESTAMP_ID_GPTP]    = MMS0_TTSCAH,
        [LAN865X_TIMESTAMP_ID_NORMAL]  = MMS0_TTSCBH,
        [LAN865X_TIMESTAMP_ID_RESERVED]= MMS0_TTSCCH,
    };
    static const u16 reg_lo[] = {
        [LAN865X_TIMESTAMP_ID_GPTP]    = MMS0_TTSCAL,
        [LAN865X_TIMESTAMP_ID_NORMAL]  = MMS0_TTSCBL,
        [LAN865X_TIMESTAMP_ID_RESERVED]= MMS0_TTSCCL,
    };

    if (tx_id < 0 || tx_id >= ARRAY_SIZE(reg_hi) || !reg_hi[tx_id] || !reg_lo[tx_id])
        return -EINVAL;

    oa_tc6_read_register(tc6, reg_hi[tx_id], &ts_h);
    oa_tc6_read_register(tc6, reg_lo[tx_id], &ts_l);

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
