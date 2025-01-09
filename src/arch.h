/* This header file is based on RaspberryPi 4 and LAN8651 */
#ifndef ARCH_H
#define ARCH_H

#include <stdint.h>
// clang-format off
#ifdef DEBUG
#define printf_debug(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define printf_debug(fmt, ...) do {} while (0)
#endif

#define MAX_CONTROL_CMD_LEN (0x7F)
#define MAX_PAYLOAD_BYTE (64) // TODO: This is configurable so need to change based on configuration
#define REG_SIZE (4)
#define MAX_REG_DATA_ONECONTROLCMD (MAX_CONTROL_CMD_LEN * REG_SIZE)
#define MAX_REG_DATA_ONECHUNK (MAX_PAYLOAD_BYTE / REG_SIZE)
#define MAX_DATA_DWORD_ONECHUNK (MAX_REG_DATA_ONECHUNK)
#define HEADER_SIZE (4)
#define FOOTER_SIZE (4)

enum {
    DNC_COMMANDTYPE_CONTROL = 0,
    DNC_COMMANDTYPE_DATA
    };

enum {
    REG_ADDR_INCREMENT_ENABLE = 0,
    REG_ADDR_INCREMENT_DISABLE
    };

enum {
    REG_COMMAND_TYPE_READ = 0,
    REG_COMMAND_TYPE_WRITE
    };
// clang-format on

struct ctrl_cmd_reg {
    uint8_t memorymap;
    uint8_t length;
    uint16_t address;
    uint32_t databuffer[MAX_CONTROL_CMD_LEN];
};

union ctrl_header {
    uint32_t ctrl_frame_head;
    struct {
        uint32_t p : 1;
        uint32_t len : 7;
        uint32_t addr : 16;
        uint32_t mms : 4;
        uint32_t aid : 1;
        uint32_t wnr : 1;
        uint32_t hdrb : 1;
        uint32_t dnc : 1;
    } ctrl_head_bits;
};

union data_header {
    uint32_t data_frame_head;
    struct {
        uint32_t p : 1;
        uint32_t rsvd3 : 5;
        uint32_t tsc : 2;
        uint32_t ebo : 6;
        uint32_t ev : 1;
        uint32_t rsvd2 : 1;
        uint32_t swo : 4;
        uint32_t sv : 1;
        uint32_t dv : 1;
        uint32_t vs : 2;
        uint32_t rsvd1 : 5;
        uint32_t norx : 1;
        uint32_t seq : 1;
        uint32_t dnc : 1;
    } tx_header_bits;
};

union data_footer {
    uint32_t data_frame_foot;
    struct {
        uint32_t p : 1;
        uint32_t txc : 5;
        uint32_t rtps : 1;
        uint32_t rtsa : 1;
        uint32_t ebo : 6;
        uint32_t ev : 1;
        uint32_t fd : 1;
        uint32_t swo : 4;
        uint32_t sv : 1;
        uint32_t dv : 1;
        uint32_t vs : 2;
        uint32_t rba : 5;
        uint32_t sync : 1;
        uint32_t hdrb : 1;
        uint32_t exst : 1;
    } rx_footer_bits;
};

#endif /* ARCH_H */
