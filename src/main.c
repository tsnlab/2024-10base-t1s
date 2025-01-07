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
            reg_initstatus = init_register(plca_mode);
            break;
        case 'f':
            printf("Follower mode\n");
            plca_mode = PLCA_MODE_FOLLOWER;
            reg_initstatus = init_register(plca_mode);
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

    if (optind < argc || argc == 1u) {
        printf("Invalid argument: %s\n Usage: %s -c|-f|-h\n", argv[optind], argv[0]);
        goto cleanup;
    }

    arp_ret = arp_test(plca_mode);
    printf("Result of arp_test is %d\n", arp_ret);

    regval = read_register(0x01u, 0x0001u);
    printf_debug("MAC_NCFGR value after ARP test is %x\n", regval);
    regval = read_register(0x01u, 0x020Au);
    printf_debug("STATS2 value after ARP test is %x\n", regval);
    regval = read_register(0x04u, 0xCA00u);
    printf_debug("MMS4, 0xCA00 value after ARP test is %x\n", regval);
    regval = read_register(0x04u, 0xCA01u);
    printf_debug("MMS4, 0xCA01 value after ARP test is %x\n", regval);
    regval = read_register(0x04u, 0xCA02u);
    printf_debug("MMS4, 0xCA02 value after ARP test is %x\n", regval);
    regval = read_register(0x04u, 0xCA03u);
    printf_debug("MMS4, 0xCA03 value after ARP test is %x\n", regval);

    //test
    while(1) {

    }

cleanup:
    spi_ret = spi_cleanup();
    if (spi_ret != SPI_E_SUCCESS) {
        printf_debug("spi_cleanup failed; the error code is %d\n", spi_ret);
    }

    return spi_ret;
}
