#include "bus.h"
#include "cart.h"

internalMemory_t internalMemory = { 0 };
hardwareRegisters_t hardwareRegisters = { 0 };

static uint16_t divCycleAccumulator = 0;
static uint16_t timaCycleAccumulator = 0;
static const uint16_t timaPeriods[4] = { 1024, 16, 64, 256 };

void timerStep(uint8_t cycles)
{
	divCycleAccumulator += cycles;
	while (divCycleAccumulator >= 256)
	{
		divCycleAccumulator -= 256;
		hardwareRegisters.div++;
	}

	if (hardwareRegisters.tac & 0x04)
	{
		uint16_t period = timaPeriods[hardwareRegisters.tac & 0x03];
		timaCycleAccumulator += cycles;
		while (timaCycleAccumulator >= period)
		{
			timaCycleAccumulator -= period;
			hardwareRegisters.tima++;
			if (hardwareRegisters.tima == 0)
			{
				hardwareRegisters.tima = hardwareRegisters.tma;
				hardwareRegisters.interruptFlag |= INTERRUPT_FLAG_TIMER;
			}
		}
	}
}

uint16_t readWordFromAddress(uint16_t adderss) {
	
	uint8_t loByte = 0;
	uint8_t hiByte = 0;
	loByte = readByteFromAddress(adderss);
	hiByte = readByteFromAddress(adderss+1);
	uint16_t value = (hiByte << 8) | (loByte);
	return value;
}

uint8_t readByteFromAddress(uint16_t adderss)
{
	uint8_t byte = 0;
	if (adderss <= CART_END_ADDRESS)
	{
		byte = readyByteFromCart(adderss);
	}
	else if (adderss == IO_REG_DIV)
	{
		return hardwareRegisters.div;
	}
	else if (adderss == IO_REG_TIMA)
	{
		return hardwareRegisters.tima;
	}
	else if (adderss == IO_REG_TMA)
	{
		return hardwareRegisters.tma;
	}
	else if (adderss == IO_REG_TAC)
	{
		return hardwareRegisters.tac;
	}
	else if (adderss == IO_REG_INTERRUPT_FLAG)
	{
		return hardwareRegisters.interruptFlag;
	}
	else if (adderss == IO_REG_LCDC_Y_POS)
	{
		//temp did lcd emulated


		hardwareRegisters.lcdYPos++;
		if (hardwareRegisters.lcdYPos > 153)
		{
			hardwareRegisters.lcdYPos = 0;
		}
		return hardwareRegisters.lcdYPos;
	}
	else if (adderss >= INTERNAL_RAM_START_ADDRESS && adderss <= INTERNAL_RAM_END_ADDRESS)
	{
		adderss -= INTERNAL_RAM_START_ADDRESS;
		byte = internalMemory.internalRam[adderss];
	}
	else if (adderss >= HIGH_RAM_START_ADDRESS && adderss <= HIGH_RAM_END_ADDRESS)
	{
		adderss -= HIGH_RAM_START_ADDRESS;
		byte = internalMemory.highRam[adderss];
	}
	else if (adderss == IO_REG_INTERRUPT_ENABLE)
	{
		return hardwareRegisters.interruptEnable;
	}
	else
	{
		printf(" [ADRESS NOT IMPLEMENTED] ");
	}

	return byte;
}

void writeWordToAddress(uint16_t adderss, uint16_t value)
{
	uint8_t loByte = value & 0x00ff;
	uint8_t hiByte = (value >> 8) & 0x00ff;
	writeByteToAddress(adderss, loByte);
	writeByteToAddress(adderss+1, hiByte);
	
}

void writeByteToAddress(uint16_t adderss, uint8_t value)
{
	uint8_t byte = 0;
	if (adderss <= CART_END_ADDRESS)
	{
	}
	else if (adderss >= INTERNAL_RAM_START_ADDRESS && adderss <= INTERNAL_RAM_END_ADDRESS)
	{

		adderss -= INTERNAL_RAM_START_ADDRESS;
		internalMemory.internalRam[adderss] = value;
	}
	else if (adderss == IO_REG_DIV)
	{
		hardwareRegisters.div = 0;
	}
	else if (adderss == IO_REG_TIMA)
	{
		hardwareRegisters.tima = value;
	}
	else if (adderss == IO_REG_TMA)
	{
		hardwareRegisters.tma = value;
	}
	else if (adderss == IO_REG_TAC)
	{
		hardwareRegisters.tac = value;
	}
	else if (adderss == IO_REG_INTERRUPT_FLAG)
	{
		hardwareRegisters.interruptFlag = value;
	}
	else if (adderss >= HIGH_RAM_START_ADDRESS && adderss <= HIGH_RAM_END_ADDRESS)
	{
		
		adderss -= HIGH_RAM_START_ADDRESS;
		internalMemory.highRam[adderss] = value;
	}
	else if (adderss == IO_REG_INTERRUPT_ENABLE)
	{
		hardwareRegisters.interruptEnable = value;
	}
	else
	{
		printf(" [ADRESS NOT IMPLEMENTED] ");
	}
}