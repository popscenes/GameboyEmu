#include "apu.h"

static void apuTriggerChannel2(apu_t* apu);
static void apuStepChannel2(apu_t* apu, uint8_t cycles);
static uint8_t apuChannel2Output(apu_t* apu);
static float apuMixSample(apu_t* apu);
static void apuStepFrameSequencer(apu_t* apu, uint8_t cycles);
static void apuClockChannel2Length(apu_t* apu);
static void apuClockChannel2Envelope(apu_t* apu);

// Bits that always read back as 1 (write-only or unused bits), indexed from NR10
static const uint8_t apuRegisterReadMasks[APU_REGISTER_COUNT] = {
	0x80, 0x3F, 0x00, 0xFF, 0xBF, // NR10-NR14
	0xFF, 0x3F, 0x00, 0xFF, 0xBF, // unused, NR21-NR24
	0x7F, 0xFF, 0x9F, 0xFF, 0xBF, // NR30-NR34
	0xFF, 0xFF, 0x00, 0x00, 0xBF, // unused, NR41-NR44
	0x00, 0x00, 0x70,             // NR50-NR52
};

// 1 = high, 0 = low, for 12.5%, 25%, 50% and 75% duty
static const uint8_t dutyPatterns[DUTY_PATTERN_COUNT][DUTY_PATTERN_STEPS] = {
	{ 0, 0, 0, 0, 0, 0, 0, 1 },
	{ 1, 0, 0, 0, 0, 0, 0, 1 },
	{ 1, 0, 0, 0, 0, 1, 1, 1 },
	{ 0, 1, 1, 1, 1, 1, 1, 0 },
};

void apuInit(apu_t* apu)
{
	APU_REG(apu, IO_REG_NR50) = 0x77;
	APU_REG(apu, IO_REG_NR51) = 0xF3;
	APU_REG(apu, IO_REG_NR52) = NR52_BIT_POWER;
}

void apuStep(apu_t* apu, uint8_t cycles)
{
	apuStepFrameSequencer(apu, cycles);
	apuStepChannel2(apu, cycles);

	apu->sampleCycleAccumulator += cycles * APU_SAMPLE_RATE;
	while (apu->sampleCycleAccumulator >= APU_CLOCK_HZ)
	{
		apu->sampleCycleAccumulator -= APU_CLOCK_HZ;
		if (apu->sampleCount < APU_SAMPLE_BUFFER_SIZE)
		{
			apu->samples[apu->sampleCount] = apuMixSample(apu) * APU_OUTPUT_GAIN;
			apu->sampleCount++;
		}
	}
}

uint8_t apuReadByte(apu_t* apu, uint16_t address)
{
	if (address >= WAVE_RAM_START_ADDRESS && address <= WAVE_RAM_END_ADDRESS)
	{
		return apu->waveRam[address - WAVE_RAM_START_ADDRESS];
	}

	if (address > APU_REGISTERS_END_ADDRESS)
	{
		return 0xFF;
	}

	uint8_t readMask = apuRegisterReadMasks[address - APU_REGISTERS_START_ADDRESS];

	if (address == IO_REG_NR52)
	{
		uint8_t status = APU_REG(apu, IO_REG_NR52) & NR52_BIT_POWER;
		if (apu->channel2.enabled)
		{
			status |= NR52_BIT_CH2_ON;
		}
		return status | readMask;
	}

	return APU_REG(apu, address) | readMask;
}

void apuWriteByte(apu_t* apu, uint16_t address, uint8_t value)
{
	if (address >= WAVE_RAM_START_ADDRESS && address <= WAVE_RAM_END_ADDRESS)
	{
		apu->waveRam[address - WAVE_RAM_START_ADDRESS] = value;
		return;
	}

	if (address > APU_REGISTERS_END_ADDRESS)
	{
		return;
	}

	APU_REG(apu, address) = value;

	if (address == IO_REG_NR21)
	{
		// TODO (you): reload channel2.lengthCounter from the low 6 bits of value
		apu->channel2.lengthCounter = PULSE_LENGTH_MAX - (value & NRX1_LENGTH_MASK);
	}

	if (address == IO_REG_NR22)
	{
		// TODO (you): if the DAC bits (NRX2_DAC_MASK) are all zero, disable channel 2
		if ((value & NRX2_DAC_MASK) == 0)
		{
			apu->channel2.enabled = false;
		}
	}

	if (address == IO_REG_NR24 && (value & NRX4_TRIGGER_BIT))
	{
		apuTriggerChannel2(apu);
	}
}

static uint16_t apuChannel2Period(apu_t* apu)
{
	uint8_t nr23 = APU_REG(apu, IO_REG_NR23);
	uint8_t nr24 = APU_REG(apu, IO_REG_NR24);
	uint16_t frequency = (nr24 & NRX4_FREQUENCY_HIGH_MASK) << 8 | nr23;
	return (FREQUENCY_TIMER_BASE - frequency) * PULSE_FREQUENCY_TIMER_MULTIPLIER;
}

static void apuTriggerChannel2(apu_t* apu)
{
	uint8_t nr22 = APU_REG(apu, IO_REG_NR22);

	apu->channel2.enabled = true;
	if ((nr22 & NRX2_DAC_MASK) == 0)
	{
		apu->channel2.enabled = false;
	}
	apu->channel2.volume = (nr22 & NRX2_VOLUME_MASK) >> NRX2_VOLUME_SHIFT;
	apu->channel2.frequencyTimer = apuChannel2Period(apu);
	if (apu->channel2.lengthCounter == 0)
	{
		apu->channel2.lengthCounter = PULSE_LENGTH_MAX;
	}

	apu->channel2.envelopeTimer = (nr22 & NRX2_ENVELOPE_PERIOD_MASK);
}

static void apuStepChannel2(apu_t* apu, uint8_t cycles)
{
	uint8_t remainingCycles = cycles;
	while (remainingCycles > 0)
	{

		if (apu->channel2.frequencyTimer <= remainingCycles)
		{
			remainingCycles -= apu->channel2.frequencyTimer;
			apu->channel2.frequencyTimer = apuChannel2Period(apu);
			apu->channel2.dutyStep++;
			if (apu->channel2.dutyStep > 7)
			{
				apu->channel2.dutyStep = 0;
			}
		}
		else
		{
			apu->channel2.frequencyTimer -= remainingCycles;
			remainingCycles = 0;
		}
	}
}

static uint8_t apuChannel2Output(apu_t* apu)
{
	if (!apu->channel2.enabled)
	{
		return 0;
	}

	uint8_t nr21 = APU_REG(apu, IO_REG_NR21);
	uint8_t duty = (nr21 & NRX1_DUTY_MASK) >> NRX1_DUTY_SHIFT;

	if (dutyPatterns[duty][apu->channel2.dutyStep] == 1)
	{
		return apu->channel2.volume;
	}

	return 0;
}

static float apuMixSample(apu_t* apu)
{
	
	return apuChannel2Output(apu) / (float)CHANNEL_MAX_VOLUME;
}

// Called every instruction. Every FRAME_SEQUENCER_PERIOD cycles, advance
// frameSequencerStep (0-7, wrapping) and clock length on even steps,
// envelope on step 7.
static void apuStepFrameSequencer(apu_t* apu, uint8_t cycles)
{
	uint8_t remainCycles = cycles;
	
	if (apu->frameSequencerCycles + remainCycles < FRAME_SEQUENCER_PERIOD)
	{
		apu->frameSequencerCycles += remainCycles;
		return;
	}

	apu->frameSequencerCycles = (apu->frameSequencerCycles + remainCycles) - FRAME_SEQUENCER_PERIOD;

	if (apu->frameSequencerStep % 2 == 0)
	{
		apuClockChannel2Length(apu);
	}
	if (apu->frameSequencerStep == 7)
	{
		apuClockChannel2Envelope(apu);
	}

	apu->frameSequencerStep++;
	if (apu->frameSequencerStep > 7)
	{
		apu->frameSequencerStep = 0;
	}

	
}

// 256 Hz. If length is enabled in NR24, count down and turn the channel off at 0.
static void apuClockChannel2Length(apu_t* apu)
{
	uint8_t nr24 = APU_REG(apu, IO_REG_NR24);
	if ((nr24 & NRX4_LENGTH_ENABLE_BIT) && apu->channel2.lengthCounter > 0)
	{
		apu->channel2.lengthCounter--;
		if (apu->channel2.lengthCounter == 0)
		{
			apu->channel2.enabled = false;
		}
	}

	
}

// 64 Hz. Step volume up or down by 1 every NR22-period clocks.
static void apuClockChannel2Envelope(apu_t* apu)
{
	uint8_t nr22 = APU_REG(apu, IO_REG_NR22);
	uint8_t period = (nr22 & NRX2_ENVELOPE_PERIOD_MASK);
	if (period == 0)
	{
		return;
	}

	apu->channel2.envelopeTimer--;
	if (apu->channel2.envelopeTimer == 0)
	{
		apu->channel2.envelopeTimer = period;
		if (nr22 & NRX2_ENVELOPE_INCREASE_BIT)
		{
			apu->channel2.volume++;
			if (apu->channel2.volume > CHANNEL_MAX_VOLUME)
			{
				apu->channel2.volume = CHANNEL_MAX_VOLUME;
			}
		}
		else
		{
			if (apu->channel2.volume >  0)
			{
				apu->channel2.volume--;
			}
		}

		
	}

}
