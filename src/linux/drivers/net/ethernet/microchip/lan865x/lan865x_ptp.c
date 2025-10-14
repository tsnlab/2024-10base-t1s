#include "lan865x_ptp.h"

#include <linux/delay.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>

#define NSEC_PER_MHZ 1000
#define MHZ_TO_NS(mhz) (NSEC_PER_MHZ / (mhz))

#define PTP_THREAD_INTERVAL_MICROSECOND 10

struct lan865x_priv* get_lan865x_priv_by_ptp_info(struct ptp_clock_info* ptp_info) {
    struct ptp_device* ptpdev = container_of(ptp_info, struct ptp_device, ptp_info);
    struct lan865x_priv* priv = dev_get_drvdata(ptpdev->dev);

    return priv;
}

#ifndef __TSN_PTP__
static int lan865x_ptp_thread_handler(void* data) {
    struct ptp_device* ptpdev = (struct ptp_device*)data;
    struct lan865x_priv* priv = dev_get_drvdata(ptpdev->dev);
    u32 status;
    timestamp_t tx_ts;
    struct skb_shared_hwtstamps skb_hwts;

    while (true) {
        // NOTE: ptp_thread_handler operates at 10µs intervals, which may affect PTP accuracy.
        udelay(PTP_THREAD_INTERVAL_MICROSECOND);

        oa_tc6_read_register(ptpdev->tc6, MMS0_OA_STATUS0, &status);

        // GPTP
        if (status & TS_A_MASK) {
            tx_ts = lan865x_read_tx_timestamp(priv, 1);
            LAN865X_DEBUG("%s: A Timestamp = %llu.%llu\n", __func__, tx_ts / NS_IN_1S, tx_ts % NS_IN_1S);

            skb_hwts.hwtstamp = ns_to_ktime(tx_ts);
            LAN865X_DEBUG("%s: tx_ts = %llu, skb_hwts.hwtstamp = %llu\n", __func__, tx_ts, skb_hwts.hwtstamp);
            skb_tstamp_tx(priv->waiting_txts_skb[LAN865X_TIMESTAMP_ID_GPTP], &skb_hwts);
            kfree_skb(priv->waiting_txts_skb[LAN865X_TIMESTAMP_ID_GPTP]);
        }
        // NORMAL
        if (status & TS_B_MASK) {
            tx_ts = lan865x_read_tx_timestamp(priv, 2);
            LAN865X_DEBUG("%s: B Timestamp = %llu.%llu\n", __func__, tx_ts / NS_IN_1S, tx_ts % NS_IN_1S);

            skb_hwts.hwtstamp = ns_to_ktime(tx_ts);
            LAN865X_DEBUG("%s: tx_ts = %llu, skb_hwts.hwtstamp = %llu\n", __func__, tx_ts, skb_hwts.hwtstamp);
            skb_tstamp_tx(priv->waiting_txts_skb[LAN865X_TIMESTAMP_ID_NORMAL], &skb_hwts);
            kfree_skb(priv->waiting_txts_skb[LAN865X_TIMESTAMP_ID_NORMAL]);
        }
        // RESERVED
        if (status & TS_C_MASK) {
            tx_ts = lan865x_read_tx_timestamp(priv, 3);
            LAN865X_DEBUG("%s: C Timestamp = %llu.%llu\n", __func__, tx_ts / NS_IN_1S, tx_ts % NS_IN_1S);
        }

        if (status & (TS_A_MASK | TS_B_MASK | TS_C_MASK)) {
            oa_tc6_write_register(ptpdev->tc6, MMS0_OA_STATUS0, status);
        }
        status = 0;
    }

    return 0;
}
#endif

bool is_gptp_packet(const struct sk_buff* skb) {
    struct ethhdr* eth;
    struct vlan_hdr* vlan;
    __be16 proto;

    eth = (struct ethhdr*)skb->data;
    proto = ntohs(eth->h_proto);

    if (proto == ETH_P_8021Q || proto == ETH_P_8021AD) {
        vlan = (struct vlan_hdr*)(skb->data + sizeof(struct ethhdr));

        return ntohs(vlan->h_vlan_encapsulated_proto) == ETH_P_1588;
    }

    return proto == ETH_P_1588;
}

static int lan865x_ptp_adjfine(struct ptp_clock_info* ptp_info, long scaled_ppm) {
    u64 ticks_scale, diff_b24;
    unsigned long flags;
    u32 ppm;
    int is_negative = 0;

    struct lan865x_priv* priv = get_lan865x_priv_by_ptp_info(ptp_info);
    struct ptp_device* ptpdev = priv->ptpdev;

    LAN865X_DEBUG("lan865x: call %s", __func__);

#if 1
    mutex_lock(&ptpdev->lock);
#else
    spin_lock_irqsave(&ptpdev->lock, flags);
#endif

    if (scaled_ppm == 0) {
        goto exit;
    }

    if (scaled_ppm < 0) {
        is_negative = 1;
        scaled_ppm = -scaled_ppm;
    }
    ppm = scaled_ppm >> 16;

    /* Adjust ticks_scale */
    diff_b24 = mul_u64_u64_div_u64(TICKS_SCALE << (24 - 16), (u64)scaled_ppm, 1000000ULL);
    ticks_scale = ((TICKS_SCALE << 24) + (is_negative ? -diff_b24 : diff_b24));

    lan865x_set_sys_clock_ti(priv, ticks_scale);
    ptpdev->ti_subnano_b24 = ticks_scale;

    LAN865X_DEBUG("%s: scaled_ppm = %ld, diff = %llu, ticks_scale = %llu = %014llx\n", __func__, scaled_ppm, diff_b24,
                  ticks_scale, ticks_scale);

exit:
#if 1
    mutex_unlock(&ptpdev->lock);
#else
    spin_unlock_irqrestore(&ptpdev->lock, flags);
#endif

    return 0;
}

static int lan865x_ptp_adjtime(struct ptp_clock_info* ptp_info, s64 delta_ns) {
    unsigned long flags;

    struct lan865x_priv* priv = get_lan865x_priv_by_ptp_info(ptp_info);
    struct ptp_device* ptpdev = priv->ptpdev;

    bool is_negative = false;
    timestamp_t hw_timestamp = 0;
    timestamp_t curr_hw_timestamp = 0;

    LAN865X_DEBUG("lan865x: call %s\n", __func__);

    if (delta_ns == 0) {
        return 0;
    }

#if 1
    mutex_lock(&ptpdev->lock);
#else
    spin_lock_irqsave(&ptpdev->lock, flags);
#endif

    hw_timestamp = lan865x_get_sys_clock(priv);

    if (delta_ns < 0) {
        is_negative = true;
        delta_ns = abs(delta_ns);
    }

    hw_timestamp += is_negative ? -delta_ns : delta_ns;

    lan865x_set_sys_clock(priv, hw_timestamp);
    curr_hw_timestamp = lan865x_get_sys_clock(priv);

    LAN865X_DEBUG("%s: delta_ns = %c%llu, curr_hw_timestamp = %llu\n", __func__, is_negative ? '-' : '+', delta_ns,
                  curr_hw_timestamp);

#if 1
    mutex_unlock(&ptpdev->lock);
#else
    spin_unlock_irqrestore(&ptpdev->lock, flags);
#endif

    return 0;
}

static int lan865x_ptp_gettimex64(struct ptp_clock_info* ptp_info, struct timespec64* res_ts,
                                  struct ptp_system_timestamp* sts) {
    u64 timestamp;
    unsigned long flags;

    LAN865X_DEBUG("lan865x: call %s", __func__);

    struct lan865x_priv* priv = get_lan865x_priv_by_ptp_info(ptp_info);
    struct ptp_device* ptpdev = priv->ptpdev;

#if 1
    mutex_lock(&ptpdev->lock);
#else
    spin_lock_irqsave(&ptpdev->lock, flags);
#endif

    ptp_read_system_prets(sts);
    timestamp = lan865x_get_sys_clock(priv);
    ptp_read_system_postts(sts);

    res_ts->tv_sec = timestamp / NS_IN_1S;
    res_ts->tv_nsec = timestamp % NS_IN_1S;

#if 1
    mutex_unlock(&ptpdev->lock);
#else
    spin_unlock_irqrestore(&ptpdev->lock, flags);
#endif

    return 0;
}

static int lan865x_ptp_settime64(struct ptp_clock_info* ptp_info, const struct timespec64* set_ts) {
    (void)set_ts;
    u64 host_timestamp;
    unsigned long flags;

    struct lan865x_priv* priv = get_lan865x_priv_by_ptp_info(ptp_info);
    struct ptp_device* ptpdev = priv->ptpdev;

    LAN865X_DEBUG("lan865x: call %s", __func__);

#if 1
    mutex_lock(&ptpdev->lock);
#else
    spin_lock_irqsave(&ptpdev->lock, flags);
#endif

    /* Get host timestamp */
    host_timestamp = (u64)set_ts->tv_sec * NS_IN_1S + set_ts->tv_nsec;

    // TODO add/sub
    lan865x_set_sys_clock(priv, host_timestamp);

#if 1
    mutex_unlock(&ptpdev->lock);
#else
    spin_unlock_irqrestore(&ptpdev->lock, flags);
#endif

    return 0;
}

struct ptp_device* ptp_device_init(struct device* dev, struct oa_tc6* tc6, s32 max_adj) {
    struct ptp_device* ptpdev;

    ptpdev = kzalloc(sizeof(struct ptp_device), GFP_KERNEL);
    if (!ptpdev) {
        dev_err(dev, "kzalloc()");
        return NULL;
    }
    memset(ptpdev, 0, sizeof(struct ptp_device));

    struct ptp_clock_info ptp_info = {
        .owner = THIS_MODULE,
        .name = "ptp",
        .max_adj = max_adj, /* max_adj --> RESERVED_CYCLE , */
        .n_ext_ts = 0,
        .pps = 0,
        .adjfine = lan865x_ptp_adjfine,
        .adjtime = lan865x_ptp_adjtime,
        .gettimex64 = lan865x_ptp_gettimex64,
        .settime64 = lan865x_ptp_settime64,
    };

#if 1
    mutex_init(&ptpdev->lock);
#else
    spin_lock_init(&ptpdev->lock);
#endif

    ptpdev->dev = dev;
    ptpdev->tc6 = tc6;

    ptpdev->ptp_clock = ptp_clock_register(&ptpdev->ptp_info, dev);
    if (IS_ERR(ptpdev->ptp_clock)) {
        dev_err(dev, "Failed to register ptp clock\n");
        kfree(ptpdev);
        return NULL;
    }

    ptpdev->ptp_info = ptp_info;
    // TODO: read from register
    ptpdev->ti_subnano_b24 = TICKS_SCALE << TISUBNS_FRAC_BITS;

#ifndef __TSN_PTP__
    ptpdev->ptp_thread = kthread_run(lan865x_ptp_thread_handler, ptpdev, "lan865x-ptp-thread");
    if (IS_ERR(ptpdev->ptp_thread)) {
        dev_err(ptpdev->dev, "Failed to create PTP thread\n");
        kfree(ptpdev);
        return NULL;
    }
#endif

    /* TODO: Configure the OA_MASK0 register to generate an interrupt on Tx Timestamp Capture.
    u32 regval;
    regval = 0;
    oa_tc6_read_register(ptpdev->tc6, 0x0000000C, &regval); // 0x0000_000C = MMS0_OA_MASK0
    LAN865X_DEBUG("%s: OA_MASK0 = 0x%08X\n", __func__, regval);
    regval &= ~(TS_A_INT_ENABLE | TS_B_INT_ENABLE | TS_C_INT_ENABLE);
    oa_tc6_write_register(ptpdev->tc6, 0x0000000C, &regval); // 0x0000_000C = MMS0_OA_MASK0
    regval = 0;
    oa_tc6_read_register(ptpdev->tc6, 0x0000000C, &regval); // 0x0000_000C = MMS0_OA_MASK0
    LAN865X_DEBUG("%s: OA_MASK0 = 0x%08X\n", __func__, regval);
    */

    return ptpdev;
}

/**
 * lan865x_get_timestamp - Convert system count to timestamp
 * @sys_count: System count value
 * @ticks_scale: Scale factor for ticks
 * @offset: Offset value
 * @return: Calculated timestamp
 */
static timestamp_t lan865x_get_timestamp(u64 sys_count, double ticks_scale, u64 offset) {
    timestamp_t timestamp = ticks_scale * sys_count;

    return timestamp + offset;
}

/**
 * lan865x_sysclock_to_timestamp - Convert system clock to timestamp
 * @tc6: lan865x_spi struct pointer
 * @sysclock: System clock value to convert
 * @return: Timestamp value, 0 on failure
 */
timestamp_t lan865x_sysclock_to_timestamp(struct lan865x_priv* priv, sysclock_t sysclock) {
    struct ptp_device* ptp_data = priv->ptpdev;

    if (!ptp_data) {
        pr_err("%s - PTP not available\n", __func__);
        return 0;
    }

    u64 offset = ptp_data->offset;

    return lan865x_get_timestamp(sysclock, ptp_data->ticks_scale, offset);
}

/**
 * lan865x_sysclock_to_txtstamp - Convert system clock to TX timestamp
 * @tc6: lan865x_spi struct pointer
 * @sysclock: System clock value to convert
 * @return: TX timestamp value
 */
timestamp_t lan865x_sysclock_to_txtstamp(struct lan865x_priv* priv, sysclock_t sysclock) {
    return lan865x_sysclock_to_timestamp(priv, sysclock) + TX_ADJUST_NS;
}

/**
 * do_tx_work - Process TX work for timestamp handling
 * @work: Work structure
 * @tstamp_id: Timestamp ID to process
 *
 * This function handles TX timestamp processing in workqueue context,
 * including retry logic and timestamp validation.
 */
static void do_tx_work(struct work_struct* work, u16 tstamp_id) {
    sysclock_t tx_tstamp;
    struct skb_shared_hwtstamps shhwtstamps;
    struct lan865x_priv* priv = container_of(work - tstamp_id, struct lan865x_priv, tx_work[0]);
    struct sk_buff* skb = priv->tx_work_skb[tstamp_id];
    sysclock_t now = lan865x_get_sys_clock(priv);

#if 0
    struct oa_tc6* tc6 = priv->tc6;
    u32 sts11, sts12;

#define MMS1_MAC_STATS11 0x00010213
#define MMS1_MAC_STATS12 0x00010214

    oa_tc6_read_register(tc6, MMS1_MAC_STATS11, &sts11);
    oa_tc6_read_register(tc6, MMS1_MAC_STATS12, &sts12);

    pr_err("%s - Total Frames Transmitted (including errors) : %d, Frames Transmitted without Error: %d\n", __func__,
           sts11, sts12);

    if (priv->tstamp_retry[tstamp_id] == 0) {
        pr_err("%s - priv->magic: 0x%llx, tstamp_id: %d\n", __func__, priv->magic, tstamp_id);
    }
#endif

    if (tstamp_id >= LAN865X_TIMESTAMP_ID_MAX) {
        pr_err("Invalid timestamp ID\n");
        return;
    }

    if (!priv->tx_work_skb[tstamp_id]) {
        pr_err("%s - priv->tx_work_skb[%d] is NULL\n", __func__, tstamp_id);
        goto return_error;
    }

#if 1
    if (now < priv->tx_work_start_after[tstamp_id]) {
        goto retry;
    }
#endif
    /*
     * Read TX timestamp several times because
     * the work thread might try to read TX timestamp
     * before the register gets updated
     */
    tx_tstamp = lan865x_read_tx_timestamp(priv, tstamp_id);
    if (tx_tstamp == priv->last_tx_tstamp[tstamp_id]) {
#if 1
        if (tx_tstamp <= (priv->tx_work_start_after[tstamp_id] - 0x10000)) {
            /* The packet might have not been sent yet */
            pr_err("%s - retry, tx_tstamp:0x%16llx, wakeup: 0x%16llx\n", __func__, tx_tstamp,
                   priv->tx_work_start_after[tstamp_id] - 0x10000);
            goto retry;
        }
        if (lan865x_get_sys_clock(priv) < priv->tx_work_wait_until[tstamp_id]) {
            /* The packet might have not been sent yet */
            // pr_err("%s - The packet might have not been sent yet\n", __func__);
            goto retry;
        }
#endif
        /*
         * Tx timestamp is not updated. Try again.
         * Waiting for it to be updated forever is not desirable,
         * so limit the number of retries
         */
        if (++(priv->tstamp_retry[tstamp_id]) >= TX_TSTAMP_MAX_RETRY) {
            /* TODO: track the number of skipped packets for ethtool stats */
            pr_err("Failed to get timestamp: timestamp is not getting updated, "
                   "the packet might have been dropped, now: 0x%llx, tx_tstamp: 0x%llx\n",
                   now, tx_tstamp);
            goto return_error;
        }
        goto retry;
    }

#if 0
    pr_err("tstamp_id: %d, priv->tstamp_retry: %d\n", tstamp_id, priv->tstamp_retry[tstamp_id]);
#else
    pr_err("%s - last_tx_tstamp: 0x%16llx,  wakeup: 0x%16llx, diff: 0x%llx\n", __func__, tx_tstamp,
           priv->tx_work_start_after[tstamp_id] - 0x10000, tx_tstamp - priv->tx_work_start_after[tstamp_id] + 0x10000);
#endif

    priv->tstamp_retry[tstamp_id] = 0;
    shhwtstamps.hwtstamp = ns_to_ktime(tx_tstamp);
    priv->last_tx_tstamp[tstamp_id] = tx_tstamp;

    priv->tx_work_skb[tstamp_id] = NULL;
    clear_bit_unlock(tstamp_id, &priv->state);

    /* Update work queue state - work completed */
    atomic_dec(&priv->tx_work_pending[tstamp_id]);

    skb_tstamp_tx(skb, &shhwtstamps);
    dev_kfree_skb_any(skb);
#if 0
    pr_err("<<< %s - priv->last_tx_tstamp[%d] - 0x%llx\n", __func__, tstamp_id, priv->last_tx_tstamp[tstamp_id]);
#endif
    return;

return_error:
    priv->tstamp_retry[tstamp_id] = 0;
    clear_bit_unlock(tstamp_id, &priv->state);
    return;

retry:
    /* Check context before scheduling work */
    if (in_atomic()) {
        pr_warn("Cannot schedule work in atomic context during retry, tstamp_id=%d\n", tstamp_id);
        /* Fallback: try to schedule on current CPU */
        queue_work_on(smp_processor_id(), system_wq, &priv->tx_work[tstamp_id]);
    } else {
        schedule_work(&priv->tx_work[tstamp_id]);
    }

    /* Track work queue state */
    atomic_inc(&priv->tx_work_pending[tstamp_id]);
    return;
}

#define DEFINE_TX_WORK(n)                               \
    void lan865x_tx_work##n(struct work_struct* work) { \
        do_tx_work(work, n);                            \
    }

DEFINE_TX_WORK(1);
DEFINE_TX_WORK(2);
DEFINE_TX_WORK(3);
DEFINE_TX_WORK(4);
