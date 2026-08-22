#pragma once
#include<stdint.h>

#define CART_START_ADDRESS 0x0000
#define CART_END_ADDRESS 0x7FFF

#define INTERNAL_RAM_START_ADDRESS 0xC000
#define INTERNAL_RAM_END_ADDRESS 0xDFFF

#define HIGH_RAM_START_ADDRESS 0xFF80
#define HIGH_RAM_END_ADDRESS 0xFFFE

#define IO_REG_INTERRUPT_FLAG 0xff0f
#define IO_REG_INTERRUPT_ENABLE 0xffff

#define INTERRUPT_FLAG_TIMER 0x04

typedef struct {
	uint8_t interruptFlag;
	uint8_t interruptEnable;
} hardwareRegisters_t;

typedef struct {
	uint8_t highRam[126];
	uint8_t internalRam[8192];

} internalMemory_t;

typedef struct gameboy_s gameboy_t;

uint16_t readWordFromAddress(gameboy_t* gb, uint16_t adderss);
uint8_t readByteFromAddress(gameboy_t* gb, uint16_t adderss);

void writeWordToAddress(gameboy_t* gb, uint16_t adderss, uint16_t value);
void writeByteToAddress(gameboy_t* gb, uint16_t adderss, uint8_t value);

void requestInterrupt(hardwareRegisters_t* hardwareRegisters, uint8_t flagBit);
