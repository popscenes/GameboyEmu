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

void cpuStep() {
	cpuInstance.currentIstructionOpCode = readByteFromAddress(cpuInstance.pc);
	cpuInstance.pc++;
	instruction_t inst = { 0 };
	if (cpuInstance.currentIstructionOpCode == 0x00)
	{
		printf("noop\n");
		cpuInstance.currentIstructionCycles = 4;
	}
	else if (cpuInstance.currentIstructionOpCode == 0x05)
	{
		uint8_t originalValue = cpuInstance.b;
		cpuInstance.b--;

		uint8_t hcarry = (((originalValue & 0xF) - (cpuInstance.b & 0xF)) & 0x10) == 0x10;
		cpuInstance.currentIstructionCycles = 4;
		setFlags(cpuInstance.b, 1, hcarry,0);
		printf("DEC B\n");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x06)
	{
		uint8_t loByte = readByteFromAddress(cpuInstance.pc);
		cpuInstance.pc++;
		cpuInstance.currentIstructionCycles = 8;
		cpuInstance.b = loByte;
		printf("LD B,%02X\n", loByte);
	}
	else if (cpuInstance.currentIstructionOpCode == 0x0d)
	{
		uint8_t originalValue = cpuInstance.b;
		cpuInstance.c--;

		uint8_t hcarry = (((originalValue & 0xF) - (cpuInstance.b & 0xF)) & 0x10) == 0x10;
		cpuInstance.currentIstructionCycles = 4;
		setFlags(cpuInstance.b, 1, hcarry, 0);
		printf("DEC C\n");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x0e)
	{
		uint8_t loByte = readByteFromAddress(cpuInstance.pc);
		cpuInstance.pc++;
		cpuInstance.currentIstructionCycles = 8;
		cpuInstance.c = loByte;
		printf("LD C,%02X\n", loByte);
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

		printf("JR NZ,%02X\n", (uint8_t)jumpBytes);
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
	else if (cpuInstance.currentIstructionOpCode == 0x32)
	{
		
		uint16_t address = (cpuInstance.h << 8) | cpuInstance.l;
		writeByteToAddress(address, cpuInstance.a);
		address--;
		cpuInstance.h = (address & 0xFF00) >> 8;
		cpuInstance.l = (address & 0x00FF);
		cpuInstance.currentIstructionCycles = 8;
		printf("LD (HL-),A\n");
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
		setFlags(value, 0,0,0);
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
	else if (cpuInstance.currentIstructionOpCode == 0xf0)
	{
		uint8_t address = readByteFromAddress(cpuInstance.pc);
		cpuInstance.pc++;
		uint8_t value = readByteFromAddress(address + 0xff00);
		cpuInstance.a = value;
		cpuInstance.currentIstructionCycles = 12;
		printf("LDH A,($FF00+%04X)\n", address);
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

