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

uint16_t PopWordFromStack(cpu_t* cpu)
{
	uint8_t hiByte = readByteFromAddress(cpu->sp++);
	uint8_t loByte = readByteFromAddress(cpu->sp++);

	uint16_t value = (hiByte << 8) | (loByte);
	return value;
}


void OR_PCAddress(cpu_t* cpu)
{
	uint8_t value = readByteFromAddress(cpu->pc);
	cpu->pc++;
	cpu->a = cpu->a | value;
	cpuInstance.currentIstructionCycles = 8;
	setFlags(cpu->a, 0, 0, 0);
	printf("LD #%02x", value);

}

void OR_HLAddress(cpu_t* cpu)
{
	uint8_t value = readByteFromAddress(cpu->hl);
	cpu->a = cpu->a | value;
	cpuInstance.currentIstructionCycles = 8;
	setFlags(cpu->a, 0, 0, 0);
	printf("LD (HL)");

}


void OR_8BitReg(cpu_t* cpu, uint8_t* reg, const char* regName)
{
	cpu->a = cpu->a | *reg;
	cpuInstance.currentIstructionCycles = 4;
	setFlags(cpu->a, 0, 0, 0);
	printf("LD %s", regName);

}

void LD_8bitRegTo8BitReg(cpu_t* cpu, uint8_t* regDest, uint8_t* regSource, const char* regDestName, const char* regSourceName)
{
	(*regDest) = (*regSource);
	cpuInstance.currentIstructionCycles = 4;
	printf("LD %s, %s", regDestName, regSourceName);
}

void LD_RegValueToRegAddressHigh(cpu_t* cpu, uint8_t* regVal, uint8_t* regAddress, const char* regValName, const char* regAddressName)
{
	writeByteToAddress((*regAddress + 0xff00), *regVal);
	cpuInstance.currentIstructionCycles = 8;
	printf("LD (FF00+%s), %s", regAddressName, regValName);
}

void LD_ByteAtHLAddressToReg(cpu_t* cpu, uint8_t* reg, const char* regName)
{
	uint8_t loByte = readByteFromAddress(cpu->hl);
	*reg = loByte;
	cpu->currentIstructionCycles = 8;
	printf("LD %s, (HL+)", regName);
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

void Ret(cpu_t* cpu)
{
	uint16_t address = PopWordFromStack(cpu);
	cpu->currentIstructionCycles = 16;
	cpu->pc = address;
	printf("RET");
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



	else if (cpuInstance.currentIstructionOpCode == 0x40)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.b, &cpuInstance.b, "B", "B");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x41)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.b, &cpuInstance.c, "B", "C");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x42)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.b, &cpuInstance.d, "B", "D");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x43)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.b, &cpuInstance.e, "B", "E");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x44)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.b, &cpuInstance.h, "B", "H");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x45)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.b, &cpuInstance.l, "B", "L");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x46)
	{
		LD_ByteAtHLAddressToReg(&cpuInstance, &cpuInstance.b, "B");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x47)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.b, &cpuInstance.a, "B", "A");
	}



	else if (cpuInstance.currentIstructionOpCode == 0x48)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.c, &cpuInstance.b, "C", "B");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x49)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.c, &cpuInstance.c, "C", "C");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x4a)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.c, &cpuInstance.d, "C", "D");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x4b)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.c, &cpuInstance.e, "C", "E");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x4c)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.c, &cpuInstance.h, "C", "H");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x4d)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.c, &cpuInstance.l, "C", "L");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x4e)
	{
		LD_ByteAtHLAddressToReg(&cpuInstance, &cpuInstance.c, "C");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x4f)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.c, &cpuInstance.a, "C", "A");
	}



	else if (cpuInstance.currentIstructionOpCode == 0x50)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.d, &cpuInstance.b, "D", "B");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x51)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.d, &cpuInstance.c, "D", "C");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x52)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.d, &cpuInstance.d, "D", "D");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x53)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.d, &cpuInstance.e, "D", "E");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x54)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.d, &cpuInstance.h, "D", "H");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x55)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.d, &cpuInstance.l, "D", "L");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x56)
	{
		LD_ByteAtHLAddressToReg(&cpuInstance, &cpuInstance.d, "D");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x57)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.d, &cpuInstance.a, "D", "A");
	}



	else if (cpuInstance.currentIstructionOpCode == 0x58)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.e, &cpuInstance.b, "E", "B");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x59)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.e, &cpuInstance.c, "E", "C");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x5a)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.e, &cpuInstance.d, "E", "D");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x5b)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.e, &cpuInstance.e, "E", "E");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x5c)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.e, &cpuInstance.h, "E", "H");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x5d)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.e, &cpuInstance.l, "E", "L");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x5e)
	{
		LD_ByteAtHLAddressToReg(&cpuInstance, &cpuInstance.e, "E");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x5f)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.e, &cpuInstance.a, "E", "A");
	}



	else if (cpuInstance.currentIstructionOpCode == 0x60)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.h, &cpuInstance.b, "H", "B");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x61)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.h, &cpuInstance.c, "H", "C");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x62)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.h, &cpuInstance.d, "H", "D");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x63)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.h, &cpuInstance.e, "H", "E");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x64)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.h, &cpuInstance.h, "H", "H");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x65)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.h, &cpuInstance.l, "H", "L");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x66)
	{
		LD_ByteAtHLAddressToReg(&cpuInstance, &cpuInstance.h, "H");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x67)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.h, &cpuInstance.a, "H", "A");
	}



	else if (cpuInstance.currentIstructionOpCode == 0x68)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.l, &cpuInstance.b, "L", "B");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x69)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.l, &cpuInstance.c, "L", "C");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x6a)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.l, &cpuInstance.d, "L", "D");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x6b)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.l, &cpuInstance.e, "L", "E");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x6c)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.l, &cpuInstance.h, "L", "H");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x6d)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.l, &cpuInstance.l, "L", "L");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x6e)
	{
		LD_ByteAtHLAddressToReg(&cpuInstance, &cpuInstance.l, "L");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x6f)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.l, &cpuInstance.a, "L", "A");
	}



	else if (cpuInstance.currentIstructionOpCode == 0x78)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.a, &cpuInstance.b, "A", "B");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x78)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.a, &cpuInstance.c, "A", "C");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x7a)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.a, &cpuInstance.d, "A", "D");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x7b)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.a, &cpuInstance.e, "A", "E");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x7c)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.a, &cpuInstance.h, "A", "H");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x7d)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.a, &cpuInstance.l, "A", "L");
	}
	else if (cpuInstance.currentIstructionOpCode == 0x7e)
	{
		LD_ByteAtHLAddressToReg(&cpuInstance, &cpuInstance.a, "A");
	}
	
	else if (cpuInstance.currentIstructionOpCode == 0x7f)
	{
		LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.a, &cpuInstance.a,  "A", "A");
	}
	else if (cpuInstance.currentIstructionOpCode == 0xaf)
	{
		uint8_t value = cpuInstance.a ^ cpuInstance.a;
		setFlags(value, 0,0,0);
		cpuInstance.currentIstructionCycles = 4;
		printf("xor A");
	}
	else if (cpuInstance.currentIstructionOpCode == 0xb0)
	{
		OR_8BitReg(&cpuInstance, &cpuInstance.b, "B");
	}
	else if (cpuInstance.currentIstructionOpCode == 0xb1)
	{
		OR_8BitReg(&cpuInstance, &cpuInstance.c, "C");
	}
	else if (cpuInstance.currentIstructionOpCode == 0xb2)
	{
		OR_8BitReg(&cpuInstance, &cpuInstance.d, "D");
	}
	else if (cpuInstance.currentIstructionOpCode == 0xb3)
	{
		OR_8BitReg(&cpuInstance, &cpuInstance.e, "E");
	}
	else if (cpuInstance.currentIstructionOpCode == 0xb4)
	{
		OR_8BitReg(&cpuInstance, &cpuInstance.h, "H");
	}
	else if (cpuInstance.currentIstructionOpCode == 0xb5)
	{
		OR_8BitReg(&cpuInstance, &cpuInstance.l, "L");
	}
	else if (cpuInstance.currentIstructionOpCode == 0xb5)
	{
		OR_HLAddress(&cpuInstance);
	}

	
	else if (cpuInstance.currentIstructionOpCode == 0xb7)
	{
		OR_8BitReg(&cpuInstance, &cpuInstance.a, "A");
	}

	
	else if (cpuInstance.currentIstructionOpCode == 0xc3)
	{
		
		uint16_t address = readWordFromAddress(cpuInstance.pc);
		cpuInstance.pc += 2;
		jump(address);

		cpuInstance.currentIstructionCycles = 16;
		printf("jp %04X", address);
	}
	else if (cpuInstance.currentIstructionOpCode == 0xc9)
	{
		Ret(&cpuInstance);
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
	else if (cpuInstance.currentIstructionOpCode == 0xfb)
	{
		printf("EI");
		cpuInstance.currentIstructionCycles = 4;
	}
	else if (cpuInstance.currentIstructionOpCode == 0xf6)
	{
	OR_PCAddress(&cpuInstance);
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

