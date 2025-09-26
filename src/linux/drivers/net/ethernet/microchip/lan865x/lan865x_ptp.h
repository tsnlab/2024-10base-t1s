#ifndef LAN865X_GPTP_H
#define LAN865X_GPTP_H

#include "lan865x_arch.h"

#define TX_TSTAMP_MAX_RETRY 400
#define TX_ADJUST_NS 0

bool is_gptp_packet(const struct sk_buff* skb);
struct ptp_device* ptp_device_init(struct device* dev, struct oa_tc6* tc6, s32 max_adj);
void ptp_device_destroy(struct ptp_device* ptp);

void lan865x_tx_work1(struct work_struct* work);
void lan865x_tx_work2(struct work_struct* work);
void lan865x_tx_work3(struct work_struct* work);
void lan865x_tx_work4(struct work_struct* work);

#endif /* LAN865X_GPTP_H */
