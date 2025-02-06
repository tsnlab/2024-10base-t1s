#include <getopt.h>

#include "arch.h"
#include "arp_test.h"
#include "register.h"
#include "spi.h"

int drv_main(int argc, char* argv[]) {
    int arp_ret = ARP_E_UNKNOWN_ERROR;
    int spi_ret = SPI_E_UNKNOWN_ERROR;
    int plca_mode = PLCA_MODE_INVALID;
    bool reg_initstatus = false;
    int opt = 0;
    uint32_t regval;

    spi_ret = api_spi_init();
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

    regval = read_register(MMS1, MAC_NCFGR);
    printf_debug("MAC_NCFGR value after ARP test is %x\n", regval);
    regval = read_register(MMS1, STATS2);
    printf_debug("STATS2 value after ARP test is %x\n", regval);
    regval = read_register(MMS4, MIDVER);
    printf_debug("MIDVER value after ARP test is %x\n", regval);
    regval = read_register(MMS4, PLCA_CTRL0);
    printf_debug("PLCA_CTRL0 value after ARP test is %x\n", regval);
    regval = read_register(MMS4, PLCA_CTRL1);
    printf_debug("PLCA_CTRL1 value after ARP test is %x\n", regval);
    regval = read_register(MMS4, PLCA_STS);
    printf_debug("PLCA_STS value after ARP test is %x\n", regval);
    regval = read_register(MMS0, OA_CONFIG0);
    printf_debug("OA_CONFIG0 value after ARP test is %x\n", regval);

cleanup:
    spi_ret = spi_cleanup();
    if (spi_ret != SPI_E_SUCCESS) {
        printf_debug("spi_cleanup failed; the error code is %d\n", spi_ret);
    }

    return spi_ret;
}

int drv_init_client() {
    int spi_ret = SPI_E_UNKNOWN_ERROR;
    int plca_mode;
    bool reg_initstatus = false;

    spi_ret = api_spi_init();
    if (spi_ret != SPI_E_SUCCESS) {
        printf_debug("spi_init failed; the error code is %d\n", spi_ret);
    }

    printf("Follower mode\n");
    plca_mode = PLCA_MODE_FOLLOWER;
    reg_initstatus = set_register(plca_mode);

    return 0;
}

int drv_init_server() {
    int spi_ret = SPI_E_UNKNOWN_ERROR;
    int plca_mode;
    bool reg_initstatus = false;

    spi_ret = api_spi_init();
    if (spi_ret != SPI_E_SUCCESS) {
        printf_debug("spi_init failed; the error code is %d\n", spi_ret);
    }

    printf("Coordinator mode\n");
    plca_mode = PLCA_MODE_COORDINATOR;
    reg_initstatus = set_register(plca_mode);

    return 0;
}

int drv_spi_cleanup() {
    int spi_ret = SPI_E_UNKNOWN_ERROR;

    spi_ret = spi_cleanup();
    if (spi_ret != SPI_E_SUCCESS) {
        printf_debug("spi_cleanup failed; the error code is %d\n", spi_ret);
    }

    return spi_ret;
}

int drv_client_main() {
    int arp_ret = ARP_E_UNKNOWN_ERROR;
    int spi_ret = SPI_E_UNKNOWN_ERROR;
    int plca_mode = PLCA_MODE_INVALID;
    bool reg_initstatus = false;
    int opt = 0;
    uint32_t regval;
    static uint32_t try = 0;

    plca_mode = PLCA_MODE_FOLLOWER;

    printf("\n");
    try++;
    printf("[TRY: %6d]\n", try);
    arp_ret = arp_test(plca_mode);
    printf("Result of arp_test is %d\n", arp_ret);

    regval = read_register(MMS1, MAC_NCFGR);
    printf_debug("MAC_NCFGR value after ARP test is %x\n", regval);
    regval = read_register(MMS1, STATS2);
    printf_debug("STATS2 value after ARP test is %x\n", regval);
    regval = read_register(MMS4, MIDVER);
    printf_debug("MIDVER value after ARP test is %x\n", regval);
    regval = read_register(MMS4, PLCA_CTRL0);
    printf_debug("PLCA_CTRL0 value after ARP test is %x\n", regval);
    regval = read_register(MMS4, PLCA_CTRL1);
    printf_debug("PLCA_CTRL1 value after ARP test is %x\n", regval);
    regval = read_register(MMS4, PLCA_STS);
    printf_debug("PLCA_STS value after ARP test is %x\n", regval);
    regval = read_register(MMS0, OA_CONFIG0);
    printf_debug("OA_CONFIG0 value after ARP test is %x\n", regval);

    return 0;
}

int drv_server_main() {
    int arp_ret = ARP_E_UNKNOWN_ERROR;
    int spi_ret = SPI_E_UNKNOWN_ERROR;
    int plca_mode = PLCA_MODE_INVALID;
    bool reg_initstatus = false;
    int opt = 0;
    uint32_t regval;
    static uint32_t try = 0;

    plca_mode = PLCA_MODE_COORDINATOR;

    printf("\n");
    try++;
    printf("[TRY: %6d]\n", try);
    arp_ret = arp_test(plca_mode);
    printf("Result of arp_test is %d\n", arp_ret);

    regval = read_register(MMS1, MAC_NCFGR);
    printf_debug("MAC_NCFGR value after ARP test is %x\n", regval);
    regval = read_register(MMS1, STATS2);
    printf_debug("STATS2 value after ARP test is %x\n", regval);
    regval = read_register(MMS4, MIDVER);
    printf_debug("MIDVER value after ARP test is %x\n", regval);
    regval = read_register(MMS4, PLCA_CTRL0);
    printf_debug("PLCA_CTRL0 value after ARP test is %x\n", regval);
    regval = read_register(MMS4, PLCA_CTRL1);
    printf_debug("PLCA_CTRL1 value after ARP test is %x\n", regval);
    regval = read_register(MMS4, PLCA_STS);
    printf_debug("PLCA_STS value after ARP test is %x\n", regval);
    regval = read_register(MMS0, OA_CONFIG0);
    printf_debug("OA_CONFIG0 value after ARP test is %x\n", regval);

    return 0;
}
