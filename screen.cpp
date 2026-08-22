#include "screen.h"

uint8_t screenReadByte(screen_t* screen, uint16_t address)
{
	if (address >= VRAM_START_ADDRESS && address <= VRAM_END_ADDRESS)
	{
		return screen->vram[address - VRAM_START_ADDRESS];
	}

	if (address == IO_REG_LCDC_Y_POS)
	{
		//temp did lcd emulated

		screen->lcdYPos++;
		if (screen->lcdYPos > 153)
		{
			screen->lcdYPos = 0;
		}
		return screen->lcdYPos;
	}

	return 0;
}

void screenWriteByte(screen_t* screen, uint16_t address, uint8_t value)
{
	if (address >= VRAM_START_ADDRESS && address <= VRAM_END_ADDRESS)
	{
		screen->vram[address - VRAM_START_ADDRESS] = value;
	}
}
