#include <getopt.h>

#include "arch.h"
#include "arp_test.h"
#include "spi.h"

int main(int argc, char* argv[]) {
    int arp_ret = ARP_E_UNKNOWN_ERROR;
    int spi_ret = SPI_E_UNKNOWN_ERROR;
    int plca_mode = PLCA_MODE_INVALID;
    bool reg_initstatus = false;
    int opt = 0;
    uint32_t regval;

    spi_ret = spi_init();
    if (spi_ret != SPI_E_SUCCESS) {
        printf_debug("spi_init failed; the error code is %d\n", spi_ret);
    }

    // Register initialization based on argument(c, f, h)
    while ((opt = getopt(argc, argv, "cfh")) != -1) {
        switch (opt) {
        case 'c':
            printf("Coordinator mode\n");
            plca_mode = PLCA_MODE_COORDINATOR;
            reg_initstatus = set_register(plca_mode);
            break;
        case 'f':
            printf("Follower mode\n");
            plca_mode = PLCA_MODE_FOLLOWER;
            reg_initstatus = set_register(plca_mode);
            break;
        case 'h':
            printf(
                "Help mode\n Usage: %s -c|-f|-h\n -c   : Coordinator mode\n -f   : Follower mode\n -h   : Help mode\n",
                argv[0]);
            break;
        default: /* '?' */
            printf("Invalid argument: %s\n Usage: %s -c|-f|-h\n", argv[opt], argv[0]);
            goto cleanup;
        }
        printf_debug("Register initialization %s\n", reg_initstatus ? "successful" : "failed");
    }

    if (optind < argc || argc == 1) {
        printf("Invalid argument: %s\n Usage: %s -c|-f|-h\n", argv[optind], argv[0]);
        goto cleanup;
    }

    arp_ret = arp_test(plca_mode);
    printf("Result of arp_test is %d\n", arp_ret);

    regval = read_register(0x01, 0x0001);
    printf_debug("MAC_NCFGR value after ARP test is %x\n", regval);
    regval = read_register(0x01, 0x020A);
    printf_debug("STATS2 value after ARP test is %x\n", regval);
    regval = read_register(0x04, 0xCA00);
    printf_debug("MMS4, 0xCA00 value after ARP test is %x\n", regval);
    regval = read_register(0x04, 0xCA01);
    printf_debug("MMS4, 0xCA01 value after ARP test is %x\n", regval);
    regval = read_register(0x04, 0xCA02);
    printf_debug("MMS4, 0xCA02 value after ARP test is %x\n", regval);
    regval = read_register(0x04, 0xCA03);
    printf_debug("MMS4, 0xCA03 value after ARP test is %x\n", regval);
    regval = read_register(0x00, 0x0004);
    printf_debug("OA_CONFIG0 value after ARP test is %x\n", regval);

cleanup:
    spi_ret = spi_cleanup();
    if (spi_ret != SPI_E_SUCCESS) {
        printf_debug("spi_cleanup failed; the error code is %d\n", spi_ret);
    }

    return spi_ret;
}
