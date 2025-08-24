#ifndef LAN865X_ARCH_H
#define LAN865X_ARCH_H

#include <linux/net_tstamp.h>
#include <linux/pci.h>
#include <linux/ptp_clock_kernel.h>
#include <linux/types.h>
#include <linux/oa_tc6.h>
#include <net/pkt_sched.h>

#ifdef __LAN865X_DEBUG__
#define lan865x_debug(...) pr_debug(__VA_ARGS__)
#else
#define lan865x_debug(...)
#endif // __LAN865X_DEBUG__

#define FRAME_TIMESTAMP_ENABLE

#define LAN865X_BUFFER_SIZE (1560)

#define MMS1_MAC_TSH 0x00010070
#define MMS1_MAC_TSL 0x00010074
#define MMS1_MAC_TN  0x00010075
#define MMS1_MAC_TA  0x00010076
#define MMS1_MAC_TI  0x00010077
#define MMS1_MAC_TISUBN  0x0001006F

enum lan865x_timestamp_id {
    LAN865X_TIMESTAMP_ID_NONE = 0,
    LAN865X_TIMESTAMP_ID_GPTP, // GPTP
    LAN865X_TIMESTAMP_ID_NORMAL, // NORMAL
    LAN865X_TIMESTAMP_ID_RESERVED, // RESERVED
    LAN865X_TIMESTAMP_ID_MAX,
};

#define MMS0_TTSCAH 0x10
#define MMS0_TTSCAL 0x11
#define MMS0_TTSCBH 0x12
#define MMS0_TTSCBL 0x13
#define MMS0_TTSCCH 0x14
#define MMS0_TTSCCL 0x15
#define MMS0_STATS11 0x0213
#define MMS0_STATS12 0x0214

#define NS_IN_1S 1000000000

/* 25Mhz = LAN8650 SPI MAX Hz */
#define TICKS_SCALE 40
#define RESERVED_CYCLE 25000000

typedef u64 sysclock_t;
typedef u64 timestamp_t;

struct oa_tc6 {
	struct device *dev;
	struct net_device *netdev;
	struct phy_device *phydev;
	struct mii_bus *mdiobus;
	struct spi_device *spi;
	struct mutex spi_ctrl_lock; /* Protects spi control transfer */
	spinlock_t tx_skb_lock; /* Protects tx skb handling */
	void *spi_ctrl_tx_buf;
	void *spi_ctrl_rx_buf;
	void *spi_data_tx_buf;
	void *spi_data_rx_buf;
	struct sk_buff *ongoing_tx_skb;
	struct sk_buff *waiting_tx_skb;
	struct sk_buff *rx_skb;
	struct task_struct *spi_thread;
	wait_queue_head_t spi_wq;
	u16 tx_skb_offset;
	u16 spi_data_tx_buf_offset;
	u16 tx_credits;
	u8 rx_chunks_available;
	bool rx_buf_overflow;
	bool int_flag;
};

struct ptp_device {
	struct device *dev;
	struct oa_tc6 *tc6;
	struct ptp_clock *ptp_clock;
	struct ptp_clock_info ptp_info;

	struct task_struct *ptp_thread;

	u32 ti_subnano_b24; // timer increase every clock (25MHz) cycle
	u64 offset;

	spinlock_t lock;
};

struct lan865x_priv {
    struct work_struct multicast_work;
    struct net_device* netdev;
    struct spi_device* spi;
    struct oa_tc6* tc6;

	struct ptp_device* ptpdev;
	struct hwtstamp_config tstamp_config;
	struct sk_buff *waiting_txts_skb[LAN865X_TIMESTAMP_ID_MAX-1];

    uint64_t total_tx_count;
    uint64_t total_tx_drop_count;
};


struct lan865x_priv* get_lan865x_priv_by_ptp_info(struct ptp_clock_info *ptp_info);

uint32_t read32(void* addr);
void write32(uint32_t val, void* addr);

sysclock_t lan865x_get_sys_clock(struct lan865x_priv* priv);
int lan865x_set_sys_clock(struct lan865x_priv* priv, u64 timestamp);
u32 lan865x_get_cycle_1s(void);
void lan865x_set_sys_clock_ti(struct lan865x_priv* priv, u64 subnano_b24);
void lan865x_add_sys_clock(struct lan865x_priv* priv, u32 add_offset);
void lan865x_sub_sys_clock(struct lan865x_priv* priv, u32 sub_offset);
timestamp_t lan865x_read_tx_timestamp(struct lan865x_priv* priv, int tx_id);
u64 lan865x_get_tx_packets(struct lan865x_priv* priv);
u64 lan865x_get_tx_drop_packets(struct lan865x_priv* priv);
void lan865x_update_tx_packets(struct lan865x_priv* priv);

#endif /* LAN865X_ARCH_H */
