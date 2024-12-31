#include <arp_test.h>
#include <hardware_dependent.h>
#include <spi.h>

int main(int argc, char* argv[]) {
    ARP_ReturnType arpRet = ARP_E_UNKNOWN_ERROR;
    SPI_ReturnType spiRet = SPI_E_UNKNOWN_ERROR;
    PLCA_Mode_t plcaMode = PLCA_MODE_COORDINATOR;
    bool regInitStatus = false;
    uint8_t argCount;

    spiRet = SPI_Init();
#ifdef DEBUG_MODE
    if (spiRet != SPI_E_SUCCESS) {
        printf("SPI_Init failed; the error code is %d\n", spiRet);
    }
#endif


    if(argc == 1u) {
        printf("No argument provided\n Usage: %s -c|-f|-h\n -c   : Coordinator mode\n -f   : Follower mode\n -h   : Help mode\n", argv[0]);
    }
    // Register initialization based on argument(c, f, h)
    for (argCount = 1u; argCount < argc; argCount++) {
        if (strcmp(argv[argCount], "-c") == 0) {
            printf("Coordinator mode\n");
            plcaMode = PLCA_MODE_COORDINATOR;
            regInitStatus = InitRegister(plcaMode);
        }
        else if (strcmp(argv[argCount], "-f") == 0) {
            printf("Follower mode\n");
            plcaMode = PLCA_MODE_FOLLOWER;
            regInitStatus = InitRegister(plcaMode);
        }
        else if (strcmp(argv[argCount], "-h") == 0) {
            printf("Help mode\n Usage: %s -c|-f|-h\n -c   : Coordinator mode\n -f   : Follower mode\n -h   : Help mode\n", argv[0]);
        }
        else {
            printf("Invalid argument: %s\n Usage: %s -c|-f|-h\n", argv[argCount], argv[0]);
        }
#ifdef DEBUG_MODE
        printf("Register initialization %s\n", regInitStatus ? "successful" : "failed");
#endif
    }

    /*
        arpRet = ArpTest();
        printf("Result of arp_test is %d\n", arpRet);
     */
    spiRet = SPI_Cleanup();
#ifdef DEBUG_MODE
    if (spiRet != SPI_E_SUCCESS) {
        printf("SPI_Cleanup failed; the error code is %d\n", spiRet);
    }
#endif

    return spiRet;
}
