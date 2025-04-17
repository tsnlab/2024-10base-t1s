#include "ioctl.h"

#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>

uint32_t do_ioctl(int mms, unsigned long addr_offset, unsigned long value, enum oper_type operation) {
    int file_descriptor;
    struct lan865x_reg reg;
    uint32_t ret = 0;

    /* Open device file */
    file_descriptor = open("/dev/lan865x", O_RDWR);
    if (file_descriptor < 0) {
        perror("Failed to open device");
        return ret;
    }

    reg.addr = (uint32_t)((mms << ADDR_WIDTH) | (addr_offset & ADDR_MASK));
    reg.value = (uint32_t)(value & VALUE_MASK);

    switch (operation) {
    case READ_OPERATION: /* read operation */
        if (ioctl(file_descriptor, LAN865X_READ_REG, &reg) < 0) {
            perror("Fail to read register");
            close(file_descriptor);
            return ret;
        }

        ret = (uint32_t)(reg.value & VALUE_MASK);

        break;

    case WRITE_OPERATION: /* write operation */
        if (ioctl(file_descriptor, LAN865X_WRITE_REG, &reg) < 0) {
            perror("Register write failure");
            close(file_descriptor);
            return ret;
        }
        break;
    }

    close(file_descriptor);
    return ret;
}

uint32_t read_register(uint8_t mms, uint16_t address) {

    unsigned long addr_offset = (unsigned long)address;

    return do_ioctl(mms, addr_offset, 0, READ_OPERATION);
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
    int mms = MMS0;
    unsigned long addr_offset = 0;
    unsigned long value = 0;

    int argflag;
    int operation = READ_OPERATION;

    while ((argflag = getopt(argc, argv, MAIN_READ_OPTION_STRING)) != -1) {
        switch (argflag) {
        case 'o':
            if (strcmp(optarg, "rd") == 0) {
                operation = READ_OPERATION;
            } else if (strcmp(optarg, "wr") == 0) {
                operation = WRITE_OPERATION;
            } else if (strcmp(optarg, "mms") == 0) {
                operation = MMS_OPERATION;
            } else {
                fprintf(stderr, "Invalid operation: %s\n", optarg);
                return 1;
            }
            break;
        case 'm':
            mms = (int)strtoul(optarg, NULL, HEX_BASE);
            switch (mms) {
            case MMS0:  /* Open Alliance 10BASE-T1x MAC-PHY Standard Registers */
            case MMS1:  /* MAC Registers */
            case MMS2:  /* PHY PCS Registers */
            case MMS3:  /* PHY PMA/PMD Registers */
            case MMS4:  /* PHY Vendor Specific Registers */
            case MMS10: /* Miscellaneous Register Descriptions */
                break;
            default:
                printf("mms %d is out of range.\n", mms);
                return -1;
            }
            break;
        case 'a':
            addr_offset = strtoul(optarg, NULL, HEX_BASE);
            break;

        case 'v':
            value = strtoul(optarg, NULL, HEX_BASE);
            break;

        case 'h':
            fprintf(stderr, "Usage: %s -o <rd/wr/mms> -m <mms(hex)> -a <address(hex)> [ -v <value(hex)>]\n", argv[0]);
            return 0;
        default:
            fprintf(stderr, "Unknown option: %c\n", argflag);
            fprintf(stderr, "Usage: %s -o <rd/wr/mms> -m <mms(hex)> -a <address(hex)> [ -v <value(hex)>]\n", argv[0]);
            break;
        }
    }

    switch (operation) {
    case READ_OPERATION:
        printf("MMS [0x%02x] register [0x%04x] value: 0x%04x\n", mms, (int)(addr_offset & ADDR_MASK),
               do_ioctl(mms, addr_offset, value, operation));
        break;
    case WRITE_OPERATION:
        do_ioctl(mms, addr_offset, value, operation);
        break;
    case MMS_OPERATION:
        return read_register_in_mms(mms);
    default:
        fprintf(stderr, "Unknown option: %c\n", argflag);
        break;
    }

    return 0;
}
