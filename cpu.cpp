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
void setFlags(uint8_t value)
{
	cpuInstance.f = 0;
	if (value == 0)
	{
		cpuInstance.f = cpuInstance.f | FLAG_ZERO;
	}
}

void cpuStep() {
	cpuInstance.currentIstructionOpCode = readByteFromAddress(cpuInstance.pc);
	cpuInstance.pc++;
	instruction_t inst = { 0 };
	if (cpuInstance.currentIstructionOpCode == 0x00)
	{
		printf("noop\n");
		cpuInstance.currentIstructionCycles = 4;
	}
	else if (cpuInstance.currentIstructionOpCode == 0x21)
	{
		uint8_t loByte = readByteFromAddress(cpuInstance.pc);
		uint8_t hiByte = readByteFromAddress(cpuInstance.pc+1);

		cpuInstance.pc += 2;

		cpuInstance.h = hiByte;
		cpuInstance.l = loByte;
		
		cpuInstance.currentIstructionCycles = 12;

		printf("LD HL, %02X%02X\n", hiByte, loByte);
	}
	else if (cpuInstance.currentIstructionOpCode == 0x31)
	{
		uint16_t value = readWordFromAddress(cpuInstance.pc);

		cpuInstance.pc += 2;
		cpuInstance.sp = value;

		cpuInstance.currentIstructionCycles = 12;
		printf("LD SP,%04X\n", value);
	}
	else if (cpuInstance.currentIstructionOpCode == 0x3E)
	{
		
		uint8_t byte = readByteFromAddress(cpuInstance.pc);

		cpuInstance.pc++;
		cpuInstance.a = byte;

		cpuInstance.currentIstructionCycles = 8;
		printf("LD A,#%d\n", byte);
	}
	else if (cpuInstance.currentIstructionOpCode == 0xaf)
	{
		uint8_t value = cpuInstance.a ^ cpuInstance.a;
		setFlags(value);
		cpuInstance.currentIstructionCycles = 4;
		printf("xor A\n");
	}
	else if (cpuInstance.currentIstructionOpCode == 0xc3)
	{
		
		uint16_t address = readWordFromAddress(cpuInstance.pc);
		cpuInstance.pc += 2;
		jump(address);

		cpuInstance.currentIstructionCycles = 16;
		printf("jp %04X\n", address);
	}
	else if (cpuInstance.currentIstructionOpCode == 0xE0)
	{
		uint8_t addressOffset = readByteFromAddress(cpuInstance.pc);
		cpuInstance.pc++;

		writeByteToAddress((addressOffset + 0xff00), cpuInstance.a);

		cpuInstance.currentIstructionCycles = 112;
		printf("LDH ($FF00+%04X), A\n", addressOffset);
	}
	else if (cpuInstance.currentIstructionOpCode == 0xEa)
	{
		uint16_t address = readWordFromAddress(cpuInstance.pc);
		cpuInstance.pc +=2;
		writeByteToAddress(address, cpuInstance.a);

		cpuInstance.currentIstructionCycles = 16;
		printf("LD %04X, A\n", address);
	}
	else if (cpuInstance.currentIstructionOpCode == 0xf3)
	{
		printf("DI\n");
		cpuInstance.currentIstructionCycles = 4;
	}
	else
	{
		printf("Unknown\n");
	}

}

