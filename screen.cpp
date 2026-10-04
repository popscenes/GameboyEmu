#include "screen.h"

uint8_t screenReadByte(screen_t* screen, uint16_t address)
{
	if (address >= VRAM_START_ADDRESS && address <= VRAM_END_ADDRESS)
	{
		return screen->vram[address - VRAM_START_ADDRESS];
	}

	if (address == IO_REG_LCDC_Y_POS)
	{
		return screen->ly;
	}

	if (address == IO_REG_LCDC)
	{
		return screen->lcdc;
	}

	if (address == IO_REG_BGP)
	{
		return screen->bgp;
	}

	if (address == IO_REG_STAT)
	{
		return screen->stat | STAT_UNUSED_BIT;
	}

	if (address == IO_REG_LYC)
	{
		return screen->lyc;
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

	if (address == IO_REG_STAT)
	{
		screen->stat = (screen->stat & ~STAT_WRITABLE_MASK) | (value & STAT_WRITABLE_MASK);
	}

	if (address == IO_REG_LYC)
	{
		screen->lyc = value;
	}
}

void screenStep(screen_t* screen, hardwareRegisters_t* hardwareRegisters, uint8_t cycles)
{
	if ((LCDC_BIT_LCD_ENABLE & screen->lcdc) == 0)
	{
		screen->scanlineCycles = 0;
		screen->ly = 0;
		screen->stat = screen->stat & ~STAT_MODE_MASK;
		return;
	}

	screen->scanlineCycles += cycles;


	if (screen->scanlineCycles >= CYCLES_PER_SCANLINE)
	{
		screen->scanlineCycles -= CYCLES_PER_SCANLINE;
		screen->ly++;

		if (screen->ly == VBLANK_START_LINE)
		{
			requestInterrupt(hardwareRegisters, INTERRUPT_FLAG_VBLANK);
			screen->frameReady = true;
		}

		else if (screen->ly == SCANLINES_PER_FRAME)
		{
			screen->ly = 0;
		}

		if (screen->ly == screen->lyc)
		{
			screen->stat = screen->stat | STAT_BIT_LYC_EQUAL;
			if (screen->stat & STAT_BIT_LYC_INT)
			{
				requestInterrupt(hardwareRegisters, INTERRUPT_FLAG_STAT);
			}
		}
		else
		{
			screen->stat = screen->stat & ~STAT_BIT_LYC_EQUAL;
		}
	}

	uint8_t screenMode = PPU_MODE_HBLANK;

	if (screen->ly >= VBLANK_START_LINE)
	{
		screenMode = PPU_MODE_VBLANK;
	}
	else if (screen->scanlineCycles < OAM_SCAN_END_CYCLE)
	{
		screenMode = PPU_MODE_OAM_SCAN;
	}
	else if (screen->scanlineCycles < DRAWING_END_CYCLE)
	{
		screenMode = PPU_MODE_DRAWING;
	}

	uint8_t currentMode = screen->stat & STAT_MODE_MASK;
	screen->stat = (screen->stat & ~STAT_MODE_MASK) | screenMode;

	if (screenMode != currentMode)
	{
		if (screenMode == PPU_MODE_HBLANK && (screen->stat & STAT_BIT_HBLANK_INT))
		{
			requestInterrupt(hardwareRegisters, INTERRUPT_FLAG_STAT);
		}

		if (screenMode == PPU_MODE_VBLANK && (screen->stat & STAT_BIT_VBLANK_INT))
		{
			requestInterrupt(hardwareRegisters, INTERRUPT_FLAG_STAT);
		}

		if (screenMode == PPU_MODE_OAM_SCAN && (screen->stat & STAT_BIT_OAM_INT))
		{
			requestInterrupt(hardwareRegisters, INTERRUPT_FLAG_STAT);
		}

		
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

					uint8_t shade = (screen->bgp >> (colorID * 2)) & 0x3;
					uint8_t gray = (255 - ((shade * 255) / 3));
					uint32_t pixelColour = 0xFF000000 | (gray << 16) | (gray << 8) | gray;

					uint32_t screenXPos = (currentCol * GB_TILE_COLS) + tileXPos;
					uint32_t screenYPos = (currentRow * GB_TILE_ROWS) + tileYPos;

					framebuffer[(screenYPos * GB_SCREEN_WIDTH) + screenXPos] = pixelColour;


				}
			}
		}
	}

}
