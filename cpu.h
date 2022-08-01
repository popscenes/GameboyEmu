#pragma once
#include <stdint.h>
typedef struct {
	uint8_t a;
	uint8_t f;
	uint8_t b;
	uint8_t c;
	uint8_t d;
	uint8_t e;
	struct {
		uint8_t h;
		uint8_t l;
	};
	uint16_t hl;
	uint16_t sp;
	uint16_t pc;
	
	uint8_t currentIstructionOpCode;
	uint8_t currentIstructionCycles;
	uint64_t totalCycles;

	char currentInstruction[];

} cpu_t;

#define FLAG_ZERO 0b10000000
#define FLAG_SUBTRACT 0b01000000
#define FLAG_HALF_CARRY 0b00100000
#define FLAG_CARRY 0b00010000

void cpuInit();
void cpuStep();

uint8_t cpuCurrentIstructionCycles();
