#include "GameBoy.h"

void gameboyStep(gameboy_t* gb)
{
	cpuStep(gb);
	timerStep(&gb->timer, &gb->hardwareRegisters, cpuCurrentIstructionCycles(gb));
	HandleInterrupts(gb);
}
