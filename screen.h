#pragma once
#include <stdint.h>
#include "bus.h"

#define VRAM_START_ADDRESS 0x8000
#define VRAM_END_ADDRESS 0x9FFF

#define IO_REG_LCDC 0xff40
#define IO_REG_STAT 0xff41
#define IO_REG_SCY 0xff42
#define IO_REG_SCX 0xff43
#define IO_REG_LCDC_Y_POS 0xff44
#define IO_REG_LYC 0xff45
#define IO_REG_BGP 0xff47
#define IO_REG_OBP0 0xff48
#define IO_REG_OBP1 0xff49
#define IO_REG_WY 0xff4a
#define IO_REG_WX 0xff4b

#define WINDOW_X_OFFSET 7

#define OAM_START_ADDRESS 0xFE00
#define OAM_END_ADDRESS 0xFE9F
#define OAM_SIZE 160

#define BG_TILE_MAP_0_ADDRESS 0x9800
#define BG_TILE_MAP_1_ADDRESS 0x9C00
#define TILE_DATA_UNSIGNED_BASE_ADDRESS 0x8000
#define TILE_DATA_SIGNED_BASE_ADDRESS 0x9000

#define LCDC_BIT_LCD_ENABLE 0x80
#define LCDC_BIT_WINDOW_TILE_MAP_SELECT 0x40
#define LCDC_BIT_WINDOW_ENABLE 0x20
#define LCDC_BIT_BG_TILE_DATA_SELECT 0x10
#define LCDC_BIT_BG_TILE_MAP_SELECT 0x08
#define LCDC_BIT_OBJ_SIZE 0x04
#define LCDC_BIT_OBJ_ENABLE 0x02

#define OAM_SPRITE_COUNT 40
#define OAM_ENTRY_SIZE 4
#define OAM_BYTE_Y 0
#define OAM_BYTE_X 1
#define OAM_BYTE_TILE 2
#define OAM_BYTE_ATTRIBUTES 3

#define SPRITE_Y_OFFSET 16
#define SPRITE_X_OFFSET 8
#define MAX_SPRITES_PER_LINE 10

#define OAM_ATTR_BG_PRIORITY 0x80
#define OAM_ATTR_Y_FLIP 0x40
#define OAM_ATTR_X_FLIP 0x20
#define OAM_ATTR_PALETTE 0x10

#define CYCLES_PER_SCANLINE 456
#define VBLANK_START_LINE 144
#define SCANLINES_PER_FRAME 154

#define PPU_MODE_HBLANK 0
#define PPU_MODE_VBLANK 1
#define PPU_MODE_OAM_SCAN 2
#define PPU_MODE_DRAWING 3

#define OAM_SCAN_END_CYCLE 80
#define DRAWING_END_CYCLE 252

#define STAT_MODE_MASK 0x03
#define STAT_BIT_LYC_EQUAL 0x04
#define STAT_BIT_HBLANK_INT 0x08
#define STAT_BIT_VBLANK_INT 0x10
#define STAT_BIT_OAM_INT 0x20
#define STAT_BIT_LYC_INT 0x40
#define STAT_WRITABLE_MASK 0x78
#define STAT_UNUSED_BIT 0x80

#define GB_SCREEN_WIDTH 160
#define GB_SCREEN_HEIGHT 144

#define GB_TILE_MAP_ROWS 18
#define GB_TILE_MAP_COLS 20

#define GB_TILE_MAP_WIDTH 32
#define GB_TILE_BYTES 16

#define GB_TILE_ROWS 8
#define GB_TILE_COLS 8

typedef struct {
	uint8_t vram[8192];
	uint8_t oam[OAM_SIZE];
	uint8_t ly;
	uint8_t lcdc;
	uint8_t stat;
	uint8_t scy;
	uint8_t scx;
	uint8_t lyc;
	uint8_t bgp;
	uint8_t obp0;
	uint8_t obp1;
	uint8_t wy;
	uint8_t wx;
	uint8_t windowLineCounter;
	uint16_t scanlineCycles;
	uint32_t lcdOffCycles;
	bool frameReady;
	uint32_t framebuffer[GB_SCREEN_WIDTH * GB_SCREEN_HEIGHT];
	uint8_t bgColorIds[GB_SCREEN_WIDTH];
} screen_t;

uint8_t screenReadByte(screen_t* screen, uint16_t address);
void screenWriteByte(screen_t* screen, uint16_t address, uint8_t value);
void screenStep(screen_t* screen, hardwareRegisters_t* hardwareRegisters, uint8_t cycles);
void screenRenderBackgroundLine(screen_t* screen, uint8_t line);
void screenRenderSpritesLine(screen_t* screen, uint8_t line);
uint32_t getPixelColour(uint8_t colorID, uint8_t pallette);
