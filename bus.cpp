#include "bus.h"
#include "cart.h"
#include "timer.h"
#include "screen.h"
#include "GameBoy.h"

void requestInterrupt(hardwareRegisters_t* hardwareRegisters, uint8_t flagBit)
{
	hardwareRegisters->interruptFlag |= flagBit;
}

uint16_t readWordFromAddress(gameboy_t* gb, uint16_t adderss) {

	uint8_t loByte = 0;
	uint8_t hiByte = 0;
	loByte = readByteFromAddress(gb, adderss);
	hiByte = readByteFromAddress(gb, adderss+1);
	uint16_t value = (hiByte << 8) | (loByte);
	return value;
}

uint8_t readByteFromAddress(gameboy_t* gb, uint16_t adderss)
{
	uint8_t byte = 0;
	if (adderss <= CART_END_ADDRESS)
	{
		byte = readyByteFromCart(&gb->cart, adderss);
	}
	else if ((adderss >= VRAM_START_ADDRESS && adderss <= VRAM_END_ADDRESS) || adderss == IO_REG_LCDC_Y_POS || adderss == IO_REG_LCDC || adderss == IO_REG_BGP)
	{
		return screenReadByte(&gb->screen, adderss);
	}
	else if (adderss >= IO_REG_DIV && adderss <= IO_REG_TAC)
	{
		return timerReadByte(&gb->timer, adderss);
	}
	else if (adderss == IO_REG_INTERRUPT_FLAG)
	{
		return gb->hardwareRegisters.interruptFlag;
	}
	else if (adderss >= INTERNAL_RAM_START_ADDRESS && adderss <= INTERNAL_RAM_END_ADDRESS)
	{
		adderss -= INTERNAL_RAM_START_ADDRESS;
		byte = gb->memory.internalRam[adderss];
	}
	else if (adderss >= HIGH_RAM_START_ADDRESS && adderss <= HIGH_RAM_END_ADDRESS)
	{
		adderss -= HIGH_RAM_START_ADDRESS;
		byte = gb->memory.highRam[adderss];
	}
	else if (adderss == IO_REG_INTERRUPT_ENABLE)
	{
		return gb->hardwareRegisters.interruptEnable;
	}
	else
	{
		printf(" [ADRESS NOT IMPLEMENTED] ");
	}

	return byte;
}

void writeWordToAddress(gameboy_t* gb, uint16_t adderss, uint16_t value)
{
	uint8_t loByte = value & 0x00ff;
	uint8_t hiByte = (value >> 8) & 0x00ff;
	writeByteToAddress(gb, adderss, loByte);
	writeByteToAddress(gb, adderss+1, hiByte);

}

void writeByteToAddress(gameboy_t* gb, uint16_t adderss, uint8_t value)
{
	uint8_t byte = 0;
	if (adderss <= CART_END_ADDRESS)
	{
	}
	else if ((adderss >= VRAM_START_ADDRESS && adderss <= VRAM_END_ADDRESS) || adderss == IO_REG_LCDC_Y_POS || adderss == IO_REG_LCDC || adderss == IO_REG_BGP)
	{
		screenWriteByte(&gb->screen, adderss, value);
	}
	else if (adderss >= INTERNAL_RAM_START_ADDRESS && adderss <= INTERNAL_RAM_END_ADDRESS)
	{

		adderss -= INTERNAL_RAM_START_ADDRESS;
		gb->memory.internalRam[adderss] = value;
	}
	else if (adderss >= IO_REG_DIV && adderss <= IO_REG_TAC)
	{
		timerWriteByte(&gb->timer, adderss, value);
	}
	else if (adderss == IO_REG_INTERRUPT_FLAG)
	{
		gb->hardwareRegisters.interruptFlag = value;
	}
	else if (adderss >= HIGH_RAM_START_ADDRESS && adderss <= HIGH_RAM_END_ADDRESS)
	{

		adderss -= HIGH_RAM_START_ADDRESS;
		gb->memory.highRam[adderss] = value;
	}
	else if (adderss == IO_REG_INTERRUPT_ENABLE)
	{
		gb->hardwareRegisters.interruptEnable = value;
	}
	else
	{
		printf(" [ADRESS NOT IMPLEMENTED] ");
	}
}
