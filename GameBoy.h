#pragma once
#define CLOCK_TICKS_PER_SECOND 4194304
//#define CLOCK_TICKS_PER_MICROSECOND CLOCK_TICKS_PER_SECOND/1000000
#define SECONDS_PER_TICK 1/CLOCK_TICKS_PER_SECOND
#define NANOSECONDS_PER_TICK 238

#include "cpu.h"
#include "bus.h"
#include "timer.h"
#include "screen.h"
#include "Cart.h"

typedef struct gameboy_s {
	cpu_t cpu;
	internalMemory_t memory;
	hardwareRegisters_t hardwareRegisters;
	timer_t timer;
	screen_t screen;
	cart_t cart;
} gameboy_t;
