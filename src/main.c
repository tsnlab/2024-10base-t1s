#include "arch.h"
#include "arp_test.h"
#include "spi.h"

int main(int argc, char* argv[]) {
    int arp_ret = ARP_E_UNKNOWN_ERROR;
    int spi_ret = SPI_E_UNKNOWN_ERROR;
    int plca_mode = PLCA_MODE_INVALID;
    bool reg_initstatus = false;
    uint8_t argcount;
    uint32_t regval;

    spi_ret = spi_init();
    if (spi_ret != SPI_E_SUCCESS) {
        printf_debug("spi_init failed; the error code is %d\n", spi_ret);
    }

    if (argc == 1u) {
        printf("No argument provided\n Usage: %s -c|-f|-h\n -c   : Coordinator mode\n -f   : Follower mode\n -h   : "
               "Help mode\n",
               argv[0]);
    }
    // Register initialization based on argument(c, f, h)
    for (argcount = 1u; argcount < argc; argcount++) {
        if (strcmp(argv[argcount], "-c") == 0) {
            printf("Coordinator mode\n");
            plca_mode = PLCA_MODE_COORDINATOR;
            reg_initstatus = init_register(plca_mode);
        } else if (strcmp(argv[argcount], "-f") == 0) {
            printf("Follower mode\n");
            plca_mode = PLCA_MODE_FOLLOWER;
            reg_initstatus = init_register(plca_mode);
        } else if (strcmp(argv[argcount], "-h") == 0) {
            printf(
                "Help mode\n Usage: %s -c|-f|-h\n -c   : Coordinator mode\n -f   : Follower mode\n -h   : Help mode\n",
                argv[0]);
        } else {
            printf("Invalid argument: %s\n Usage: %s -c|-f|-h\n", argv[argcount], argv[0]);
        }
        printf_debug("Register initialization %s\n", reg_initstatus ? "successful" : "failed");
    }

    arp_ret = arp_test(plca_mode);
    printf("Result of arp_test is %d\n", arp_ret);

    regval = read_register(0x01u, 0x0001u);
    printf_debug("MAC_NCFGR value after ARP test is %x\n", regval);
    regval = read_register(0x01u, 0x020Au);
    printf_debug("STATS2 value after ARP test is %x\n", regval);

    spi_ret = spi_cleanup();
    if (spi_ret != SPI_E_SUCCESS) {
        printf_debug("spi_cleanup failed; the error code is %d\n", spi_ret);
    }

    return spi_ret;
}
