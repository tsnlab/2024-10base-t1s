#ifndef LAN865X_ARCH_H
#define LAN865X_ARCH_H

#include <linux/net_tstamp.h>
#include <linux/pci.h>
#include <linux/ptp_clock_kernel.h>
#include <linux/types.h>
#include <net/pkt_sched.h>

#ifdef __LAN865X_DEBUG__
#define lan865x_debug(...) pr_debug(__VA_ARGS__)
#else
#define lan865x_debug(...)
#endif // __LAN865X_DEBUG__

#define FRAME_TIMESTAMP_ENABLE

#define LAN865X_BUFFER_SIZE (1560)

#define MMS1_MAC_TSH 0x1070
#define MMS1_MAC_TSL 0x1074
#define MMS1_MAC_TN 0x1075
#define MMS1_MAC_TA 0x1076
#define MMS1_MAC_TI 0x1077

enum lan865x_timestamp_id {
    LAN865X_TIMESTAMP_ID_NONE = 0,
    LAN865X_TIMESTAMP_ID_A,
    LAN865X_TIMESTAMP_ID_B,
    LAN865X_TIMESTAMP_ID_C,
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

typedef u64 sysclock_t;
typedef u64 timestamp_t;

struct oa_tc6 {
    struct device* dev;
    struct net_device* netdev;
    struct phy_device* phydev;
    struct mii_bus* mdiobus;
    struct spi_device* spi;
    struct mutex spi_ctrl_lock; /* Protects spi control transfer */
    spinlock_t tx_skb_lock;     /* Protects tx skb handling */
    void* spi_ctrl_tx_buf;
    void* spi_ctrl_rx_buf;
    void* spi_data_tx_buf;
    void* spi_data_rx_buf;
    struct sk_buff* ongoing_tx_skb;
    struct sk_buff* waiting_tx_skb;
    struct sk_buff* rx_skb;
    struct task_struct* spi_thread;
    wait_queue_head_t spi_wq;
    u16 tx_skb_offset;
    u16 spi_data_tx_buf_offset;
    u16 tx_credits;
    u8 rx_chunks_available;
    bool rx_buf_overflow;
    bool int_flag;
};

struct lan865x_priv {
    struct work_struct multicast_work;
    struct net_device* netdev;
    struct spi_device* spi;
    struct oa_tc6* tc6;
    struct hwtstamp_config tstamp_config;

    uint64_t total_tx_count;
    uint64_t total_tx_drop_count;
};

int oa_tc6_read_register(struct oa_tc6* tc6, u32 address, u32* value);

uint32_t read32(void* addr);
void write32(uint32_t val, void* addr);

sysclock_t lan865x_get_sys_clock(struct oa_tc6* tc6);
u32 lan865x_get_cycle_1s(struct oa_tc6* tc6);
timestamp_t lan865x_read_tx_timestamp(struct oa_tc6* tc6, int tx_id);
u64 lan865x_get_tx_packets(struct oa_tc6* tc6);
u64 lan865x_get_tx_drop_packets(struct oa_tc6* tc6);
void lan865x_update_tx_packets(struct oa_tc6* tc6);

#endif /* LAN865X_ARCH_H */
