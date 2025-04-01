#include "ioctl.h"

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>

int do_ioctl(int mms, unsigned long addr_offset, unsigned long value, int operation) {
    int fd;
    struct lan865x_reg reg;

    /* Open device file */
    fd = open("/dev/lan865x", O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return -1;
    }

    reg.addr = (uint32_t)((mms << 16) | (addr_offset & 0xFFFF));
    reg.value = (uint32_t)(value & 0xFFFF);

    switch (operation) {
    case 0: /* read operation */
        if (ioctl(fd, LAN865X_READ_REG, &reg) < 0) {
            perror("Fail to read register");
            close(fd);
            return -1;
        }

        printf("MMS [0x%02x] register [0x%04x] value: 0x%04x\n", mms, reg.addr & 0xFFFF, reg.value);

        break;

    case 1: /* write operation */
        if (ioctl(fd, LAN865X_WRITE_REG, &reg) < 0) {
            perror("Register write failure");
            close(fd);
            return -1;
        }
        break;
    }

    close(fd);
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
            if (strcmp(optarg, "rd") == 0)
                operation = 0;
            else if (strcmp(optarg, "wr") == 0)
                operation = 1;
            else {
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
            fprintf(stderr, "Usage: %s -o <rd/wr> -m <mms(hex)> -a <address(hex)> [ -v <value(hex)>]\n", argv[0]);
            return 0;
        }
    }

    return do_ioctl(mms, addr_offset, value, operation);
}
