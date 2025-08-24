#ifndef LAN865X_GPTP_H
#define LAN865X_GPTP_H

#include "lan865x_arch.h"

bool is_gptp_packet(const struct sk_buff* skb);
struct ptp_device* ptp_device_init(struct device *dev, struct oa_tc6 *tc6, s32 max_adj);
void ptp_device_destroy(struct ptp_device *ptp);

#endif /* LAN865X_GPTP_H */
