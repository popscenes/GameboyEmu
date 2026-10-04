#include "joypad.h"

uint8_t joypadReadByte(joypad_t* joypad)
{
	if ((joypad->select & JOYP_SELECT_DIRECTIONS_BIT) == 0)
	{
		return joypad->select | (JOYP_BUTTONS_MASK & ~joypad->pressedDirections) | JOYP_UNUSED_BITS;
	}

	if ((joypad->select & JOYP_SELECT_ACTIONS_BIT) == 0)
	{
		return joypad->select | (JOYP_BUTTONS_MASK & ~joypad->pressedActions) | JOYP_UNUSED_BITS;
	}

	return joypad->select | (JOYP_BUTTONS_MASK) | JOYP_UNUSED_BITS;
}

void joypadWriteByte(joypad_t* joypad, uint8_t value)
{
	joypad->select = (value & JOYP_SELECT_MASK);
}

void joypadSetButton(joypad_t* joypad, joypadButton_t button, bool pressed)
{
	uint8_t buttonValue = 1 << (button & 3);

	if (button >= JOYPAD_BUTTON_A)
	{
		if (pressed)
		{
			joypad->pressedActions = joypad->pressedActions | buttonValue;
		}
		else
		{
			joypad->pressedActions = joypad->pressedActions & ~buttonValue;
		}
		
	}
	else
	{
		if (pressed)
		{
			joypad->pressedDirections = joypad->pressedDirections | buttonValue;
		}
		else
		{
			joypad->pressedDirections = joypad->pressedDirections & ~buttonValue;
		}
	}

	
}
