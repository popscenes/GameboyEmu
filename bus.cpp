#include "bus.h"
#include "cart.h"

internalMemory_t internalMemory = { 0 };
hardwareRegisters_t hardwareRegisters = { 0 };

uint16_t readWordFromAddress(uint16_t adderss) {
	
	uint8_t loByte = 0;
	uint8_t hiByte = 0;

	if (adderss <= CART_END_ADDRESS)
	{
		loByte = readyByteFromCart(adderss);
		hiByte = readyByteFromCart(adderss + 1);
	}
	else if (adderss == IO_REG_INTERRUPT_FLAG)
	{
		return hardwareRegisters.interruptFlag;
	}
	else if (adderss >= INTERNAL_RAM_START_ADDRESS && adderss <= INTERNAL_RAM_END_ADDRESS)
	{
		adderss -= INTERNAL_RAM_START_ADDRESS;

		loByte = internalMemory.internalRam[adderss];
		hiByte = internalMemory.internalRam[adderss+1];
	}
	else if (adderss >= HIGH_RAM_START_ADDRESS && adderss <= HIGH_RAM_END_ADDRESS)
	{
		adderss -= HIGH_RAM_START_ADDRESS;

		loByte = internalMemory.highRam[adderss];
		hiByte = internalMemory.highRam[adderss + 1];
	}
	else if (adderss == IO_REG_INTERRUPT_ENABLE)
	{
		return hardwareRegisters.interruptFlag;
	}
	else
	{
		//printf(" [ADRESS NOT IMPLEMENTED] ");
	}

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
		//printf(" [ADRESS NOT IMPLEMENTED] ");
	}

	return byte;
}

void writeWordToAddress(uint16_t adderss, uint16_t value)
{
	//printf(" [ADRESS NOT IMPLEMENTED] ");
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
		//printf(" [ADRESS NOT IMPLEMENTED] ");
	}
}