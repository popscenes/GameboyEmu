#pragma once
#include<stdint.h>

#define CART_START_ADDRESS 0x0000
#define CART_END_ADDRESS 0x7FFF

#define INTERNAL_RAM_START_ADDRESS 0xC000
#define INTERNAL_RAM_END_ADDRESS 0xDFFF

#define HIGH_RAM_START_ADDRESS 0xFF80
#define HIGH_RAM_END_ADDRESS 0xFFFE

#define IO_REG_DIV 0xff04
#define IO_REG_TIMA 0xff05
#define IO_REG_TMA 0xff06
#define IO_REG_TAC 0xff07
#define IO_REG_INTERRUPT_FLAG 0xff0f
#define IO_REG_LCDC_Y_POS 0xff44
#define IO_REG_INTERRUPT_ENABLE 0xffff

#define INTERRUPT_FLAG_TIMER 0x04

typedef struct {
	uint8_t lcdYPos; //temp did lcd emulated
	uint8_t interruptFlag;
	uint8_t interruptEnable;
	uint8_t div;
	uint8_t tima;
	uint8_t tma;
	uint8_t tac;
} hardwareRegisters_t;

typedef struct {
	uint8_t highRam[126];
	uint8_t internalRam[8192];

} internalMemory_t;

uint16_t readWordFromAddress(uint16_t adderss);
uint8_t readByteFromAddress(uint16_t adderss);

void writeWordToAddress(uint16_t adderss, uint16_t value);
void writeByteToAddress(uint16_t adderss, uint8_t value);

void timerStep(uint8_t cycles);
