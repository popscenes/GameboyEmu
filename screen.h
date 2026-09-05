#pragma once
#include <stdint.h>

#define VRAM_START_ADDRESS 0x8000
#define VRAM_END_ADDRESS 0x9FFF

#define IO_REG_LCDC 0xff40
#define IO_REG_LCDC_Y_POS 0xff44
#define IO_REG_BGP 0xff47

#define BG_TILE_MAP_0_ADDRESS 0x9800
#define BG_TILE_MAP_1_ADDRESS 0x9C00
#define TILE_DATA_UNSIGNED_BASE_ADDRESS 0x8000
#define TILE_DATA_SIGNED_BASE_ADDRESS 0x9000

#define LCDC_BIT_BG_TILE_DATA_SELECT 0x10
#define LCDC_BIT_BG_TILE_MAP_SELECT 0x08

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
	uint8_t lcdYPos;
	uint8_t lcdc;
	uint8_t bgp;
} screen_t;

uint8_t screenReadByte(screen_t* screen, uint16_t address);
void screenWriteByte(screen_t* screen, uint16_t address, uint8_t value);

// Decodes the current background tile map + tile data into a 160x144 ARGB8888 framebuffer.
void screenRenderBackground(screen_t* screen, uint32_t* framebuffer);
