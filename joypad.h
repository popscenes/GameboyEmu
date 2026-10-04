#pragma once
#include <stdint.h>

#define JOYP_SELECT_ACTIONS_BIT 0x20
#define JOYP_SELECT_DIRECTIONS_BIT 0x10
#define JOYP_SELECT_MASK 0x30
#define JOYP_BUTTONS_MASK 0x0F
#define JOYP_UNUSED_BITS 0xC0

typedef enum {
	JOYPAD_BUTTON_RIGHT,
	JOYPAD_BUTTON_LEFT,
	JOYPAD_BUTTON_UP,
	JOYPAD_BUTTON_DOWN,
	JOYPAD_BUTTON_A,
	JOYPAD_BUTTON_B,
	JOYPAD_BUTTON_SELECT,
	JOYPAD_BUTTON_START,
} joypadButton_t;

typedef struct {
	uint8_t select;
	// Active-high (1 = pressed) so a zero-initialised gameboy_t starts with nothing held.
	uint8_t pressedDirections;
	uint8_t pressedActions;
} joypad_t;

uint8_t joypadReadByte(joypad_t* joypad);
void joypadWriteByte(joypad_t* joypad, uint8_t value);
void joypadSetButton(joypad_t* joypad, joypadButton_t button, bool pressed);
