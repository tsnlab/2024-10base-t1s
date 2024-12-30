#include <arp_test.h>
#include <hardware_dependent.h>
#include <spi.h>

int main(int argc, char* argv[]) {
    ARP_ReturnType arpRet = ARP_E_UNKNOWN_ERROR;
    SPI_ReturnType spiRet = SPI_E_UNKNOWN_ERROR;
    uint8_t MMS = 0u;
    uint16_t Address = 0x0004u;
    uint32_t data = 0u;
    bool executionStatus = false;
    // PLCA setting must be done before arp_test

    spiRet = SPI_Init();
    if (spiRet != SPI_E_SUCCESS) {
        printf("SPI_Init failed; the error code is %d\n", spiRet);
    }

    data = ReadRegister(MMS, Address);
    printf("read data from MMS %d, Address %d is 0x%08X\n", MMS, Address, data);

    MMS = 1u;
    Address = 0x0000u;

    data = ReadRegister(MMS, Address);
    printf("read data before writing to MMS %d, Address %d is 0x%08X\n", MMS, Address, data);

    data = 0x103u;
    executionStatus = WriteRegister(MMS, Address, data);
    if (executionStatus == true) {
        printf("Write Register to MMS %d, Address %d is successful\n", MMS, Address);
    } else {
        printf("Write Register to MMS %d, Address %d is failed\n", MMS, Address);
    }

    data = ReadRegister(MMS, Address);
    printf("read data after writing to MMS %d, Address %d is 0x%08X\n", MMS, Address, data);

    /*
        arpRet = ArpTest();
        printf("Result of arp_test is %d\n", arpRet);
     */
    spiRet = SPI_Cleanup();
    if (spiRet != SPI_E_SUCCESS) {
        printf("SPI_Cleanup failed; the error code is %d\n", spiRet);
    }

    return spiRet;
}
