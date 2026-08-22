#pragma once
#include <stdint.h>
#include "bus.h"

#define IO_REG_DIV 0xff04
#define IO_REG_TIMA 0xff05
#define IO_REG_TMA 0xff06
#define IO_REG_TAC 0xff07

typedef struct {
	uint8_t div;
	uint8_t tima;
	uint8_t tma;
	uint8_t tac;
	uint16_t divCycleAccumulator;
	uint16_t timaCycleAccumulator;
} timer_t;

void timerStep(timer_t* timer, hardwareRegisters_t* hardwareRegisters, uint8_t cycles);
uint8_t timerReadByte(timer_t* timer, uint16_t address);
void timerWriteByte(timer_t* timer, uint16_t address, uint8_t value);
