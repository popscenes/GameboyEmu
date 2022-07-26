#include "cpu.h""
#include "bus.h""
#include <stdio.h>

cpu_t cpuInstance = { 0 };

typedef struct {
	char name[8];
	uint8_t instructionLength;
	uint8_t type;
} instruction_t;

void cpuInit() {
	cpuInstance.pc = 0x100;
}

void jump(uint16_t address)
{
	cpuInstance.pc = address;
}

uint8_t isFlagSet(uint8_t flag)
{
	return cpuInstance.f & flag;
}

void setFlags(uint8_t value, uint8_t subtract, uint8_t halfCarry, uint8_t carry)
{
	cpuInstance.f = 0;

	if (value == 0)
	{
		cpuInstance.f = cpuInstance.f | FLAG_ZERO;
	}
	if (subtract != 0)
	{
		cpuInstance.f = cpuInstance.f | FLAG_SUBTRACT;
	}
	if (halfCarry != 0)
	{
		cpuInstance.f = cpuInstance.f | FLAG_HALF_CARRY;
	}
	if (carry != 0)
	{
		cpuInstance.f = cpuInstance.f | FLAG_CARRY;
	}
}

void LD_MemTo8BitReg(cpu_t * cpu, uint8_t* reg, char regName)
{
	
	uint8_t loByte = readByteFromAddress(cpuInstance.pc);
	cpu->pc++;
	cpu->currentIstructionCycles = 8;
	*reg = loByte;
	printf("LD %c,%02X", regName, loByte);
}

void LD_MemTo16BitReg(cpu_t* cpu, uint16_t* reg, const char* regName)
{
	uint16_t word = readWordFromAddress(cpuInstance.pc);
	cpu->pc += 2;
	*reg = word;
	cpu->currentIstructionCycles = 12;

	printf("LD %s, %04X%", regName, word);
}

void DEC_8BitReg(cpu_t* cpu, uint8_t* reg, const char* regName)
{
	uint8_t originalValue = *reg;
	(*reg)--;

	uint8_t hcarry = (((originalValue & 0xF) - (*reg & 0xF)) & 0x10) == 0x10;
	cpu->currentIstructionCycles = 4;
	setFlags(*reg, 1, hcarry, 0);
	printf("DEC %s", regName);
}

void cpuStep() {
	
	cpuInstance.currentIstructionOpCode = readByteFromAddress(cpuInstance.pc);
	printf("%04X - %02X :", cpuInstance.pc, cpuInstance.currentIstructionOpCode);
	cpuInstance.pc++;
	if (cpuInstance.currentIstructionOpCode == 0x00)
	{
		printf("noop");
		cpuInstance.currentIstructionCycles = 4;
	}
	else if (cpuInstance.currentIstructionOpCode == 0x05)
	{
		DEC_8BitReg(&cpuInstance, &cpuInstance.b, "B");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x06)
	{
		LD_MemTo8BitReg(&cpuInstance, &cpuInstance.b, 'B');
	}
	else if (cpuInstance.currentIstructionOpCode == 0x0d)
	{
		DEC_8BitReg(&cpuInstance, &cpuInstance.c, "C");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x0e)
	{
		LD_MemTo8BitReg(&cpuInstance, &cpuInstance.c, 'C');
	}
	else if (cpuInstance.currentIstructionOpCode == 0x20)
	{
		int8_t jumpBytes = readByteFromAddress(cpuInstance.pc);
		cpuInstance.pc++;
		if (isFlagSet(FLAG_ZERO))
		{
			cpuInstance.pc += jumpBytes;
			cpuInstance.currentIstructionCycles = 12;
		}
		else
		{
			cpuInstance.currentIstructionCycles = 8;
		}

		printf("JR NZ,%02X", (uint8_t)jumpBytes);
	}
	else if (cpuInstance.currentIstructionOpCode == 0x21)
	{
		LD_MemTo16BitReg(&cpuInstance, &cpuInstance.hl, "HL");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x31)
	{
		LD_MemTo16BitReg(&cpuInstance, &cpuInstance.sp, "SP");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x32)
	{
		
		uint16_t address = (cpuInstance.h << 8) | cpuInstance.l;
		writeByteToAddress(address, cpuInstance.a);
		address--;
		cpuInstance.h = (address & 0xFF00) >> 8;
		cpuInstance.l = (address & 0x00FF);
		cpuInstance.currentIstructionCycles = 8;
		printf("LD (HL-),A");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x3E)
	{
		LD_MemTo8BitReg(&cpuInstance, &cpuInstance.a, 'A');
	}
	else if (cpuInstance.currentIstructionOpCode == 0xaf)
	{
		uint8_t value = cpuInstance.a ^ cpuInstance.a;
		setFlags(value, 0,0,0);
		cpuInstance.currentIstructionCycles = 4;
		printf("xor A");
	}
	else if (cpuInstance.currentIstructionOpCode == 0xc3)
	{
		
		uint16_t address = readWordFromAddress(cpuInstance.pc);
		cpuInstance.pc += 2;
		jump(address);

		cpuInstance.currentIstructionCycles = 16;
		printf("jp %04X", address);
	}
	else if (cpuInstance.currentIstructionOpCode == 0xE0)
	{
		uint8_t addressOffset = readByteFromAddress(cpuInstance.pc);
		cpuInstance.pc++;

		writeByteToAddress((addressOffset + 0xff00), cpuInstance.a);

		cpuInstance.currentIstructionCycles = 12;
		printf("LDH ($FF00+%04X), A", addressOffset);
	}
	else if (cpuInstance.currentIstructionOpCode == 0xEa)
	{
		uint16_t address = readWordFromAddress(cpuInstance.pc);
		cpuInstance.pc +=2;
		writeByteToAddress(address, cpuInstance.a);

		cpuInstance.currentIstructionCycles = 16;
		printf("LD %04X, A", address);
	}
	else if (cpuInstance.currentIstructionOpCode == 0xf0)
	{
		uint8_t address = readByteFromAddress(cpuInstance.pc);
		cpuInstance.pc++;
		uint8_t value = readByteFromAddress(address + 0xff00);
		cpuInstance.a = value;
		cpuInstance.currentIstructionCycles = 12;
		printf("LDH A,($FF00+%04X)", address);
	}
	else if (cpuInstance.currentIstructionOpCode == 0xf3)
	{
		printf("DI");
		cpuInstance.currentIstructionCycles = 4;
	}
	else
	{
		printf("Unknown");
	}

	printf("\n");

}

