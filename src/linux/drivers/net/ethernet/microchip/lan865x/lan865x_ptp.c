#include "lan865x_ptp.h"

#define NSEC_PER_MHZ 1000
#define MHZ_TO_NS(mhz) (NSEC_PER_MHZ / (mhz))

struct lan865x_priv* get_lan865x_priv_by_ptp_info(struct ptp_clock_info *ptp_info) {
	struct ptp_device* ptpdev = container_of(ptp_info, struct ptp_device, ptp_info);
	struct lan865x_priv* priv = dev_get_drvdata(ptpdev->dev);
	
	return priv;
}

#if 1
#include <linux/delay.h>
#define MMS0_OA_STATUS0 0x00000008
#define TS_A_MASK (1 << 8)
#define TS_B_MASK (1 << 9)
#define TS_C_MASK (1 << 10)

#define MMS0_OA_MASK0	0x0000000C
#define TS_A_INT_ENABLE (1 << 8)
#define TS_B_INT_ENABLE (1 << 9)
#define TS_C_INT_ENABLE (1 << 10)
#endif

static int lan865x_ptp_thread_handler(void* data) 
{
	struct ptp_device* ptpdev = (struct ptp_device*)data;
	struct lan865x_priv* priv = dev_get_drvdata(ptpdev->dev);
	u32 status;
	timestamp_t tx_ts;
	struct skb_shared_hwtstamps skb_hwts;

	while(true) {
		//wait_event_interruptible(ptpdev->tc6->spi_wq, ptpdev->tc6->ts_int_flag);
		udelay(10);

		//if (ptpdev->tc6->ts_int_flag) {
		//	ptpdev->tc6->ts_int_flag = false;
		//}
		
		//pr_err("Wake-Up %s\n", __func__);

		oa_tc6_read_register(ptpdev->tc6, MMS0_OA_STATUS0, &status);

		// GPTP
		if (status & TS_A_MASK) {
			tx_ts = lan865x_read_tx_timestamp(priv, 1);
			pr_err("%s: A Timestamp = %u.%u\n", __func__, tx_ts/NS_IN_1S, tx_ts%NS_IN_1S);

			skb_hwts.hwtstamp = ns_to_ktime(tx_ts);	
			pr_err("%s: tx_ts = %llu, skb_hwts.hwtstamp = %llu\n", __func__, tx_ts, skb_hwts.hwtstamp);
			skb_tstamp_tx(priv->waiting_txts_skb[LAN865X_TIMESTAMP_ID_A], &skb_hwts);
			//dev_kfree_skb_any(priv->waiting_txts_skb[LAN865X_TIMESTAMP_ID_A]);
			kfree_skb(priv->waiting_txts_skb[LAN865X_TIMESTAMP_ID_A]);
		}
		// NORMAL
		if (status & TS_B_MASK) {
			tx_ts = lan865x_read_tx_timestamp(priv, 2);
			pr_err("%s: B Timestamp = %u.%u\n", __func__, tx_ts/NS_IN_1S, tx_ts%NS_IN_1S);

			skb_hwts.hwtstamp = ns_to_ktime(tx_ts);	
			pr_err("%s: tx_ts = %llu, skb_hwts.hwtstamp = %llu\n", __func__, tx_ts, skb_hwts.hwtstamp);
			skb_tstamp_tx(priv->waiting_txts_skb[LAN865X_TIMESTAMP_ID_B], &skb_hwts);
			//dev_kfree_skb_any(priv->waiting_txts_skb[LAN865X_TIMESTAMP_ID_A]);
			kfree_skb(priv->waiting_txts_skb[LAN865X_TIMESTAMP_ID_B]);
		}
		// RESERVED
		if (status & TS_C_MASK) {
			tx_ts = lan865x_read_tx_timestamp(priv, 3);
			pr_err("%s: C Timestamp = %u.%u\n", __func__, tx_ts/NS_IN_1S, tx_ts%NS_IN_1S);
		}

		if (status & (TS_A_MASK | TS_B_MASK | TS_C_MASK) ) {
			oa_tc6_write_register(ptpdev->tc6, MMS0_OA_STATUS0, status);
		}
		status = 0;
	}

	return 0;
}

static int lan865x_ptp_adjfine(struct ptp_clock_info *ptp_info, long scaled_ppm)
{
	u64 diff;
	unsigned long flags;
	u32 ppm;
	int is_negative = 0;

	struct lan865x_priv* priv = get_lan865x_priv_by_ptp_info(ptp_info);
	struct ptp_device* ptpdev = priv->ptpdev;

	pr_err("lan865x: call %s", __func__);

	spin_lock_irqsave(&ptpdev->lock, flags);

	if (scaled_ppm == 0) {
		goto exit;
	}

	if (scaled_ppm < 0) {
		is_negative = 1;
		scaled_ppm = -scaled_ppm;
	}
	ppm = scaled_ppm >> 16;

	/* Adjust ticks_scale */
	diff = mul_u64_u64_div_u64(TICKS_SCALE, (u64)scaled_ppm, 1000000ULL << 16);
	ptpdev->ticks_scale = (TICKS_SCALE + (is_negative ? - diff : diff)) & 0xFF;
	lan865x_set_sys_clock_nanocount(priv, (u8)ptpdev->ticks_scale);

	if (is_negative) {
		lan865x_sub_sys_clock(priv, (ppm & 0xFFFFFFFF));
		pr_err("%s: sub_sys_clock to ppm(us) = %u\n", __func__, ppm);
	} else {
		lan865x_add_sys_clock(priv, (ppm & 0xFFFFFFFF));
		pr_err("%s: add_sys_clock to ppm(us) = %u\n", __func__, ppm);
	}

	pr_err("%s: is_negative = %d, scaled_ppm = %d, ptpdev->ticks_scale = %u\n", __func__,
			is_negative, scaled_ppm, ptpdev->ticks_scale); 
	pr_err("%s: set_sys_clock_ns = %u\n", __func__, (u8)ptpdev->ticks_scale & 0xFF);

exit:
	spin_unlock_irqrestore(&ptpdev->lock, flags);

	return 0;
}

static int lan865x_ptp_adjtime(struct ptp_clock_info *ptp_info, s64 delta_ns)
{
	unsigned long flags;

	struct lan865x_priv* priv = get_lan865x_priv_by_ptp_info(ptp_info);
	struct ptp_device* ptpdev = priv->ptpdev;

	bool is_negative = false;
	u32 sec, nsec;
	timestamp_t hw_timestamp;

	pr_err("lan865x: call %s\n", __func__);

	spin_lock_irqsave(&ptpdev->lock, flags);

	if (delta_ns == 0)
		return 0;

	hw_timestamp = lan865x_get_sys_clock(priv);

	if (delta_ns < 0) {
		is_negative = true;
		delta_ns = abs(delta_ns);
	}

	hw_timestamp += is_negative ? -delta_ns : delta_ns;

	lan865x_set_sys_clock(priv, hw_timestamp);

	pr_err("%s: delta_ns = %c%llu, hw_timestamp = %llu\n", __func__, is_negative ? '-':'+', delta_ns, hw_timestamp);

	spin_unlock_irqrestore(&ptpdev->lock, flags);

	return 0;
}

static int lan865x_ptp_gettimex64(struct ptp_clock_info *ptp_info, struct timespec64 *ts,
			struct ptp_system_timestamp *sts) 
{
	u64 clock, timestamp;
	unsigned long flags;

	//pr_err("call %s", __func__);

	struct lan865x_priv* priv = get_lan865x_priv_by_ptp_info(ptp_info);
	struct ptp_device* ptpdev = priv->ptpdev;

	spin_lock_irqsave(&ptpdev->lock, flags);

	ptp_read_system_prets(sts);
	timestamp = lan865x_get_sys_clock(priv);
	ptp_read_system_postts(sts);

	ts->tv_sec = timestamp / NS_IN_1S;
	ts->tv_nsec = timestamp % NS_IN_1S;

	spin_unlock_irqrestore(&ptpdev->lock, flags);

	return 0;
}

static int lan865x_ptp_settime64(struct ptp_clock_info *ptp_info, const struct timespec64 *ts)
{
	u64 hw_timestamp, host_timestamp, sys_clock;
	unsigned long flags;

	struct lan865x_priv* priv = get_lan865x_priv_by_ptp_info(ptp_info);
	struct ptp_device* ptpdev = priv->ptpdev;

	pr_err("call %s", __func__);

	spin_lock_irqsave(&ptpdev->lock, flags);

	/* Get host timestamp */
	host_timestamp = (u64)ts->tv_sec * NS_IN_1S + ts->tv_nsec;

	ptpdev->ticks_scale = TICKS_SCALE;
	hw_timestamp = lan865x_get_sys_clock(priv);
	ptpdev->offset = host_timestamp - hw_timestamp;

	spin_unlock_irqrestore(&ptpdev->lock, flags);

	return 0;
}

struct ptp_device* ptp_device_init(struct device *dev, struct oa_tc6 *tc6, s32 max_adj)
{
	struct ptp_device *ptpdev;
	struct timespec64 ts;

	ptpdev = kzalloc(sizeof(struct ptp_device), GFP_KERNEL);
	if (!ptpdev) {
		dev_err(dev, "kzalloc()");
		return NULL;
	}
	memset(ptpdev, 0, sizeof(struct ptp_device));

	struct ptp_clock_info ptp_info = {
		.owner = THIS_MODULE,
		.name = "ptp",
		.max_adj = max_adj,
		.n_ext_ts = 0,
		.pps = 0,
		.adjfine = lan865x_ptp_adjfine,
		.adjtime = lan865x_ptp_adjtime,
		.gettimex64 = lan865x_ptp_gettimex64,
		.settime64 = lan865x_ptp_settime64,
	};

	spin_lock_init(&ptpdev->lock);

	ptpdev->dev = dev;
	ptpdev->tc6 = tc6;

	ptpdev->ptp_clock = ptp_clock_register(&ptpdev->ptp_info, dev);
	if (IS_ERR(ptpdev->ptp_clock)) {
		dev_err(dev, "Failed to register ptp clock\n");
		kfree(ptpdev);
		return NULL;
	}

	ptpdev->ptp_info = ptp_info;
	ptpdev->ticks_scale = TICKS_SCALE;

	ptpdev->ptp_thread = kthread_run(lan865x_ptp_thread_handler, ptpdev, "lan865x-ptp-thread");
	if (IS_ERR(ptpdev->ptp_thread)) {
		dev_err(ptpdev->dev, "Failed to create PTP thread\n");
		kfree(ptpdev);
		return NULL;
	}

	u32 regval;

#if 1
	regval = 0;
	oa_tc6_read_register(ptpdev->tc6, 0x00000004, &regval); // 0x0000_0004 = MMS0_OA_CONFIG0
	pr_err("%s: OA_CONFIG0 = 0x%08X\n", __func__, regval);

	regval = 0;
	oa_tc6_read_register(ptpdev->tc6, 0x00000008, &regval); // 0x0000_0008 = MMS0_OA_STATUS0
	pr_err("%s: OA_STATUS0 = 0x%08X\n", __func__, regval);

	regval = 40; // 40ns
	oa_tc6_write_register(ptpdev->tc6, MMS1_MAC_TI, regval);

	//regval = 0;
	//oa_tc6_read_register(ptpdev->tc6, 0x0000000C, &regval); // 0x0000_000C = MMS0_OA_MASK0
	//pr_err("%s: OA_MASK0 = 0x%08X\n", __func__, regval);

	// Configure the OA_MASK0 register to generate an interrupt on Tx Timestamp Capture.
	//regval &= ~(TS_A_INT_ENABLE | TS_B_INT_ENABLE | TS_C_INT_ENABLE);
	//oa_tc6_write_register(ptpdev->tc6, 0x0000000C, &regval); // 0x0000_000C = MMS0_OA_MASK0
	
	//regval = 0;
	//oa_tc6_read_register(ptpdev->tc6, 0x0000000C, &regval); // 0x0000_000C = MMS0_OA_MASK0
	//pr_err("%s: OA_MASK0 = 0x%08X\n", __func__, regval);
#endif /* 1 */

	return ptpdev;
}
