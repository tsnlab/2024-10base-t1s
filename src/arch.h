#include <stdint.h>

/* This header file is based on RaspberryPi 4 and LAN8651 */
#ifndef ARCH_H
#define ARCH_H

#ifdef DEBUG
#define printf_debug(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define printf_debug(fmt, ...) \
    do {                       \
    } while (0)
#endif

#define MAX_CONTROL_CMD_LEN ((uint8_t)(0x7F))
#define MAX_PAYLOAD_BYTE ((uint8_t)64) // ToDo This is configurable so need to change based on configuration
#define EACH_REG_SIZE ((uint8_t)4)
#define MAX_REG_DATA_ONECONTROLCMD ((uint8_t)(MAX_CONTROL_CMD_LEN * EACH_REG_SIZE))
#define MAX_REG_DATA_ONECHUNK ((uint8_t)(MAX_PAYLOAD_BYTE / EACH_REG_SIZE))
#define MAX_DATA_DWORD_ONECHUNK (MAX_REG_DATA_ONECHUNK)
#define HEADER_FOOTER_SIZE ((uint8_t)4)
#define SIZE_OF_MAC_ADDR ((uint8_t)6)

enum { DNC_COMMANDTYPE_CONTROL = 0, DNC_COMMANDTYPE_DATA };

enum { REG_ADDR_INCREMENT_ENABLE = 0, REG_ADDR_INCREMENT_DISABLE };

enum { REG_COMMAND_TYPE_READ = 0, REG_COMMAND_TYPE_WRITE };

struct st_ctrl_cmd_reg {
    uint8_t memorymap;
    uint8_t length;
    uint16_t address;
    uint32_t databuffer[MAX_CONTROL_CMD_LEN];
};

union u_ctrl_header_footer {
    uint8_t ctrl_header_array[HEADER_FOOTER_SIZE];
    uint32_t ctrl_frame_head;
    struct {
        uint32_t P : 1;
        uint32_t LEN : 7;
        uint32_t ADDR : 16;
        uint32_t MMS : 4;
        uint32_t AID : 1;
        uint32_t WNR : 1;
        uint32_t HDRB : 1;
        uint32_t DNC : 1;
    } st_ctrl_head_foot_bits;
};

union u_data_header_footer {
    uint32_t data_frame_head_foot;
    uint8_t data_frame_header_buffer[HEADER_FOOTER_SIZE];
    struct {
        uint32_t P : 1;
        uint32_t RSVD3 : 5;
        uint32_t TSC : 2;
        uint32_t EBO : 6;
        uint32_t EV : 1;
        uint32_t RSVD2 : 1;
        uint32_t SWO : 4;
        uint32_t SV : 1;
        uint32_t DV : 1;
        uint32_t VS : 2;
        uint32_t RSVD1 : 5;
        uint32_t NORX : 1;
        uint32_t SEQ : 1;
        uint32_t DNC : 1;
    } st_tx_header_bits;

    struct {
        uint32_t P : 1;
        uint32_t TXC : 5;
        uint32_t RTPS : 1;
        uint32_t RTSA : 1;
        uint32_t EBO : 6;
        uint32_t EV : 1;
        uint32_t FD : 1;
        uint32_t SWO : 4;
        uint32_t SV : 1;
        uint32_t DV : 1;
        uint32_t VS : 2;
        uint32_t RBA : 5;
        uint32_t SYNC : 1;
        uint32_t HDRB : 1;
        uint32_t EXST : 1;
    } st_rx_footer_bits;
};

#endif /* ARCH_H */
