#include <arp_test.h>
#include <hardware_dependent.h>
#include <spi.h>

int main(int argc, char* argv[]) {
    ARP_ReturnType arpRet = ARP_E_UNKNOWN_ERROR;
    SPI_ReturnType spiRet = SPI_E_UNKNOWN_ERROR;
    // PLCA setting must be done before arp_test

    spiRet = SPI_Init();
    if (spiRet != SPI_E_SUCCESS) {
        printf("SPI_Init failed; the error code is %d\n", spiRet);
    }

    /*     arpRet = ArpTest();
        printf("Result of arp_test is %d\n", arpRet);
     */
    spiRet = SPI_Cleanup();
    if (spiRet != SPI_E_SUCCESS) {
        printf("SPI_Cleanup failed; the error code is %d\n", spiRet);
    }

    return spiRet;
}
