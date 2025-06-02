#ifndef API_DEVICE_API_DEVICE_DRIVER_H
#define API_DEVICE_API_DEVICE_DRIVER_H

#include <stdint.h>

int api_spi_init(void);
int api_spi_cleanup(void);

int api_read_register_in_register_group(int reg_grp);
int api_read_register_in_mms(int mms);
int api_write_register_in_register_group(int reg_grp, int addr, uint32_t data);
int api_write_register_in_mms(int mms, int addr, uint32_t data);
uint64_t api_get_mac_address();
int api_config_node(int node_id, int node_cnt);
int api_config_mac_address(uint64_t mac);
int api_configure_plca_to_mac_phy();

int drv_init_client();
int drv_init_server();
int drv_spi_cleanup();

#endif /* API_DEVICE_API_DEVICE_DRIVER_H */
