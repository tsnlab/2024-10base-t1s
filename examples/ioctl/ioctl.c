#include "ioctl.h"

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>

uint32_t do_ioctl(int mms, unsigned long addr_offset, unsigned long value, int operation) {
    int fd;
    struct lan865x_reg reg;
    uint32_t ret = 0;

    /* Open device file */
    fd = open("/dev/lan865x", O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return ret;
    }

    reg.addr = (uint32_t)((mms << 16) | (addr_offset & 0xFFFF));
    reg.value = (uint32_t)(value & 0xFFFF);

    switch (operation) {
    case 0: /* read operation */
        if (ioctl(fd, LAN865X_READ_REG, &reg) < 0) {
            perror("Fail to read register");
            close(fd);
            return ret;
        }

        ret = (uint32_t)(reg.value & 0xFFFF);

        break;

    case 1: /* write operation */
        if (ioctl(fd, LAN865X_WRITE_REG, &reg) < 0) {
            perror("Register write failure");
            close(fd);
            return ret;
        }
        break;
    }

    close(fd);
    return ret;
}

uint32_t read_register(uint8_t mms, uint16_t address) {

    unsigned long addr_offset = (unsigned long)address;

    return do_ioctl(mms, addr_offset, 0, 0);
}

static void dump_reginfo(int mms, struct reginfo* reginfo) {

    for (int i = 0; reginfo[i].address >= 0; i++) {
        printf("address: 0x%04x - value: 0x%08x - %s\n", reginfo[i].address,
               read_register(mms, (uint16_t)reginfo[i].address), reginfo[i].desc);
    }
}

/* read all register values in memory map selector */
static int read_register_in_mms(int mms) {
    switch (mms) {
    case MMS0: /* Open Alliance 10BASE-T1x MAC-PHY Standard Registers */
        dump_reginfo(mms, reg_open_alliance);
        break;
    case MMS1: /* MAC Registers */
        dump_reginfo(mms, reg_mac);
        break;
    case MMS2: /* PHY PCS Registers */
        dump_reginfo(mms, reg_phy_pcs);
        break;
    case MMS3: /* PHY PMA/PMD Registers */
        dump_reginfo(mms, reg_phy_pma_pmd);
        break;
    case MMS4: /* PHY Vendor Specific Registers */
        dump_reginfo(mms, reg_phy_vendor_specific);
        break;
    case MMS10: /* Miscellaneous Register Descriptions */
        dump_reginfo(mms, reg_miscellaneous);
        break;
    default:
        printf("%s - Unknown memory map selector(0x%02x)\n", __func__, mms);
        return -1;
    }

    return 0;
}

#define MAIN_READ_OPTION_STRING "o:m:a:v:h"
int main(int argc, char* argv[]) {
    int mms = 0;
    unsigned long addr_offset = 0;
    unsigned long value = 0;

    int argflag;
    int operation = 0;

    while ((argflag = getopt(argc, (char**)argv, MAIN_READ_OPTION_STRING)) != -1) {
        switch (argflag) {
        case 'o':
            if (strcmp(optarg, "rd") == 0) {
                operation = 0;
            } else if (strcmp(optarg, "wr") == 0) {
                operation = 1;
            } else if (strcmp(optarg, "mms") == 0) {
                operation = 2;
            } else {
                fprintf(stderr, "Invalid operation: %s\n", optarg);
                return 1;
            }
            break;
        case 'm':
            mms = strtoul(optarg, NULL, 16);
            switch (mms) {
            case 0x00: /* Open Alliance 10BASE-T1x MAC-PHY Standard Registers */
            case 0x01: /* MAC Registers */
            case 0x02: /* PHY PCS Registers */
            case 0x03: /* PHY PMA/PMD Registers */
            case 0x04: /* PHY Vendor Specific Registers */
            case 0x0A: /* Miscellaneous Register Descriptions */
                break;
            default:
                printf("mms %d is out of range.\n", mms);
                return -1;
            }
            break;
        case 'a':
            addr_offset = strtoul(optarg, NULL, 16);
            break;

        case 'v':
            value = strtoul(optarg, NULL, 16);
            break;

        case 'h':
            fprintf(stderr, "Usage: %s -o <rd/wr/mms> -m <mms(hex)> -a <address(hex)> [ -v <value(hex)>]\n", argv[0]);
            return 0;
        }
    }

    switch (operation) {
    case 0:
        printf("MMS [0x%02x] register [0x%04x] value: 0x%04x\n", mms, (int)(addr_offset & 0xFFFF),
               do_ioctl(mms, addr_offset, value, operation));
        break;
    case 1:
        return do_ioctl(mms, addr_offset, value, operation);
    case 2:
        return read_register_in_mms(mms);
    }

    return 0;
}
