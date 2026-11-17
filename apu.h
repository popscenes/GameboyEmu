#pragma once
#include <stdint.h>

#define IO_REG_NR10 0xff10
#define IO_REG_NR11 0xff11
#define IO_REG_NR12 0xff12
#define IO_REG_NR13 0xff13
#define IO_REG_NR14 0xff14
#define IO_REG_NR21 0xff16
#define IO_REG_NR22 0xff17
#define IO_REG_NR23 0xff18
#define IO_REG_NR24 0xff19
#define IO_REG_NR30 0xff1a
#define IO_REG_NR31 0xff1b
#define IO_REG_NR32 0xff1c
#define IO_REG_NR33 0xff1d
#define IO_REG_NR34 0xff1e
#define IO_REG_NR41 0xff20
#define IO_REG_NR42 0xff21
#define IO_REG_NR43 0xff22
#define IO_REG_NR44 0xff23
#define IO_REG_NR50 0xff24
#define IO_REG_NR51 0xff25
#define IO_REG_NR52 0xff26

#define APU_REGISTERS_START_ADDRESS 0xff10
#define APU_REGISTERS_END_ADDRESS 0xff26
#define APU_REGISTER_COUNT (APU_REGISTERS_END_ADDRESS - APU_REGISTERS_START_ADDRESS + 1)
#define WAVE_RAM_START_ADDRESS 0xff30
#define WAVE_RAM_END_ADDRESS 0xff3f
#define WAVE_RAM_SIZE 16
#define APU_END_ADDRESS 0xff3f

#define APU_REG(apu, address) ((apu)->registers[(address) - APU_REGISTERS_START_ADDRESS])

#define NR52_BIT_POWER 0x80
#define NR52_BIT_CH1_ON 0x01
#define NR52_BIT_CH2_ON 0x02
#define NR52_BIT_CH3_ON 0x04
#define NR52_BIT_CH4_ON 0x08

#define NRX1_DUTY_MASK 0xC0
#define NRX1_DUTY_SHIFT 6
#define NRX1_LENGTH_MASK 0x3F

#define NRX2_VOLUME_MASK 0xF0
#define NRX2_VOLUME_SHIFT 4
#define NRX2_ENVELOPE_INCREASE_BIT 0x08
#define NRX2_ENVELOPE_PERIOD_MASK 0x07
#define NRX2_DAC_MASK 0xF8

#define NRX4_TRIGGER_BIT 0x80
#define NRX4_LENGTH_ENABLE_BIT 0x40
#define NRX4_FREQUENCY_HIGH_MASK 0x07

#define DUTY_PATTERN_COUNT 4
#define DUTY_PATTERN_STEPS 8
#define FREQUENCY_TIMER_BASE 2048
#define PULSE_FREQUENCY_TIMER_MULTIPLIER 4
#define CHANNEL_MAX_VOLUME 15

#define APU_CLOCK_HZ 4194304
#define APU_SAMPLE_RATE 44100
#define APU_SAMPLE_BUFFER_SIZE 4096
#define APU_OUTPUT_GAIN 0.15f

typedef struct {
	bool enabled;
	uint8_t volume;
	uint8_t dutyStep;
	uint16_t frequencyTimer;
} pulseChannel_t;

typedef struct {
	uint8_t registers[APU_REGISTER_COUNT];
	uint8_t waveRam[WAVE_RAM_SIZE];
	pulseChannel_t channel2;
	uint32_t sampleCycleAccumulator;
	float samples[APU_SAMPLE_BUFFER_SIZE];
	uint32_t sampleCount;
} apu_t;

void apuInit(apu_t* apu);
void apuStep(apu_t* apu, uint8_t cycles);
uint8_t apuReadByte(apu_t* apu, uint16_t address);
void apuWriteByte(apu_t* apu, uint16_t address, uint8_t value);
