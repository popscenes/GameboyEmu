#include "screen.h"

uint8_t screenReadByte(screen_t* screen, uint16_t address)
{
	if (address >= VRAM_START_ADDRESS && address <= VRAM_END_ADDRESS)
	{
		return screen->vram[address - VRAM_START_ADDRESS];
	}
	if (address >= OAM_START_ADDRESS && address <= OAM_END_ADDRESS)
	{
		return screen->oam[address - OAM_START_ADDRESS];
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

	if (address == IO_REG_SCX)
	{
		return screen->scx;
	}

	if (address == IO_REG_SCY)
	{
		return screen->scy;
	}

	if (address == IO_REG_OBP0)
	{
		return screen->obp0;
	}

	if (address == IO_REG_OBP1)
	{
		return screen->obp1;
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
	if (address >= OAM_START_ADDRESS && address <= OAM_END_ADDRESS)
	{
		screen->oam[address - OAM_START_ADDRESS] = value;
	}

	if (address == IO_REG_LCDC)
	{
		screen->lcdc = value;
	}

	if (address == IO_REG_BGP)
	{
		screen->bgp = value;
	}

	if (address == IO_REG_SCX)
	{
		screen->scx = value;
	}

	if (address == IO_REG_SCY)
	{
		screen->scy = value;
	}

	if (address == IO_REG_OBP0)
	{
		screen->obp0 = value;
	}

	if (address == IO_REG_OBP1)
	{
		screen->obp1 = value;
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
		if (screenMode == PPU_MODE_DRAWING)
		{
			screenRenderBackgroundLine(screen, screen->ly);
			screenRenderSpritesLine(screen, screen->ly);
		}

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

uint32_t getPixelColour(uint8_t colorID, uint8_t pallette)
{
	
	uint8_t shade = (pallette >> (colorID * 2)) & 0x3;
	uint8_t gray = (255 - ((shade * 255) / 3));
	uint32_t pixelColour = 0xFF000000 | (gray << 16) | (gray << 8) | gray;

	return pixelColour;
}

void screenRenderBackgroundLine(screen_t* screen, uint8_t line)
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

	
	int32_t bgY = (line + screen->scy) & 255;
	uint8_t pixelrow = (bgY % 8);

	for (int32_t x = 0; x < GB_SCREEN_WIDTH; x++)
	{
		int32_t bgX = (x + screen->scx) & 255;
		
		int32_t vramTileAddress = (tilemapAddress + (bgY / 8) * 32 + (bgX / 8))- VRAM_START_ADDRESS;
		int32_t tiledataStart = (signedTileDataMode == true) ? (TILE_DATA_SIGNED_BASE_ADDRESS + ((int8_t)screen->vram[vramTileAddress]) * GB_TILE_BYTES) :
			(TILE_DATA_UNSIGNED_BASE_ADDRESS + ((uint8_t)screen->vram[vramTileAddress]) * GB_TILE_BYTES);

		
		uint8_t lowByte = screen->vram[(tiledataStart + pixelrow * 2) - VRAM_START_ADDRESS];
		uint8_t highByte = screen->vram[(tiledataStart + pixelrow * 2 + 1) - VRAM_START_ADDRESS];

		uint8_t pixelLowBit = (lowByte >> (7 - (bgX%8))) & 1;
		uint8_t pixelHighbit = (highByte >> (7 - (bgX%8))) & 1;

		uint8_t colorID = (pixelHighbit << 1) | pixelLowBit;
		uint32_t pixelColour = getPixelColour(colorID, screen->bgp);

		screen->framebuffer[line * GB_SCREEN_WIDTH + x] = pixelColour;
	}
}

void screenRenderSpritesLine(screen_t* screen, uint8_t line)
{
	if ((screen->lcdc & LCDC_BIT_OBJ_ENABLE) == 0)
	{
		return;
	}

	for (int32_t i = OAM_SPRITE_COUNT-1; i >= 0; i--)
	{
		uint32_t oamIndex = i * OAM_ENTRY_SIZE;

		uint8_t spriteYPos = screen->oam[oamIndex + OAM_BYTE_Y];
		uint8_t spriteXPos = screen->oam[oamIndex + OAM_BYTE_X];
		uint8_t spriteTile = screen->oam[oamIndex + OAM_BYTE_TILE];
		uint8_t spriteAttributes = screen->oam[oamIndex + OAM_BYTE_ATTRIBUTES];

		int32_t positionX = spriteXPos - SPRITE_X_OFFSET;
		int32_t positionY = spriteYPos - SPRITE_Y_OFFSET;

		bool flipX = false;
		bool flipY = false;

		if ((spriteAttributes & OAM_ATTR_Y_FLIP) != 0)
		{
			flipY = true;
		}

		if ((spriteAttributes & OAM_ATTR_X_FLIP) != 0)
		{
			flipX = true;
		}

		uint32_t tileIndex = spriteTile * GB_TILE_BYTES;

		int32_t tileYPos = line - positionY;
		if (tileYPos < 0 || tileYPos > 7)
		{
			continue;
		}

		int32_t pixelPositionY = positionY + tileYPos;
		for (int tileXPos = 0; tileXPos < GB_TILE_ROWS; tileXPos++)
		{
			int32_t pixelPositionX = positionX + tileXPos;
			

			if (pixelPositionX < 0 || pixelPositionX >= GB_SCREEN_WIDTH)
			{
				continue;
			}

			uint32_t resolvedYRow = flipY ? (7 - tileYPos) : tileYPos;
			uint32_t resolvedXRow = flipX ? (7 - tileXPos) : tileXPos;

			uint8_t pixelrow = (resolvedYRow % 8);
			uint8_t lowByte = screen->vram[(tileIndex + pixelrow * 2)];
			uint8_t highByte = screen->vram[(tileIndex + pixelrow * 2 + 1)];

			uint8_t pixelLowBit = (lowByte >> (7 - resolvedXRow)) & 1;
			uint8_t pixelHighbit = (highByte >> (7 - resolvedXRow)) & 1;

			uint8_t pallette = screen->obp0;
			if ((spriteAttributes & OAM_ATTR_PALETTE) > 0)
			{
				pallette = screen->obp1;
			}

			uint8_t colorID = (pixelHighbit << 1) | pixelLowBit;
			if (colorID == 0)
			{
				continue;
			}
			uint32_t pixelColour = getPixelColour(colorID, pallette);
			screen->framebuffer[(pixelPositionY * GB_SCREEN_WIDTH) + pixelPositionX] = pixelColour;
		}
	}
}
