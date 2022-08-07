#include "cpu.h"
#include "bus.h"
#include <stdio.h>
#include <windows.h>

cpu_t cpuInstance = { 0 };

typedef struct {
	char name[8];
} instruction_t;

uint8_t cpuCurrentIstructionCycles()
{
	return cpuInstance.currentIstructionCycles;
}

const char * instructions[0x100];

void cpuInit() {
	instructions[0x00] = "NOP";
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

void PushWordToStack(cpu_t* cpu, uint16_t word)
{
	uint8_t loByte = word & 0x00ff;
	uint8_t hiByte = (word >> 8) & 0x00ff;

	cpu->sp--;
	writeByteToAddress(cpu->sp, loByte);
	cpu->sp--;
	writeByteToAddress(cpu->sp, hiByte);
	
}

void LD_RegValueToRegAddressHigh(cpu_t* cpu, uint8_t* regVal, uint8_t* regAddress, const char* regValName, const char* regAddressName)
{
	writeByteToAddress((*regAddress + 0xff00), *regVal);
	cpuInstance.currentIstructionCycles = 8;
	printf("LD (FF00+%s), %s", regAddressName, regValName);
}

void LD_ByteAtHLAddressToRegWithInc(cpu_t* cpu, uint8_t* reg, const char* regName)
{
	uint8_t loByte = readByteFromAddress(cpu->hl);
	*reg = loByte;
	cpu->hl++;
	cpu->currentIstructionCycles = 8;
	printf("LD %s, (HL+)", regName);
}

void LD_ByteToAddress(cpu_t* cpu, uint16_t address, const char* regName)
{
	uint8_t loByte = readByteFromAddress(cpu->pc);
	writeByteToAddress(address, loByte);
	cpu->currentIstructionCycles = 12;
	cpu->pc++;

	printf("LD (%s),%02X", regName, loByte);
}

void LD_ByteToReg(cpu_t * cpu, uint8_t* reg, char regName)
{
	
	uint8_t loByte = readByteFromAddress(cpu->pc);
	cpu->pc++;
	cpu->currentIstructionCycles = 8;
	*reg = loByte;
	
	printf("LD %c,%02X", regName, loByte);
	
}

void LD_WordTo16BitReg(cpu_t* cpu, uint16_t* reg, const char* regName)
{
	uint16_t word = readWordFromAddress(cpu->pc);
	cpu->pc += 2;
	*reg = word;
	cpu->currentIstructionCycles = 12;

	printf("LD %s, %04X", regName, word);
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

void DEC_16BitReg(cpu_t* cpu, uint16_t* reg, const char* regName)
{
	(*reg)--;
	cpu->currentIstructionCycles = 8;
	printf("DEC %s", regName);
}

void INC_8BitReg(cpu_t* cpu, uint8_t* reg, const char* regName)
{
	uint8_t originalValue = *reg;
	(*reg)++;

	uint8_t hcarry = (((originalValue & 0xF) + (*reg & 0xF)) & 0x10) == 0x10;
	cpu->currentIstructionCycles = 4;
	setFlags(*reg, 0, hcarry, 0);
	printf("DEC %s", regName);
}

void Call(cpu_t* cpu)
{
	uint16_t addressToJump = readWordFromAddress(cpu->pc);
	uint16_t addressToPush = cpu->pc+2;
	PushWordToStack(cpu, addressToPush);
	cpu->pc = addressToJump;
	cpu->currentIstructionCycles = 24;
	printf("CALL %0x4", addressToJump);
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
	else if (cpuInstance.currentIstructionOpCode == 0x01)
	{
		LD_WordTo16BitReg(&cpuInstance, &cpuInstance.bc, "BC");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x05)
	{
		DEC_8BitReg(&cpuInstance, &cpuInstance.b, "B");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x06)
	{
		LD_ByteToReg(&cpuInstance, &cpuInstance.b, 'B');
	}
	else if (cpuInstance.currentIstructionOpCode == 0x0b)
	{
		DEC_16BitReg(&cpuInstance, &cpuInstance.bc, "BC");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x0c)
	{
		INC_8BitReg(&cpuInstance, &cpuInstance.c, "C");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x0d)
	{
		DEC_8BitReg(&cpuInstance, &cpuInstance.c, "C");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x0e)
	{
		LD_ByteToReg(&cpuInstance, &cpuInstance.c, 'C');
	}
	else if (cpuInstance.currentIstructionOpCode == 0x11)
	{
		LD_WordTo16BitReg(&cpuInstance, &cpuInstance.de, "DE");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x16)
	{
		LD_ByteToReg(&cpuInstance, &cpuInstance.d, 'D');
	}
	else if (cpuInstance.currentIstructionOpCode == 0x1b)
	{
		DEC_16BitReg(&cpuInstance, &cpuInstance.de, "DE");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x1e)
	{
		LD_ByteToReg(&cpuInstance, &cpuInstance.e, 'E');
	}
	else if (cpuInstance.currentIstructionOpCode == 0x20)
	{
		int8_t jumpBytes = readByteFromAddress(cpuInstance.pc);
		cpuInstance.pc++;
		if (!isFlagSet(FLAG_ZERO))
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
		LD_WordTo16BitReg(&cpuInstance, &cpuInstance.hl, "HL");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x26)
	{
		LD_ByteToReg(&cpuInstance, &cpuInstance.h, 'H');
	}
	else if (cpuInstance.currentIstructionOpCode == 0x2b)
	{
		DEC_16BitReg(&cpuInstance, &cpuInstance.hl, "HL");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x2e)
	{
		LD_ByteToReg(&cpuInstance, &cpuInstance.h, 'L');
	}
	else if (cpuInstance.currentIstructionOpCode == 0x2a)
	{
		LD_ByteAtHLAddressToRegWithInc(&cpuInstance, &cpuInstance.a, "A");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x31)
	{
		LD_WordTo16BitReg(&cpuInstance, &cpuInstance.sp, "SP");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x32)
	{
		uint16_t address = cpuInstance.hl;
		writeByteToAddress(address, cpuInstance.a);
		cpuInstance.hl--;
		cpuInstance.currentIstructionCycles = 8;
		printf("LD (HL-),A");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x36)
	{
		LD_ByteToAddress(&cpuInstance, cpuInstance.hl, "HL");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x3b)
	{
	DEC_16BitReg(&cpuInstance, &cpuInstance.sp, "SP");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x3E)
	{
		LD_ByteToReg(&cpuInstance, &cpuInstance.a, 'A');
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
	else if (cpuInstance.currentIstructionOpCode == 0xcd)
	{
		Call(&cpuInstance);
	}
	else if (cpuInstance.currentIstructionOpCode == 0xE0)
	{
		uint8_t addressOffset = readByteFromAddress(cpuInstance.pc);
		cpuInstance.pc++;

		writeByteToAddress((addressOffset + 0xff00), cpuInstance.a);

		cpuInstance.currentIstructionCycles = 12;
		printf("LDH ($FF00+%04X), A", addressOffset);
	}
	else if (cpuInstance.currentIstructionOpCode == 0xE2)
	{
		LD_RegValueToRegAddressHigh(&cpuInstance, &cpuInstance.a, &cpuInstance.c, "A", "C");
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
	else if (cpuInstance.currentIstructionOpCode == 0xfe)
	{
		
		uint8_t value = readByteFromAddress(cpuInstance.pc);
		uint8_t compare = cpuInstance.a - value;

		uint8_t hcarry = (((cpuInstance.a & 0xF) - (compare & 0xF)) & 0x10) == 0x10;
		uint8_t carry = (cpuInstance.a < value);
		setFlags(compare, 1, hcarry, carry);

		cpuInstance.pc++;
		cpuInstance.currentIstructionCycles = 8;
		printf("CP %02X", value);
	}
	else
	{
 		cpuInstance.currentIstructionCycles = 0;
		printf("Unknown");
	}

	cpuInstance.totalCycles += cpuInstance.currentIstructionCycles;

	printf("\tA:%02X B:%02X C:%02X D:%02X E:%02X F:%02X HL:%04X SP:%04X\n", cpuInstance.a, cpuInstance.b, cpuInstance.c, cpuInstance.d, cpuInstance.e, cpuInstance.f, cpuInstance.hl, cpuInstance.sp);

}

