#pragma once
#include <stdint.h>

#define VRAM_START_ADDRESS 0x8000
#define VRAM_END_ADDRESS 0x9FFF

#define IO_REG_LCDC_Y_POS 0xff44

typedef struct {
	uint8_t vram[8192];
	uint8_t lcdYPos;
} screen_t;

uint8_t screenReadByte(screen_t* screen, uint16_t address);
void screenWriteByte(screen_t* screen, uint16_t address, uint8_t value);
