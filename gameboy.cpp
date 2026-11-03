#include "GameBoy.h"

void gameboyInit(gameboy_t* gb)
{
	cpuInit(gb);

	gb->screen.lcdc = 0x91;
	gb->screen.bgp = 0xFC;
	gb->screen.stat = 0x85;
	gb->hardwareRegisters.interruptFlag = 0xE1;
}

void gameboyStep(gameboy_t* gb)
{
	cpuStep(gb);
	timerStep(&gb->timer, &gb->hardwareRegisters, cpuCurrentIstructionCycles(gb));
	screenStep(&gb->screen, &gb->hardwareRegisters, cpuCurrentIstructionCycles(gb));
	HandleInterrupts(gb);
}
