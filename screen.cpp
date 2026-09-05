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

	if (address == IO_REG_LCDC)
	{
		return screen->lcdc;
	}

	if (address == IO_REG_BGP)
	{
		return screen->bgp;
	}

	return 0;
}

void screenWriteByte(screen_t* screen, uint16_t address, uint8_t value)
{
	if (address >= VRAM_START_ADDRESS && address <= VRAM_END_ADDRESS)
	{
		screen->vram[address - VRAM_START_ADDRESS] = value;
	}

	if (address == IO_REG_LCDC)
	{
		screen->lcdc = value;
	}

	if (address == IO_REG_BGP)
	{
		screen->bgp = value;
	}
}

void screenRenderBackground(screen_t* screen, uint32_t* framebuffer)
{
	uint32_t tilemapAddress = BG_TILE_MAP_0_ADDRESS;
	if ((screen->lcdc & LCDC_BIT_BG_TILE_MAP_SELECT) != 0)
	{
	  tilemapAddress = BG_TILE_MAP_1_ADDRESS;
	}

	bool signedTileDataMode = true;
	if ((screen->lcdc & LCDC_BIT_BG_TILE_DATA_SELECT) != 0)
	{
		signedTileDataMode = false;
	}

	for (int currentRow = 0; currentRow < GB_TILE_MAP_ROWS; currentRow++)
	{
		for (int currentCol = 0; currentCol < GB_TILE_MAP_COLS; currentCol++)
		{
			int32_t vramTileAddress = tilemapAddress + ((currentRow * GB_TILE_MAP_WIDTH) + currentCol) - VRAM_START_ADDRESS;

			int32_t tiledataStart = (signedTileDataMode == true) ? (TILE_DATA_SIGNED_BASE_ADDRESS + ((int8_t)screen->vram[vramTileAddress]) * GB_TILE_BYTES):

			(TILE_DATA_UNSIGNED_BASE_ADDRESS + ((uint8_t)screen->vram[vramTileAddress]) * GB_TILE_BYTES);

			for (int tileXPos = 0; tileXPos < GB_TILE_ROWS; tileXPos++)
			{
				for (int tileYPos = 0; tileYPos < GB_TILE_COLS; tileYPos++)
				{
					uint8_t pixelrow = (tileYPos % 8);
					uint8_t lowByte = screen->vram[(tiledataStart + pixelrow * 2) - VRAM_START_ADDRESS];
					uint8_t highByte = screen->vram[(tiledataStart + pixelrow * 2 + 1) - VRAM_START_ADDRESS];

					uint8_t pixelLowBit = (lowByte >> (7 - tileXPos)) & 1;
					uint8_t pixelHighbit = (highByte >> (7 - tileXPos)) & 1;
					uint8_t colorID = (pixelHighbit << 1) | pixelLowBit;
				}
			}
		}
	}

}
