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
void setFlag(uint8_t flag)
{
	cpuInstance.f = cpuInstance.f | FLAG_ZERO;
}

void resetFlag(uint8_t flag)
{
	cpuInstance.f = cpuInstance.f & ~FLAG_ZERO;
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





void ADD_8BitReg(cpu_t* cpu, uint8_t* reg, const char* regName)
{
	uint8_t originalValue = cpu->a;
	cpu->a = cpu->a + *reg;
	cpuInstance.currentIstructionCycles = 4;
	uint8_t hcarry = (((originalValue & 0xF) + (*reg & 0xF)) & 0x10) == 0x10;
	uint8_t carry = (originalValue > cpu->a);

	setFlags(cpu->a, 0, hcarry, carry);
	printf("ADD %s", regName);
}

void OR_8BitReg(cpu_t* cpu, uint8_t* reg, const char* regName)
{
	cpu->a = cpu->a | *reg;
	cpuInstance.currentIstructionCycles = 4;
	setFlags(cpu->a, 0, 0, 0);
	printf("OR %s", regName);

}

void OR_PCAddress(cpu_t* cpu)
{
	uint8_t value = readByteFromAddress(cpu->pc);
	cpu->pc++;
	cpu->a = cpu->a | value;
	cpuInstance.currentIstructionCycles = 8;
	setFlags(cpu->a, 0, 0, 0);
	printf("OR #%02x", value);

}

void OR_HLAddress(cpu_t* cpu)
{
	uint8_t value = readByteFromAddress(cpu->hl);
	cpu->a = cpu->a | value;
	cpuInstance.currentIstructionCycles = 8;
	setFlags(cpu->a, 0, 0, 0);
	printf("OR (HL)");

}


void XOR_8BitReg(cpu_t* cpu, uint8_t* reg, const char* regName)
{
	cpuInstance.a = cpuInstance.a ^ *reg;
	setFlags(cpuInstance.a, 0, 0, 0);
	cpuInstance.currentIstructionCycles = 4;
	printf("XOR %s", regName);

}

void SWAP_8BitReg(cpu_t* cpu, uint8_t* reg, const char* regName)
{
	uint8_t lowNibble =  *reg;
	uint8_t highNibble = *reg;
	
	highNibble = (highNibble << 4) & 0xF0;
	lowNibble = (lowNibble >> 4) & 0xF0;

	*reg = highNibble | lowNibble;

	cpuInstance.currentIstructionCycles = 8;
	setFlags(*reg, 0, 0, 0);
	printf("swap %s", regName);

}

void RST(cpu_t* cpu, uint8_t jumpParam)
{
	PushWordToStack(cpu, cpu->pc);
	
	cpuInstance.pc = jumpParam;
	cpuInstance.currentIstructionCycles = 16;
	printf("RST %x02", jumpParam);
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

void AND_8BitReg(cpu_t* cpu, uint8_t* reg, const char* regName)
{
	cpu->a = cpu->a & *reg;
	setFlags(cpu->a, 0, 1, 0);
	cpu->currentIstructionCycles = 4;
	printf("AND %s", regName);
}

void AND_FromHL(cpu_t* cpu)
{
	uint8_t value = readByteFromAddress(cpu->hl);
	cpu->a = cpu->a & value;
	setFlags(cpu->a, 0, 1, 0);
	cpu->currentIstructionCycles = 8;
	printf("AND d8 %02X", value);
}

void AND_FromPC(cpu_t* cpu)
{
	uint8_t value =  readByteFromAddress(cpu->pc);
	cpu->pc++;
	cpu->a = cpu->a & value;
	setFlags(cpu->a, 0, 1, 0);
	cpu->currentIstructionCycles = 8;
	printf("AND d8 %02X", value);
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

void CPL(cpu_t* cpu)
{
	cpu->a = ~cpu->a;
	cpu->currentIstructionCycles = 4;
	setFlag(FLAG_SUBTRACT);
	setFlag(FLAG_HALF_CARRY);
	printf("CPL");
}

void cpuStep() {
	
	cpuInstance.currentIstructionOpCode = readByteFromAddress(cpuInstance.pc);
	
	if (cpuInstance.currentIstructionOpCode == 0xcb)
	{
		cpuInstance.currentIstructionCBOpCode = readByteFromAddress(cpuInstance.pc+1);
		printf("%04X - %02X %02X:", cpuInstance.pc, cpuInstance.currentIstructionOpCode, cpuInstance.currentIstructionCBOpCode);
		cpuInstance.pc++;
		cpuInstance.pc++;
	}
	else
	{
		printf("%04X - %02X :", cpuInstance.pc, cpuInstance.currentIstructionOpCode);
		cpuInstance.pc++;
	}
	
	switch (cpuInstance.currentIstructionOpCode)
	{
	
		case 0x00:
		{
			printf("noop");
			cpuInstance.currentIstructionCycles = 4;
			break;
		}
		case 0x01:
		{
			LD_WordTo16BitReg(&cpuInstance, &cpuInstance.bc, "BC");
			break;
		}
		case 0x05:
		{
			DEC_8BitReg(&cpuInstance, &cpuInstance.b, "B");
			break;
		}
		case 0x06:
		{
			LD_ByteToReg(&cpuInstance, &cpuInstance.b, 'B');
			break;
		}
		case 0x0b:
		{
			DEC_16BitReg(&cpuInstance, &cpuInstance.bc, "BC");
			break;
		}
		case 0x0c:
		{
			INC_8BitReg(&cpuInstance, &cpuInstance.c, "C");
			break;
		}
		case 0x0d:
		{
			DEC_8BitReg(&cpuInstance, &cpuInstance.c, "C");
			break;
		}
		case 0x0e:
		{
			LD_ByteToReg(&cpuInstance, &cpuInstance.c, 'C');
			break;
		}
		case 0x11:
		{
			LD_WordTo16BitReg(&cpuInstance, &cpuInstance.de, "DE");
			break;
		}
		case 0x16:
		{
			LD_ByteToReg(&cpuInstance, &cpuInstance.d, 'D');
			break;
		}
		case 0x1b:
		{
			DEC_16BitReg(&cpuInstance, &cpuInstance.de, "DE");
			break;
		}
		case 0x1e:
		{
			LD_ByteToReg(&cpuInstance, &cpuInstance.e, 'E');
			break;
		}
		case 0x20:
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
			break;
		}
		case 0x21:
		{
			LD_WordTo16BitReg(&cpuInstance, &cpuInstance.hl, "HL");
			break;
		}
		case 0x26:
		{
			LD_ByteToReg(&cpuInstance, &cpuInstance.h, 'H');
			break;
		}
		case 0x2b:
		{
			DEC_16BitReg(&cpuInstance, &cpuInstance.hl, "HL");
			break;
		}
		case 0x2a:
		{
			LD_ByteAtHLAddressToRegWithInc(&cpuInstance, &cpuInstance.a, "A");
			break;
		}
		case 0x2e:
		{
			LD_ByteToReg(&cpuInstance, &cpuInstance.h, 'L');
			break;
		}
		case 0x2f:
		{
			CPL(&cpuInstance);
			break;
		}
		case 0x31:
		{
			LD_WordTo16BitReg(&cpuInstance, &cpuInstance.sp, "SP");
			break;
		}
		case 0x32:
		{
			uint16_t address = cpuInstance.hl;
			writeByteToAddress(address, cpuInstance.a);
			cpuInstance.hl--;
			cpuInstance.currentIstructionCycles = 8;
			printf("LD (HL-),A");
			break;
		}
		case 0x36:
		{
			LD_ByteToAddress(&cpuInstance, cpuInstance.hl, "HL");
			break;
		}
		case 0x3b:
		{
			DEC_16BitReg(&cpuInstance, &cpuInstance.sp, "SP");
			break;
		}
		case 0x3E:
		{
			LD_ByteToReg(&cpuInstance, &cpuInstance.a, 'A');
			break;
		}



		case 0x40:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.b, &cpuInstance.b, "B", "B");
			break;
		}
		case 0x41:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.b, &cpuInstance.c, "B", "C");
			break;
		}
		case 0x42:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.b, &cpuInstance.d, "B", "D");
			break;
		}
		case 0x43:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.b, &cpuInstance.e, "B", "E");
			break;
		}
		case 0x44:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.b, &cpuInstance.h, "B", "H");
			break;
		}
		case 0x45:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.b, &cpuInstance.l, "B", "L");
			break;
		}
		case 0x46:
		{
			LD_ByteAtHLAddressToReg(&cpuInstance, &cpuInstance.b, "B");
			break;
		}
		case 0x47:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.b, &cpuInstance.a, "B", "A");
			break;
		}



		case 0x48:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.c, &cpuInstance.b, "C", "B");
			break;
		}
		case 0x49:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.c, &cpuInstance.c, "C", "C");
			break;
		}
		case 0x4a:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.c, &cpuInstance.d, "C", "D");
			break;
		}
		case 0x4b:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.c, &cpuInstance.e, "C", "E");
			break;
		}
		case 0x4c:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.c, &cpuInstance.h, "C", "H");
			break;
		}
		case 0x4d:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.c, &cpuInstance.l, "C", "L");
			break;
		}
		case 0x4e:
		{
			LD_ByteAtHLAddressToReg(&cpuInstance, &cpuInstance.c, "C");
			break;
		}
		case 0x4f:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.c, &cpuInstance.a, "C", "A");
			break;
		}



		case 0x50:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.d, &cpuInstance.b, "D", "B");
			break;
		}
		case 0x51:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.d, &cpuInstance.c, "D", "C");
			break;
		}
		case 0x52:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.d, &cpuInstance.d, "D", "D");
			break;
		}
		case 0x53:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.d, &cpuInstance.e, "D", "E");
			break;
		}
		case 0x54:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.d, &cpuInstance.h, "D", "H");
			break;
		}
		case 0x55:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.d, &cpuInstance.l, "D", "L");
			break;
		}
		case 0x56:
		{
			LD_ByteAtHLAddressToReg(&cpuInstance, &cpuInstance.d, "D");
			break;
		}
		case 0x57:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.d, &cpuInstance.a, "D", "A");
			break;
		}



		case 0x58:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.e, &cpuInstance.b, "E", "B");
			break;
		}
		case 0x59:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.e, &cpuInstance.c, "E", "C");
			break;
		}
		case 0x5a:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.e, &cpuInstance.d, "E", "D");
			break;
		}
		case 0x5b:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.e, &cpuInstance.e, "E", "E");
			break;
		}
		case 0x5c:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.e, &cpuInstance.h, "E", "H");
			break;
		}
		case 0x5d:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.e, &cpuInstance.l, "E", "L");
			break;
		}
		case 0x5e:
		{
			LD_ByteAtHLAddressToReg(&cpuInstance, &cpuInstance.e, "E");
			break;
		}
		case 0x5f:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.e, &cpuInstance.a, "E", "A");
			break;
		}



		case 0x60:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.h, &cpuInstance.b, "H", "B");
			break;
		}
		case 0x61:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.h, &cpuInstance.c, "H", "C");
			break;
		}
		case 0x62:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.h, &cpuInstance.d, "H", "D");
			break;
		}
		case 0x63:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.h, &cpuInstance.e, "H", "E");
			break;
		}
		case 0x64:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.h, &cpuInstance.h, "H", "H");
			break;
		}
		case 0x65:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.h, &cpuInstance.l, "H", "L");
		}
		case 0x66:
		{
			LD_ByteAtHLAddressToReg(&cpuInstance, &cpuInstance.h, "H");
			break;
		}
		case 0x67:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.h, &cpuInstance.a, "H", "A");
			break;
		}



		case 0x68:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.l, &cpuInstance.b, "L", "B");
			break;
		}
		case 0x69:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.l, &cpuInstance.c, "L", "C");
			break;
		}
		case 0x6a:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.l, &cpuInstance.d, "L", "D");
			break;
		}
		case 0x6b:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.l, &cpuInstance.e, "L", "E");
			break;
		}
		case 0x6c:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.l, &cpuInstance.h, "L", "H");
			break;
		}
		case 0x6d:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.l, &cpuInstance.l, "L", "L");
			break;
		}
		case 0x6e:
		{
			LD_ByteAtHLAddressToReg(&cpuInstance, &cpuInstance.l, "L");
			break;
		}
		case 0x6f:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.l, &cpuInstance.a, "L", "A");
			break;
		}



		case 0x78:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.a, &cpuInstance.b, "A", "B");
			break;
		}
		case 0x79:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.a, &cpuInstance.c, "A", "C");
			break;
		}
		case 0x7a:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.a, &cpuInstance.d, "A", "D");
			break;
		}
		case 0x7b:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.a, &cpuInstance.e, "A", "E");
			break;
		}
		case 0x7c:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.a, &cpuInstance.h, "A", "H");
			break;
		}
		case 0x7d:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.a, &cpuInstance.l, "A", "L");
			break;
		}
		case 0x7e:
		{
			LD_ByteAtHLAddressToReg(&cpuInstance, &cpuInstance.a, "A");
			break;
		}
	
		case 0x7f:
		{
			LD_8bitRegTo8BitReg(&cpuInstance, &cpuInstance.a, &cpuInstance.a,  "A", "A");
			break;
		}

		case 0x80:
		{
			ADD_8BitReg(&cpuInstance, &cpuInstance.b, "B");
			break;
		}
		case 0x81:
		{
			ADD_8BitReg(&cpuInstance, &cpuInstance.c, "C");
			break;
		}
		case 0x82:
		{
			ADD_8BitReg(&cpuInstance, &cpuInstance.d, "D");
			break;
		}
		case 0x83:
		{
			ADD_8BitReg(&cpuInstance, &cpuInstance.e, "E");
			break;
		}
		case 0x84:
		{
			ADD_8BitReg(&cpuInstance, &cpuInstance.h, "H");
			break;
		}
		case 0x85:
		{
			ADD_8BitReg(&cpuInstance, &cpuInstance.l, "L");
			break;
		}
		case 0x87:
		{
			ADD_8BitReg(&cpuInstance, &cpuInstance.a, "A");
			break;
		}

	
		case 0xa0:
		{
			AND_8BitReg(&cpuInstance, &cpuInstance.b, "B");
			break;
		}
		case 0xa1:
		{
			AND_8BitReg(&cpuInstance, &cpuInstance.c, "C");
			break;
		}
		case 0xa2:
		{
			AND_8BitReg(&cpuInstance, &cpuInstance.d, "D");
			break;
		}
		case 0xa3:
		{
			AND_8BitReg(&cpuInstance, &cpuInstance.e, "E");
			break;
		}
		case 0xa4:
		{
			AND_8BitReg(&cpuInstance, &cpuInstance.h, "H");
			break;
		}
		case 0xa5:
		{
			AND_8BitReg(&cpuInstance, &cpuInstance.l, "L");
			break;
		}
		case 0xa6:
		{
			AND_FromHL(&cpuInstance);
			break;
		}
		case 0xa7:
		{
			AND_8BitReg(&cpuInstance, &cpuInstance.a, "A");
			break;
		}
		case 0xa8:
		{
			XOR_8BitReg(&cpuInstance, &cpuInstance.b, "B");
			break;
		}
		case 0xa9:
		{
			XOR_8BitReg(&cpuInstance, &cpuInstance.c, "C");
			break;
		}
		case 0xaa:
		{
			XOR_8BitReg(&cpuInstance, &cpuInstance.d, "D");
			break;
		}
		case 0xab:
		{
			XOR_8BitReg(&cpuInstance, &cpuInstance.e, "E");
			break;
		}
		case 0xac:
		{
			XOR_8BitReg(&cpuInstance, &cpuInstance.h, "H");
			break;
		}
		case 0xad:
		{
			XOR_8BitReg(&cpuInstance, &cpuInstance.l, "L");
			break;
		}
		case 0xaf:
		{
			XOR_8BitReg(&cpuInstance, &cpuInstance.a, "A");
			break;
		}

		case 0xb0:
		{
			OR_8BitReg(&cpuInstance, &cpuInstance.b, "B");
			break;
		}
		case 0xb1:
		{
			OR_8BitReg(&cpuInstance, &cpuInstance.c, "C");
			break;
		}
		case 0xb2:
		{
			OR_8BitReg(&cpuInstance, &cpuInstance.d, "D");
			break;
		}
		case 0xb3:
		{
			OR_8BitReg(&cpuInstance, &cpuInstance.e, "E");
			break;
		}
		case 0xb4:
		{
			OR_8BitReg(&cpuInstance, &cpuInstance.h, "H");
			break;
		}
		case 0xb5:
		{
			OR_8BitReg(&cpuInstance, &cpuInstance.l, "L");
			break;
		}
		case 0xb6:
		{
			OR_HLAddress(&cpuInstance);
			break;
		}
		case 0xb7:
		{
			OR_8BitReg(&cpuInstance, &cpuInstance.a, "A");
			break;
		}

	
		case 0xc3:
		{
		
			uint16_t address = readWordFromAddress(cpuInstance.pc);
			cpuInstance.pc += 2;
			jump(address);

			cpuInstance.currentIstructionCycles = 16;
			printf("jp %04X", address);
			break;
		}
		case 0xc9:
		{
			Ret(&cpuInstance);
			break;
		}
		case 0xcb:
		{
		
			if (cpuInstance.currentIstructionCBOpCode == 0x30)
			{
				SWAP_8BitReg(&cpuInstance, &cpuInstance.b, "B");
			}
			else if (cpuInstance.currentIstructionCBOpCode == 0x31)
			{
				SWAP_8BitReg(&cpuInstance, &cpuInstance.c, "C");
			}
			else if (cpuInstance.currentIstructionCBOpCode == 0x32)
			{
				SWAP_8BitReg(&cpuInstance, &cpuInstance.d, "D");
			}
			else if (cpuInstance.currentIstructionCBOpCode == 0x33)
			{
				SWAP_8BitReg(&cpuInstance, &cpuInstance.e, "E");
			}
			else if (cpuInstance.currentIstructionCBOpCode == 0x34)
			{
				SWAP_8BitReg(&cpuInstance, &cpuInstance.h, "H");
			}
			else if (cpuInstance.currentIstructionCBOpCode == 0x35)
			{
				SWAP_8BitReg(&cpuInstance, &cpuInstance.l, "L");
			}
			else if (cpuInstance.currentIstructionCBOpCode == 0x37)
			{
				SWAP_8BitReg(&cpuInstance, &cpuInstance.a, "A");
			}
			else
			{
				printf("Unknown CB");
			}
			break;
		}
		case 0xcd:
		{
			Call(&cpuInstance);
			break;
		}
		case 0xE0:
		{
			uint8_t addressOffset = readByteFromAddress(cpuInstance.pc);
			cpuInstance.pc++;

			writeByteToAddress((addressOffset + 0xff00), cpuInstance.a);

			cpuInstance.currentIstructionCycles = 12;
			printf("LDH ($FF00+%04X), A", addressOffset);
			break;
		}
		case 0xE2:
		{
			LD_RegValueToRegAddressHigh(&cpuInstance, &cpuInstance.a, &cpuInstance.c, "A", "C");
			break;
		}
		case 0xEa:
		{
			uint16_t address = readWordFromAddress(cpuInstance.pc);
			cpuInstance.pc +=2;
			writeByteToAddress(address, cpuInstance.a);

			cpuInstance.currentIstructionCycles = 16;
			printf("LD %04X, A", address);
			break;
		}
		case 0xE6:
		{
			AND_FromPC(&cpuInstance);
			break;
		}
		case 0xEf:
		{
			RST(&cpuInstance, 0x28);
			break;
		}
		case 0xf0:
		{
			uint8_t address = readByteFromAddress(cpuInstance.pc);
			cpuInstance.pc++;
			uint8_t value = readByteFromAddress(address + 0xff00);
			cpuInstance.a = value;
			cpuInstance.currentIstructionCycles = 12;
			printf("LDH A,($FF00+%04X)", address);
			break;
		}
		case 0xf3:
		{
			printf("DI");
			cpuInstance.currentIstructionCycles = 4;
			break;
		}
		case 0xfb:
		{
			printf("EI");
			cpuInstance.currentIstructionCycles = 4;
			break;
		}
		case 0xf6:
		{
			OR_PCAddress(&cpuInstance);
			break;
		}
		case 0xfe:
		{
		
			uint8_t value = readByteFromAddress(cpuInstance.pc);
			uint8_t compare = cpuInstance.a - value;

			uint8_t hcarry = (((cpuInstance.a & 0xF) - (compare & 0xF)) & 0x10) == 0x10;
			uint8_t carry = (cpuInstance.a < value);
			setFlags(compare, 1, hcarry, carry);

			cpuInstance.pc++;
			cpuInstance.currentIstructionCycles = 8;
			printf("CP %02X", value);
			break;
		}
		default:
		{
			cpuInstance.currentIstructionCycles = 0;
			printf("Unknown");
			break;
		}
	}

	cpuInstance.totalCycles += cpuInstance.currentIstructionCycles;

	printf("\tA:%02X B:%02X C:%02X D:%02X E:%02X F:%02X HL:%04X SP:%04X\n", cpuInstance.a, cpuInstance.b, cpuInstance.c, cpuInstance.d, cpuInstance.e, cpuInstance.f, cpuInstance.hl, cpuInstance.sp);

}

