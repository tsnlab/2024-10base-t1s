#include "lan865x_arch.h"

#include <linux/io.h>

uint32_t read32(void *addr)
{
	return ioread32(addr);
}

void write32(u32 val, void *addr)
{
	iowrite32(val, addr);
}

sysclock_t lan865x_get_sys_clock(struct oa_tc6 *tc6)
{
	u32 sec_h, sec_l, nano_sec;
	u64 sec;
	u64 clock;

	oa_tc6_read_register(tc6, MMS1_MAC_TSH, &sec_h);
	oa_tc6_read_register(tc6, MMS1_MAC_TSL, &sec_l);
	oa_tc6_read_register(tc6, MMS1_MAC_TN, &nano_sec);

	sec = ((u64)sec_h << 32) | sec_l;

	clock = sec * NS_IN_1S + nano_sec;

	return clock;
}

u32 lan865x_get_cycle_1s(struct oa_tc6 *tc6)
{
	return NS_IN_1S;
}

timestamp_t lan865x_read_tx_timestamp(struct oa_tc6 *tc6, int tx_id)
{
	u32 ts_h = 0, ts_l = 0;
	u64 timestamp;

	switch (tx_id) {
	case 1:
		oa_tc6_read_register(tc6, MMS0_TTSCAH, &ts_h);
		oa_tc6_read_register(tc6, MMS0_TTSCAL, &ts_l);
		break;
	case 2:
		oa_tc6_read_register(tc6, MMS0_TTSCBH, &ts_h);
		oa_tc6_read_register(tc6, MMS0_TTSCBL, &ts_l);
		break;
	case 3:
		oa_tc6_read_register(tc6, MMS0_TTSCCH, &ts_h);
		oa_tc6_read_register(tc6, MMS0_TTSCCL, &ts_l);
		break;
	}

	timestamp = ((u64)ts_h << 32) | ts_l;
	return timestamp;
}

void lan865x_update_tx_packets(struct oa_tc6 *tc6)
{
	struct net_device *netdev = tc6->netdev;
	struct lan865x_priv *priv = netdev_priv(netdev);
	u32 tx_count = 0, total_tx_count = 0;

	/* This register gets cleared after read */
	oa_tc6_read_register(tc6, MMS0_STATS12, &tx_count);
	oa_tc6_read_register(tc6, MMS0_STATS11, &total_tx_count);

	priv->total_tx_count += tx_count;
	if ((total_tx_count - tx_count) > 0) {
		priv->total_tx_drop_count += (total_tx_count - tx_count);
	}
}

u64 lan865x_get_tx_packets(struct oa_tc6 *tc6)
{
	struct net_device *netdev = tc6->netdev;
	struct lan865x_priv *priv = netdev_priv(netdev);

	lan865x_update_tx_packets(tc6);

	return priv->total_tx_count;
}

u64 lan865x_get_tx_drop_packets(struct oa_tc6 *tc6)
{
	struct net_device *netdev = tc6->netdev;
	struct lan865x_priv *priv = netdev_priv(netdev);

	lan865x_update_tx_packets(tc6);

	return priv->total_tx_drop_count;
}
