#include "timer.h"
#include "bus.h"

static const uint16_t timaPeriods[4] = { 1024, 16, 64, 256 };

void timerStep(timer_t* timer, hardwareRegisters_t* hardwareRegisters, uint8_t cycles)
{
	timer->divCycleAccumulator += cycles;
	while (timer->divCycleAccumulator >= 256)
	{
		timer->divCycleAccumulator -= 256;
		timer->div++;
	}

	if (timer->tac & 0x04)
	{
		uint16_t period = timaPeriods[timer->tac & 0x03];
		timer->timaCycleAccumulator += cycles;
		while (timer->timaCycleAccumulator >= period)
		{
			timer->timaCycleAccumulator -= period;
			timer->tima++;
			if (timer->tima == 0)
			{
				timer->tima = timer->tma;
				requestInterrupt(hardwareRegisters, INTERRUPT_FLAG_TIMER);
			}
		}
	}
}

uint8_t timerReadByte(timer_t* timer, uint16_t address)
{
	switch (address)
	{
		case IO_REG_DIV: return timer->div;
		case IO_REG_TIMA: return timer->tima;
		case IO_REG_TMA: return timer->tma;
		case IO_REG_TAC: return timer->tac;
	}
	return 0;
}

void timerWriteByte(timer_t* timer, uint16_t address, uint8_t value)
{
	switch (address)
	{
		case IO_REG_DIV: timer->div = 0; break;
		case IO_REG_TIMA: timer->tima = value; break;
		case IO_REG_TMA: timer->tma = value; break;
		case IO_REG_TAC: timer->tac = value; break;
	}
}
