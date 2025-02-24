#pragma once

#include <stdint.h>

/**
 * Configures the LAN8650/1 for optimal performance in 10BASE-T1S networks
 *     - configure the PHY transceiver in the device
 *     - configure the MAC to set time stamping at the end of the Start of Frame Delimiter (SFD)
 *     - set the Timer Increment register to 40 ns to be used as a 25 MHz internal clock
 *     - (Optional) configure SQI(Signal Quality Indicator)
 *     - Enable PLCA
 *     - Enable unicast, multicast
 *     - Enable MACPHY TX, RX
 *     - Etc.
 * @param handle>=0, as returned by a call to spi_init
 * @return nothing
 */
void init_lan865x(unsigned int handle);

/**
 * Read the register value at addr address in mms
 * @param handle>=0, as returned by a call to spi_init
 * @param mms - Memory Map Selector
 *        MMS0 = 0x00  : Open Alliance 10BASE-T1x MAC-PHY Standard Registers
 *        MMS1 = 0x01, : MAC Registers
 *        MMS2 = 0x02, : PHY PCS Registers
 *        MMS3 = 0x03, : PHY PMA/PMD Registers
 *        MMS4 = 0x04, : PHY Vendor Specific Registers
 *        MMS10 = 0x0A,: Miscellaneous Register Descriptions
 * @param addr address
 * @return the value of register if OK, otherwise 0.
 */
uint32_t read_register(unsigned int handle, uint8_t mms, uint16_t addr);

/**
 * Write the value into the register at addr address in mms
 * @param handle>=0, as returned by a call to spi_init
 * @param mms - Memory Map Selector
 *        MMS0 = 0x00  : Open Alliance 10BASE-T1x MAC-PHY Standard Registers
 *        MMS1 = 0x01, : MAC Registers
 *        MMS2 = 0x02, : PHY PCS Registers
 *        MMS3 = 0x03, : PHY PMA/PMD Registers
 *        MMS4 = 0x04, : PHY Vendor Specific Registers
 *        MMS10 = 0x0A,: Miscellaneous Register Descriptions
 * @param addr address
 * @param val value to be written into the register
 * @return RET_SUCCESS if OK, otherwise -RET_FAIL.
 */
int32_t write_register(unsigned int handle, uint8_t mms, uint16_t addr, uint32_t val);

/**
 * Get the MAC address set on the chip
 * @param handle>=0, as returned by a call to spi_init
 * @return MAC address
 */
uint64_t get_mac_address_on_chip(unsigned int handle);

/**
 * Set the MAC address to the chip
 * @param handle>=0, as returned by a call to spi_init
 * @param mac, MAC address
 * @return RET_SUCCESS if OK, otherwise -RET_FAIL.
 */
int set_mac_address_to_chip(unsigned int handle, uint64_t mac);

/**
 * Set the Node ID & Node count to the chip
 * @param handle>=0, as returned by a call to spi_init
 * @param node_id, 0 PLCA Coordinator node Local ID
 *                 1-0xFE PLCA Follower node Local ID
 *                 0xFF PLCA Disabled
 * @param node_count, the maximum number of nodes supported on the multidrop network.
 *                 Valid range: 0x01-0xFF
 * @return RET_SUCCESS if OK, otherwise -RET_FAIL.
 */
int set_node_config(unsigned int handle, int node_id, int node_cnt);

/**
 * Read all register values in mms
 * @param handle>=0, as returned by a call to spi_init
 * @param mms - Memory Map Selector
 *        MMS0 = 0x00  : Open Alliance 10BASE-T1x MAC-PHY Standard Registers
 *        MMS1 = 0x01, : MAC Registers
 *        MMS2 = 0x02, : PHY PCS Registers
 *        MMS3 = 0x03, : PHY PMA/PMD Registers
 *        MMS4 = 0x04, : PHY Vendor Specific Registers
 *        MMS10 = 0x0A,: Miscellaneous Register Descriptions
 * @return RET_SUCCESS if OK, otherwise -RET_FAIL.
 */
int read_all_registers_in_mms(unsigned int handle, uint8_t mms);

/**
 * After reading the current value of the register at the addr address in mms, write a new value and read the changed
 * value.
 * @param handle>=0, as returned by a call to spi_init
 * @param mms - Memory Map Selector
 *        MMS0 = 0x00  : Open Alliance 10BASE-T1x MAC-PHY Standard Registers
 *        MMS1 = 0x01, : MAC Registers
 *        MMS2 = 0x02, : PHY PCS Registers
 *        MMS3 = 0x03, : PHY PMA/PMD Registers
 *        MMS4 = 0x04, : PHY Vendor Specific Registers
 *        MMS10 = 0x0A,: Miscellaneous Register Descriptions
 * @param addr address
 * @param val value to be written into the register
 * @return RET_SUCCESS if OK, otherwise -RET_FAIL.
 */
int write_register_in_mms(unsigned int handle, uint8_t mms, int32_t addr, uint32_t data);

/**
 * Transmit a packet of length bytes
 * @param handle>=0, as returned by a call to spi_init
 * @param * packet, Starting address of packet to be transmitted
 * @param lengthi, packet length
 * @return RET_SUCCESS if OK, otherwise error code.
 */
int spi_transmit_frame(unsigned int handle, uint8_t* packet, uint16_t length);

/**
 * Receive a packet
 * @param handle>=0, as returned by a call to spi_init
 * @param * packet, Buffer pointer to store received packet
 * @param * lengthi, Variable pointer to store the length of the received packet
 * @return RET_SUCCESS if OK, otherwise error code.
 */
int spi_receive_frame(unsigned int handle, uint8_t* packet, uint16_t* length);
