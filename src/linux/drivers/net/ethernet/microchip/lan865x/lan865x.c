// SPDX-License-Identifier: GPL-2.0+
/*
 * Microchip's LAN865x 10BASE-T1S MAC-PHY driver
 *
 * Author: Parthiban Veerasooran <parthiban.veerasooran@microchip.com>
 */

#include <linux/bitfield.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/net_tstamp.h>
#include <linux/oa_tc6.h>
#include <linux/phy.h>

#include "lan865x_arch.h"
#include "lan865x_ioctl.h"
#include "lan865x_ptp.h"

#define DRV_NAME "lan8650"

/* MAC Network Control Register */
#define LAN865X_REG_MAC_NET_CTL 0x00010000
#define MAC_NET_CTL_TXEN BIT(3) /* Transmit Enable */
#define MAC_NET_CTL_RXEN BIT(2) /* Receive Enable */

/* MAC Network Configuration Reg */
#define LAN865X_REG_MAC_NET_CFG 0x00010001
#define MAC_NET_CFG_PROMISCUOUS_MODE BIT(4)
#define MAC_NET_CFG_MULTICAST_MODE BIT(6)
#define MAC_NET_CFG_UNICAST_MODE BIT(7)

/* MAC Hash Register Bottom */
#define LAN865X_REG_MAC_L_HASH 0x00010020
/* MAC Hash Register Top */
#define LAN865X_REG_MAC_H_HASH 0x00010021
/* MAC Specific Addr 1 Bottom Reg */
#define LAN865X_REG_MAC_L_SADDR1 0x00010022
/* MAC Specific Addr 1 Top Reg */
#define LAN865X_REG_MAC_H_SADDR1 0x00010023

/* PLCA Control 1 Register */
#define LAN865X_REG_PLCA_CTRL1 0x0004ca02

/* NOTE: Knob MAX of the T1S HAT board is 16, but the LAN8650 supports a maximum count of only 8. */
#define LAN8650_NODE_MAX_COUNT 8
#define NODE_ID_BITS_WIDTH 8
#define NODE_ID_MASK 0xFF

#define REGISTER_MAC_MASK 0xffffffff

#define MAC_ADDR_LENGTH 6
#define NUM_OF_BITS_IN_BYTE 8

static int lan865x_set_nodeid(struct lan865x_priv* priv, u32 node_id) {
    u32 regval;

    regval = (LAN8650_NODE_MAX_COUNT << NODE_ID_BITS_WIDTH) | (node_id & NODE_ID_MASK);
    return oa_tc6_write_register(priv->tc6, LAN865X_REG_PLCA_CTRL1, regval);
}

static int lan865x_set_hw_macaddr_low_bytes(struct oa_tc6* tc6, const u8* mac) {
    u32 regval;

    regval = (mac[3] << 24) | (mac[2] << 16) | (mac[1] << 8) | mac[0];

    return oa_tc6_write_register(tc6, LAN865X_REG_MAC_L_SADDR1, regval);
}

static int lan865x_set_hw_macaddr(struct lan865x_priv* priv, const u8* mac) {
    int restore_ret;
    u32 regval;
    int ret;

    /* Configure MAC address low bytes */
    ret = lan865x_set_hw_macaddr_low_bytes(priv->tc6, mac);
    if (ret) {
        return ret;
    }

    /* Prepare and configure MAC address high bytes */
    regval = (mac[5] << 8) | mac[4];
    ret = oa_tc6_write_register(priv->tc6, LAN865X_REG_MAC_H_SADDR1, regval);
    if (!ret) {
        return 0;
    }

    /* Restore the old MAC address low bytes from netdev if the new MAC
     * address high bytes setting failed.
     */
    restore_ret = lan865x_set_hw_macaddr_low_bytes(priv->tc6, priv->netdev->dev_addr);
    if (restore_ret) {
        return restore_ret;
    }

    return ret;
}

static int lan865x_ethtool_get_ts_info(struct net_device* netdev, struct ethtool_ts_info* ts_info) {
    struct lan865x_priv* priv = (struct lan865x_priv*)netdev_priv(netdev);

    ts_info->phc_index = ptp_clock_index(priv->ptpdev->ptp_clock);

    ts_info->so_timestamping = SOF_TIMESTAMPING_TX_SOFTWARE | SOF_TIMESTAMPING_RX_SOFTWARE | SOF_TIMESTAMPING_SOFTWARE |
                               SOF_TIMESTAMPING_TX_HARDWARE | SOF_TIMESTAMPING_RX_HARDWARE |
                               SOF_TIMESTAMPING_RAW_HARDWARE;

    ts_info->tx_types = BIT(HWTSTAMP_TX_OFF) | BIT(HWTSTAMP_TX_ON);

    ts_info->rx_filters = BIT(HWTSTAMP_FILTER_NONE) | BIT(HWTSTAMP_FILTER_ALL) | BIT(HWTSTAMP_FILTER_PTP_V2_L2_EVENT) |
                          BIT(HWTSTAMP_FILTER_PTP_V2_L2_SYNC) | BIT(HWTSTAMP_FILTER_PTP_V2_L2_DELAY_REQ);

    return 0;
}

static const struct ethtool_ops lan865x_ethtool_ops = {
    .get_link_ksettings = phy_ethtool_get_link_ksettings,
    .set_link_ksettings = phy_ethtool_set_link_ksettings,
    .get_ts_info = lan865x_ethtool_get_ts_info,
};

static int lan865x_get_ts_config(struct net_device* netdev, struct ifreq* ifr) {
    struct lan865x_priv* priv = (struct lan865x_priv*)netdev_priv(netdev);
    struct hwtstamp_config* hwts_config = &priv->tstamp_config;

    return copy_to_user(ifr->ifr_data, hwts_config, sizeof(*hwts_config)) ? -EFAULT : 0;
}

static int lan865x_set_ts_config(struct net_device* netdev, struct ifreq* ifr) {
    struct lan865x_priv* priv = (struct lan865x_priv*)netdev_priv(netdev);
    struct hwtstamp_config* hwts_config = &priv->tstamp_config;

    return copy_from_user(hwts_config, ifr->ifr_data, sizeof(*hwts_config)) ? -EFAULT : 0;
}

static int lan865x_set_mac_address(struct net_device* netdev, void* addr) {
    struct lan865x_priv* priv = (struct lan865x_priv*)netdev_priv(netdev);
    struct sockaddr* address = addr;
    int ret;

    ret = eth_prepare_mac_addr_change(netdev, addr);
    if (ret < 0) {
        return ret;
    }

    if (ether_addr_equal(address->sa_data, netdev->dev_addr)) {
        return 0;
    }

    ret = lan865x_set_hw_macaddr(priv, address->sa_data);
    if (ret) {
        return ret;
    }

    eth_commit_mac_addr_change(netdev, addr);

    return 0;
}

static u32 get_address_bit(u8 addr[ETH_ALEN], u32 bit) {
    return ((addr[bit / NUM_OF_BITS_IN_BYTE]) >> (bit % NUM_OF_BITS_IN_BYTE)) & 1;
}

static u32 lan865x_hash(u8 addr[ETH_ALEN]) {
    u32 hash_index = 0;

    for (int i = 0; i < MAC_ADDR_LENGTH; i++) {
        u32 hash = 0;

        for (int j = 0; j < NUM_OF_BITS_IN_BYTE; j++) {
            hash ^= get_address_bit(addr, (j * MAC_ADDR_LENGTH) + i);
        }

        hash_index |= (hash << i);
    }

    return hash_index;
}

static int lan865x_set_specific_multicast_addr(struct lan865x_priv* priv) {
    struct netdev_hw_addr* hw_addr;
    u32 hash_lo = 0;
    u32 hash_hi = 0;
    int ret;

    netdev_for_each_mc_addr(hw_addr, priv->netdev) {
        u32 bit_num = lan865x_hash(hw_addr->addr);

        if (bit_num >= BIT(5)) {
            hash_hi |= (1 << (bit_num - BIT(5)));
        } else {
            hash_lo |= (1 << bit_num);
        }
    }

    /* Enabling specific multicast addresses */
    ret = oa_tc6_write_register(priv->tc6, LAN865X_REG_MAC_H_HASH, hash_hi);
    if (ret) {
        netdev_err(priv->netdev, "Failed to write reg_hashh: %d\n", ret);
        return ret;
    }

    ret = oa_tc6_write_register(priv->tc6, LAN865X_REG_MAC_L_HASH, hash_lo);
    if (ret) {
        netdev_err(priv->netdev, "Failed to write reg_hashl: %d\n", ret);
    }

    return ret;
}

static int lan865x_set_all_multicast_addr(struct lan865x_priv* priv) {
    int ret;

    /* Enabling all multicast addresses */
    ret = oa_tc6_write_register(priv->tc6, LAN865X_REG_MAC_H_HASH, REGISTER_MAC_MASK);
    if (ret) {
        netdev_err(priv->netdev, "Failed to write reg_hashh: %d\n", ret);
        return ret;
    }

    ret = oa_tc6_write_register(priv->tc6, LAN865X_REG_MAC_L_HASH, REGISTER_MAC_MASK);
    if (ret) {
        netdev_err(priv->netdev, "Failed to write reg_hashl: %d\n", ret);
    }

    return ret;
}

static int lan865x_clear_all_multicast_addr(struct lan865x_priv* priv) {
    int ret;

    ret = oa_tc6_write_register(priv->tc6, LAN865X_REG_MAC_H_HASH, 0);
    if (ret) {
        netdev_err(priv->netdev, "Failed to write reg_hashh: %d\n", ret);
        return ret;
    }

    ret = oa_tc6_write_register(priv->tc6, LAN865X_REG_MAC_L_HASH, 0);
    if (ret) {
        netdev_err(priv->netdev, "Failed to write reg_hashl: %d\n", ret);
    }

    return ret;
}

static void lan865x_multicast_work_handler(struct work_struct* work) {
    struct lan865x_priv* priv = container_of(work, struct lan865x_priv, multicast_work);
    u32 regval = 0;
    int ret;

    if (priv->netdev->flags & IFF_PROMISC) {
        /* Enabling promiscuous mode */
        regval |= MAC_NET_CFG_PROMISCUOUS_MODE;
        regval &= (~MAC_NET_CFG_MULTICAST_MODE);
        regval &= (~MAC_NET_CFG_UNICAST_MODE);
    } else if (priv->netdev->flags & IFF_ALLMULTI) {
        /* Enabling all multicast mode */
        if (lan865x_set_all_multicast_addr(priv)) {
            return;
        }

        regval &= (~MAC_NET_CFG_PROMISCUOUS_MODE);
        regval |= MAC_NET_CFG_MULTICAST_MODE;
        regval &= (~MAC_NET_CFG_UNICAST_MODE);
    } else if (!netdev_mc_empty(priv->netdev)) {
        /* Enabling specific multicast mode */
        if (lan865x_set_specific_multicast_addr(priv)) {
            return;
        }

        regval &= (~MAC_NET_CFG_PROMISCUOUS_MODE);
        regval |= MAC_NET_CFG_MULTICAST_MODE;
        regval &= (~MAC_NET_CFG_UNICAST_MODE);
    } else {
        /* Enabling local mac address only */
        if (lan865x_clear_all_multicast_addr(priv)) {
            return;
        }
    }
    ret = oa_tc6_write_register(priv->tc6, LAN865X_REG_MAC_NET_CFG, regval);
    if (ret) {
        netdev_err(priv->netdev, "Failed to enable promiscuous/multicast/normal mode: %d\n", ret);
    }
}

static void lan865x_set_multicast_list(struct net_device* netdev) {
    struct lan865x_priv* priv = netdev_priv(netdev);

    schedule_work(&priv->multicast_work);
}

// TODO
//
// lan865x_recv_packet() {
//		rxfilter(); // for RX HW Timestamp, Ref: oa_tc6.c
//		netif_rx();
// }

static netdev_tx_t lan865x_send_packet(struct sk_buff* skb, struct net_device* netdev) {
    struct lan865x_priv* priv = netdev_priv(netdev);
    struct hwtstamp_config hwts_config = priv->tstamp_config;

    struct sk_buff* cloned_skb;
    u8 ts_capture_mode;

    if (skb_shinfo(skb)->tx_flags & SKBTX_HW_TSTAMP) {

        cloned_skb = skb_clone(skb, GFP_ATOMIC);
        if (!cloned_skb) {
            netdev_err(netdev, "%s: skb_clone() failed\n", __func__);
            return -1; // FIXME
        }

        /* NOTE:
         *	skb_clone() does not copy the user-space socket (sk) information.
         *	However, TX timestamping requires a valid sk to queue the timestamp to the user socket.
         *	Therefore, we manually copy the sk pointer from the original skb. */
        cloned_skb->sk = skb->sk;

        if (hwts_config.tx_type != HWTSTAMP_TX_ON) {
            ts_capture_mode = LAN865X_TIMESTAMP_ID_NONE;
            kfree_skb(cloned_skb);
        } else if (is_gptp_packet(skb)) {
            ts_capture_mode = LAN865X_TIMESTAMP_ID_GPTP;
            priv->waiting_txts_skb[LAN865X_TIMESTAMP_ID_GPTP] = skb_get(cloned_skb);
            skb_shinfo(priv->waiting_txts_skb[LAN865X_TIMESTAMP_ID_GPTP])->tx_flags |= SKBTX_IN_PROGRESS;
        } else {
            ts_capture_mode = LAN865X_TIMESTAMP_ID_NORMAL;
            priv->waiting_txts_skb[LAN865X_TIMESTAMP_ID_NORMAL] = skb_get(cloned_skb);
            skb_shinfo(priv->waiting_txts_skb[LAN865X_TIMESTAMP_ID_NORMAL])->tx_flags |= SKBTX_IN_PROGRESS;
        }
    }

    return oa_tc6_start_xmit(priv->tc6, skb, ts_capture_mode);
}

static int lan865x_hw_disable(struct lan865x_priv* priv) {
    u32 regval;

    if (oa_tc6_read_register(priv->tc6, LAN865X_REG_MAC_NET_CTL, &regval)) {
        return -ENODEV;
    }

    regval &= ~(MAC_NET_CTL_TXEN | MAC_NET_CTL_RXEN);

    if (oa_tc6_write_register(priv->tc6, LAN865X_REG_MAC_NET_CTL, regval)) {
        return -ENODEV;
    }

    return 0;
}

static int lan865x_net_close(struct net_device* netdev) {
    struct lan865x_priv* priv = netdev_priv(netdev);
    int ret;

    netif_stop_queue(netdev);
    phy_stop(netdev->phydev);
    ret = lan865x_hw_disable(priv);
    if (ret) {
        netdev_err(netdev, "Failed to disable the hardware: %d\n", ret);
        return ret;
    }

    return 0;
}

static int lan865x_hw_enable(struct lan865x_priv* priv) {
    u32 regval;

    if (oa_tc6_read_register(priv->tc6, LAN865X_REG_MAC_NET_CTL, &regval)) {
        return -ENODEV;
    }

    regval |= MAC_NET_CTL_TXEN | MAC_NET_CTL_RXEN;

    if (oa_tc6_write_register(priv->tc6, LAN865X_REG_MAC_NET_CTL, regval)) {
        return -ENODEV;
    }

    return 0;
}

static int lan865x_net_open(struct net_device* netdev) {
    struct lan865x_priv* priv = netdev_priv(netdev);
    int ret;

    ret = lan865x_hw_enable(priv);
    if (ret) {
        netdev_err(netdev, "Failed to enable hardware: %d\n", ret);
        return ret;
    }

    phy_start(netdev->phydev);

    return 0;
}

static int lan865x_netdev_ioctl(struct net_device* netdev, struct ifreq* ifr, int cmd) {
    switch (cmd) {
    case SIOCGHWTSTAMP:
        return lan865x_get_ts_config(netdev, ifr);
    case SIOCSHWTSTAMP:
        return lan865x_set_ts_config(netdev, ifr);
    default:
        return -EOPNOTSUPP;
    }
}

static const struct net_device_ops lan865x_netdev_ops = {
    .ndo_open = lan865x_net_open,
    .ndo_stop = lan865x_net_close,
    .ndo_start_xmit = lan865x_send_packet,
    .ndo_set_rx_mode = lan865x_set_multicast_list,
    .ndo_set_mac_address = lan865x_set_mac_address,
    .ndo_eth_ioctl = lan865x_netdev_ioctl,
};

static long lan865x_ioctl(struct file* file, unsigned int cmd, unsigned long arg);
static int lan865x_open(struct inode* inode, struct file* file);
static int lan865x_release(struct inode* inode, struct file* file);
static const struct file_operations lan865x_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = lan865x_ioctl,
    .open = lan865x_open,
    .release = lan865x_release,
};

static struct miscdevice lan865x_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "lan865x",
    .fops = &lan865x_fops,
    .mode = 0666,
};

// TODO: Cleanup
static struct oa_tc6* g_tc6;

static int lan865x_probe(struct spi_device* spi) {
    struct net_device* netdev;
    struct lan865x_priv* priv;
    int ret;
    struct device* dev = &spi->dev;
    struct gpio_descs* nodeid_gpios;
    int gpio_values[4];
    u32 node_id = 0;
    u8 mac_addr[ETH_ALEN];

    netdev = alloc_etherdev(sizeof(struct lan865x_priv));
    if (!netdev) {
        return -ENOMEM;
    }

    priv = netdev_priv(netdev);
    priv->netdev = netdev;
    priv->spi = spi;
    spi_set_drvdata(spi, priv);
    INIT_WORK(&priv->multicast_work, lan865x_multicast_work_handler);

    // TODO: lan865x register init
    // ref: oa_tc6.c -> init_lan865x()
    // ref: oa_tc6.c -> set_macphy_register()
    // ref: oa_tc6.c -> indirect_read()

    priv->tc6 = oa_tc6_init(spi, netdev);
    // TODO: Cleanup
    g_tc6 = priv->tc6;
    if (!priv->tc6) {
        ret = -ENODEV;
        goto free_netdev;
    }

    /* As per the point s3 in the below errata, SPI receive Ethernet frame
     * transfer may halt when starting the next frame in the same data block
     * (chunk) as the end of a previous frame. The RFA field should be
     * configured to 01b or 10b for proper operation. In these modes, only
     * one receive Ethernet frame will be placed in a single data block.
     * When the RFA field is written to 01b, received frames will be forced
     * to only start in the first word of the data block payload (SWO=0). As
     * recommended, enable zero align receive frame feature for proper
     * operation.
     *
     * https://ww1.microchip.com/downloads/aemDocuments/documents/AIS/ProductDocuments/Errata/LAN8650-1-Errata-80001075.pdf
     */
    ret = oa_tc6_zero_align_receive_frame_enable(priv->tc6);
    if (ret) {
        dev_err(&spi->dev, "Failed to set ZARFE: %d\n", ret);
        goto oa_tc6_exit;
    }

    nodeid_gpios = devm_gpiod_get_array(dev, "nodeid", GPIOD_IN);
    if (IS_ERR(nodeid_gpios)) {
        dev_err(dev, "GPIO get array: %ld\n", PTR_ERR(nodeid_gpios));
        return PTR_ERR(nodeid_gpios);
    }

    for (int i = 0; i < nodeid_gpios->ndescs; i++) {
        gpio_values[i] = gpiod_get_value(nodeid_gpios->desc[i]);
    }

    for (int i = 0; i < nodeid_gpios->ndescs; i++) {
        node_id = (node_id << 1) + gpio_values[nodeid_gpios->ndescs - 1 - i];
    }

    /* Get the MAC address from the SPI device tree node */
    if (device_get_ethdev_address(&spi->dev, netdev)) {
        eth_hw_addr_random(netdev);
    }

    ret = lan865x_set_hw_macaddr(priv, netdev->dev_addr);
    if (ret) {
        dev_err(&spi->dev, "Failed to configure MAC: %d\n", ret);
        goto oa_tc6_exit;
    }

    netdev->if_port = IF_PORT_10BASET;
    netdev->irq = spi->irq;
    netdev->netdev_ops = &lan865x_netdev_ops;
    netdev->ethtool_ops = &lan865x_ethtool_ops;

    ret = register_netdev(netdev);
    if (ret) {
        dev_err(&spi->dev, "Register netdev failed (ret = %d)", ret);
        goto oa_tc6_exit;
    }

    ret = device_get_mac_address(&spi->dev, mac_addr);
    if (!ret) {
        mac_addr[ETH_ALEN - 1] = node_id & NODE_ID_MASK;
        eth_hw_addr_set(netdev, mac_addr);
        lan865x_set_hw_macaddr(priv, mac_addr);
    }

    ret = lan865x_set_nodeid(priv, node_id);
    if (ret) {
        dev_err(&spi->dev, "lan865x_set_nodeid failed (ret = %d)", ret);
        goto oa_tc6_exit;
    }

    priv->ptpdev = ptp_device_init(dev, priv->tc6, (s32)spi->max_speed_hz);
    if (!priv->ptpdev) {
        dev_err(dev, "ptp_device_init()");
        goto oa_tc6_exit;
    }

    return misc_register(&lan865x_miscdev);

oa_tc6_exit:
    oa_tc6_exit(priv->tc6);
free_netdev:
    free_netdev(priv->netdev);
    return ret;
}

static void lan865x_remove(struct spi_device* spi) {
    struct lan865x_priv* priv = spi_get_drvdata(spi);

    cancel_work_sync(&priv->multicast_work);
    unregister_netdev(priv->netdev);
    oa_tc6_exit(priv->tc6);
    free_netdev(priv->netdev);
    misc_deregister(&lan865x_miscdev);
}

// TODO: Cleanup
static long lan865x_ioctl(struct file* file, unsigned int cmd, unsigned long arg) {
    struct lan865x_reg reg;
    int ret = 0;

    switch (cmd) {
    case LAN865X_READ_REG:
        if (copy_from_user(&reg, (void __user*)arg, sizeof(reg))) {
            return -EFAULT;
        }

        ret = oa_tc6_read_register(g_tc6, reg.addr, &reg.value);

        if (ret < 0) {
            return ret;
        }

        if (copy_to_user((void __user*)arg, &reg, sizeof(reg))) {
            return -EFAULT;
        }
        break;

    case LAN865X_WRITE_REG:
        if (copy_from_user(&reg, (void __user*)arg, sizeof(reg))) {
            return -EFAULT;
        }

        ret = oa_tc6_write_register(g_tc6, reg.addr, reg.value);
        break;

    default:
        return -ENOTTY;
    }

    return ret;
}

static int lan865x_open(struct inode* inode, struct file* file) {
    struct spi_device* spi = container_of(file->private_data, struct spi_device, dev);
    file->private_data = spi;
    return 0;
}

static int lan865x_release(struct inode* inode, struct file* file) {
    file->private_data = NULL;
    return 0;
}

static const struct spi_device_id spidev_spi_ids[] = {
    {.name = "lan8650"},
    {},
};

static const struct of_device_id lan865x_dt_ids[] = {
    {.compatible = "microchip,lan8650"},
    {/* Sentinel */},
};
MODULE_DEVICE_TABLE(of, lan865x_dt_ids);

static struct spi_driver lan865x_driver = {
    .driver =
        {
            .name = DRV_NAME,
            .of_match_table = lan865x_dt_ids,
        },
    .probe = lan865x_probe,
    .remove = lan865x_remove,
    .id_table = spidev_spi_ids,
};
module_spi_driver(lan865x_driver);

MODULE_DESCRIPTION(DRV_NAME " 10Base-T1S MACPHY Ethernet Driver");
MODULE_AUTHOR("Parthiban Veerasooran <parthiban.veerasooran@microchip.com>");
MODULE_LICENSE("GPL");
