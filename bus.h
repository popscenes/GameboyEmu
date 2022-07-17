#pragma once
#include<stdint.h>

#define CART_START_ADDRESS 0x0000
#define CART_END_ADDRESS 0x7FFF

#define HIGH_RAM_START_ADDRESS 0xFF80
#define HIGH_RAM_END_ADDRESS 0xFFFE

typedef struct {
	uint8_t highRam[126];
} internalMemory_t;

uint16_t readWordFromAddress(uint16_t adderss);
uint8_t readByteFromAddress(uint16_t adderss);

void writeWordToAddress(uint16_t adderss, uint16_t value);
void writeByteToAddress(uint16_t adderss, uint8_t value);
