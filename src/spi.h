#ifndef SPI_H
#define SPI_H

// sudo apt install pigpio
#include <pigpio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum {
    SPI_E_SUCCESS = 0,
    SPI_E_UNKNOWN_ERROR,
    SPI_E_INIT_ERROR,
    SPI_E_ISR_INIT_ERROR,
};

enum {
    PLCA_MODE_COORDINATOR = 0,
    PLCA_MODE_FOLLOWER,
    PLCA_MODE_INVALID,
};

int spi_init(void);
int spi_transfer(uint8_t* rxbuffer, uint8_t* txbuffer, uint16_t length);
int spi_cleanup(void);

bool set_register(int mode);
uint32_t read_register(uint8_t MMS, uint16_t Address);
uint32_t write_register(uint8_t MMS, uint16_t Address, uint32_t data);

uint8_t get_parity(uint32_t valueToCalculateParity);

#endif /* SPI_H */
