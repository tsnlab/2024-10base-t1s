#ifndef SPI_H
#define SPI_H

// sudo apt install pigpio
#include <pigpio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef enum {
    SPI_E_SUCCESS = 0,
    SPI_E_UNKNOWN_ERROR,
    SPI_E_INIT_ERROR,
    SPI_E_ISR_INIT_ERROR,
} SPI_ReturnType;

SPI_ReturnType SPI_Init(void);
SPI_ReturnType SPI_Transfer(uint8_t* rxBuffer, uint8_t* txBuffer, uint16_t length);
SPI_ReturnType SPI_Cleanup(void);

typedef enum {
    PLCA_MODE_COORDINATOR = 0,
    PLCA_MODE_FOLLOWER = 1,
} PLCA_Mode_t;

uint32_t ReadRegister(uint8_t MMS, uint16_t Address);
uint32_t WriteRegister(uint8_t MMS, uint16_t Address, uint32_t data);

bool GetParity(uint32_t valueToCalculateParity);
void ConvertEndianness(uint32_t valueToConvert, uint32_t* convertedValue);

#endif /* SPI_H */
