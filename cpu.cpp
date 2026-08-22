#include "cpu.h"
#include "bus.h"
#include "timer.h"
#include "GameBoy.h"
#include <stdio.h>
#include <windows.h>

typedef struct {
	char name[8];
} instruction_t;

uint8_t cpuCurrentIstructionCycles(gameboy_t* gb)
{
	return gb->cpu.currentIstructionCycles;
}

const char * instructions[0x100];

void cpuInit(gameboy_t* gb) {
	instructions[0x00] = "NOP";
	gb->cpu.pc = 0x100;
}

void jump(gameboy_t* gb, uint16_t address)
{
	cpu_t* cpu = &gb->cpu;
	cpu->pc = address;
}

uint8_t isFlagSet(gameboy_t* gb, uint8_t flag)
{
	cpu_t* cpu = &gb->cpu;
	return cpu->f & flag;
}
void setFlag(gameboy_t* gb, uint8_t flag)
{
	cpu_t* cpu = &gb->cpu;
	cpu->f = cpu->f | flag;
}

void resetFlag(gameboy_t* gb, uint8_t flag)
{
	cpu_t* cpu = &gb->cpu;
	cpu->f = cpu->f & ~flag;
}

void setFlags(gameboy_t* gb, uint8_t value, uint8_t subtract, uint8_t halfCarry, uint8_t carry)
{
	cpu_t* cpu = &gb->cpu;
	cpu->f = 0;

	if (value == 0)
	{
		cpu->f = cpu->f | FLAG_ZERO;
	}
	if (subtract != 0)
	{
		cpu->f = cpu->f | FLAG_SUBTRACT;
	}
	if (halfCarry != 0)
	{
		cpu->f = cpu->f | FLAG_HALF_CARRY;
	}
	if (carry != 0)
	{
		cpu->f = cpu->f | FLAG_CARRY;
	}
}

void PushWordToStack(gameboy_t* gb, uint16_t word)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t loByte = word & 0x00ff;
	uint8_t hiByte = (word >> 8) & 0x00ff;

	cpu->sp--;
	writeByteToAddress(gb, cpu->sp, loByte);
	cpu->sp--;
	writeByteToAddress(gb, cpu->sp, hiByte);
	
}

uint16_t PopWordFromStack(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t hiByte = readByteFromAddress(gb, cpu->sp++);
	uint8_t loByte = readByteFromAddress(gb, cpu->sp++);

	uint16_t value = (hiByte << 8) | (loByte);
	return value;
}

void Jump_HL(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	cpu->pc = cpu->hl;
	cpu->currentIstructionCycles = 4;
	printf("JP HL");
}

void POP_16BitReq(gameboy_t* gb, uint16_t* reg, const char* regName)
{
	cpu_t* cpu = &gb->cpu;
	uint16_t value = PopWordFromStack(gb);
	*reg = value;
	cpu->currentIstructionCycles = 12;

	printf("POP %s", regName);
}

void PUSH_16BitReq(gameboy_t* gb, uint16_t* reg, const char* regName)
{
	cpu_t* cpu = &gb->cpu;
	PushWordToStack(gb, *reg);
	cpu->currentIstructionCycles = 16;
	printf("PUSH %s", regName);
}


void ADD_16BitReg(gameboy_t* gb, uint16_t* destReg, uint16_t* sourceReg, const char* destRegName, const char* sourceRegName)
{
	cpu_t* cpu = &gb->cpu;
	uint16_t originalValue = (*destReg);
	uint16_t sourceValue = (*sourceReg);
	(*destReg) += sourceValue;

	uint8_t hcarry = (((originalValue & 0xFFF) + (sourceValue & 0xFFF)) & 0x1000) == 0x1000;
	uint8_t carry = (originalValue > (*destReg));
	setFlags(gb, (*destReg), 0, hcarry, carry);
	cpu->currentIstructionCycles = 8;

	printf("ADD %s, %s", destRegName, sourceRegName);
}

void ADD_8BitReg(gameboy_t* gb, uint8_t* reg, const char* regName)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t originalValue = cpu->a;
	cpu->a = cpu->a + *reg;
	cpu->currentIstructionCycles = 4;
	uint8_t hcarry = (((originalValue & 0xF) + (*reg & 0xF)) & 0x10) == 0x10;
	uint8_t carry = (originalValue > cpu->a);

	setFlags(gb, cpu->a, 0, hcarry, carry);
	printf("ADD %s", regName);
}

void OR_8BitReg(gameboy_t* gb, uint8_t* reg, const char* regName)
{
	cpu_t* cpu = &gb->cpu;
	cpu->a = cpu->a | *reg;
	cpu->currentIstructionCycles = 4;
	setFlags(gb, cpu->a, 0, 0, 0);
	printf("OR %s", regName);

}

void OR_PCAddress(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t value = readByteFromAddress(gb, cpu->pc);
	cpu->pc++;
	cpu->a = cpu->a | value;
	cpu->currentIstructionCycles = 8;
	setFlags(gb, cpu->a, 0, 0, 0);
	printf("OR #%02x", value);

}

void OR_HLAddress(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t value = readByteFromAddress(gb, cpu->hl);
	cpu->a = cpu->a | value;
	cpu->currentIstructionCycles = 8;
	setFlags(gb, cpu->a, 0, 0, 0);
	printf("OR (HL)");

}


void XOR_8BitReg(gameboy_t* gb, uint8_t* reg, const char* regName)
{
	cpu_t* cpu = &gb->cpu;
	cpu->a = cpu->a ^ *reg;
	setFlags(gb, cpu->a, 0, 0, 0);
	cpu->currentIstructionCycles = 4;
	printf("XOR %s", regName);

}

void SWAP_8BitReg(gameboy_t* gb, uint8_t* reg, const char* regName)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t lowNibble =  *reg;
	uint8_t highNibble = *reg;

	highNibble = (highNibble << 4) & 0xF0;
	lowNibble = (lowNibble >> 4) & 0x0F;

	*reg = highNibble | lowNibble;

	cpu->currentIstructionCycles = 8;
	setFlags(gb, *reg, 0, 0, 0);
	printf("swap %s", regName);

}

void RST(gameboy_t* gb, uint8_t jumpParam)
{
	cpu_t* cpu = &gb->cpu;
	PushWordToStack(gb, cpu->pc);
	
	cpu->pc = jumpParam;
	cpu->currentIstructionCycles = 16;
	printf("RST %x02", jumpParam);
}

void LD_RegTo16BitRegAddress(gameboy_t* gb, uint16_t addressReg, uint8_t regSource, const char* addressRegName, const char* regSourceName)
{
	cpu_t* cpu = &gb->cpu;
	writeByteToAddress(gb, addressReg, regSource);
	cpu->currentIstructionCycles = 8;
	printf("LD %s, %s", addressRegName, regSourceName);

}


void LD_16BitRegAddressToReg(gameboy_t* gb, uint8_t *regDest, uint16_t addressReg, const char* regDestName, const char* addressRegName)
{
	cpu_t* cpu = &gb->cpu;
	uint16_t value = readByteFromAddress(gb, addressReg);
	*regDest = value;
	cpu->currentIstructionCycles = 8;
	printf("LD %s, %s", regDestName, addressRegName);

}


void LD_8bitRegTo8BitReg(gameboy_t* gb, uint8_t* regDest, uint8_t* regSource, const char* regDestName, const char* regSourceName)
{
	cpu_t* cpu = &gb->cpu;
	(*regDest) = (*regSource);
	cpu->currentIstructionCycles = 4;
	printf("LD %s, %s", regDestName, regSourceName);
}

void LD_RegValueToRegAddressHigh(gameboy_t* gb, uint8_t* regVal, uint8_t* regAddress, const char* regValName, const char* regAddressName)
{
	cpu_t* cpu = &gb->cpu;
	writeByteToAddress(gb, (*regAddress + 0xff00), *regVal);
	cpu->currentIstructionCycles = 8;
	printf("LD (FF00+%s), %s", regAddressName, regValName);
}

void LD_ByteAtHLAddressToReg(gameboy_t* gb, uint8_t* reg, const char* regName)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t loByte = readByteFromAddress(gb, cpu->hl);
	*reg = loByte;
	cpu->currentIstructionCycles = 8;
	printf("LD %s, (HL+)", regName);
}

void LD_ByteAtHLAddressToRegWithInc(gameboy_t* gb, uint8_t* reg, const char* regName)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t loByte = readByteFromAddress(gb, cpu->hl);
	*reg = loByte;
	cpu->hl++;
	cpu->currentIstructionCycles = 8;
	printf("LD %s, (HL+)", regName);
}

void LD_ByteToAddress(gameboy_t* gb, uint16_t address, const char* regName)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t loByte = readByteFromAddress(gb, cpu->pc);
	writeByteToAddress(gb, address, loByte);
	cpu->currentIstructionCycles = 12;
	cpu->pc++;

	printf("LD (%s),%02X", regName, loByte);
}

void LD_ByteToReg(gameboy_t* gb, uint8_t* reg, char regName)
{
	cpu_t* cpu = &gb->cpu;

	uint8_t loByte = readByteFromAddress(gb, cpu->pc);
	cpu->pc++;
	cpu->currentIstructionCycles = 8;
	*reg = loByte;
	
	printf("LD %c,%02X", regName, loByte);
	
}

void LD_WordTo16BitReg(gameboy_t* gb, uint16_t* reg, const char* regName)
{
	cpu_t* cpu = &gb->cpu;
	uint16_t word = readWordFromAddress(gb, cpu->pc);
	cpu->pc += 2;
	*reg = word;
	cpu->currentIstructionCycles = 12;

	printf("LD %s, %04X", regName, word);
}

void DEC_8BitReg(gameboy_t* gb, uint8_t* reg, const char* regName)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t originalValue = *reg;
	(*reg)--;

	uint8_t hcarry = (((originalValue & 0xF) - (*reg & 0xF)) & 0x10) == 0x10;
	cpu->currentIstructionCycles = 4;
	setFlags(gb, *reg, 1, hcarry, isFlagSet(gb, FLAG_CARRY));
	printf("DEC %s", regName);
}

void DEC_16BitReg(gameboy_t* gb, uint16_t* reg, const char* regName)
{
	cpu_t* cpu = &gb->cpu;
	(*reg)--;
	cpu->currentIstructionCycles = 8;
	printf("DEC %s", regName);
}

void INC_16BitReg(gameboy_t* gb, uint16_t* reg, const char* regName)
{
	cpu_t* cpu = &gb->cpu;
	(*reg)++;
	cpu->currentIstructionCycles = 8;
	printf("INC %s", regName);
}

void INC_8BitReg(gameboy_t* gb, uint8_t* reg, const char* regName)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t originalValue = *reg;
	(*reg)++;

	uint8_t hcarry = (((originalValue & 0xF) + (*reg & 0xF)) & 0x10) == 0x10;
	cpu->currentIstructionCycles = 4;
	setFlags(gb, *reg, 0, hcarry, isFlagSet(gb, FLAG_CARRY));
	printf("INC %s", regName);
}

void AND_8BitReg(gameboy_t* gb, uint8_t* reg, const char* regName)
{
	cpu_t* cpu = &gb->cpu;
	cpu->a = cpu->a & *reg;
	setFlags(gb, cpu->a, 0, 1, 0);
	cpu->currentIstructionCycles = 4;
	printf("AND %s", regName);
}

void AND_FromHL(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t value = readByteFromAddress(gb, cpu->hl);
	cpu->a = cpu->a & value;
	setFlags(gb, cpu->a, 0, 1, 0);
	cpu->currentIstructionCycles = 8;
	printf("AND d8 %02X", value);
}

void AND_FromPC(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t value =  readByteFromAddress(gb, cpu->pc);
	cpu->pc++;
	cpu->a = cpu->a & value;
	setFlags(gb, cpu->a, 0, 1, 0);
	cpu->currentIstructionCycles = 8;
	printf("AND d8 %02X", value);
}

void Rotate_A_RLCA(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t origVal = cpu->a;
	uint8_t value = (origVal << 1) | (origVal >> 7);
	uint8_t carry = (origVal >> 7);
	cpu->f = carry ? FLAG_CARRY : 0;
	cpu->a = value;
	cpu->currentIstructionCycles = 4;
	printf("RLCA");
}

void Ret(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	uint16_t address = PopWordFromStack(gb);
	cpu->currentIstructionCycles = 16;
	cpu->pc = address;
	printf("RET");
}

void Call(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	uint16_t addressToJump = readWordFromAddress(gb, cpu->pc);
	uint16_t addressToPush = cpu->pc+2;
	PushWordToStack(gb, addressToPush);
	cpu->pc = addressToJump;
	cpu->currentIstructionCycles = 24;
	printf("CALL %0x4", addressToJump);
}

void CPL(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	cpu->a = ~cpu->a;
	cpu->currentIstructionCycles = 4;
	setFlag(gb, FLAG_SUBTRACT);
	setFlag(gb, FLAG_HALF_CARRY);
	printf("CPL");
}

void ADD_FromHL(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t value = readByteFromAddress(gb, cpu->hl);
	uint8_t originalValue = cpu->a;
	cpu->a = cpu->a + value;
	uint8_t hcarry = (((originalValue & 0xF) + (value & 0xF)) & 0x10) == 0x10;
	uint8_t carry = (originalValue > cpu->a);
	setFlags(gb, cpu->a, 0, hcarry, carry);
	cpu->currentIstructionCycles = 8;
	printf("ADD A,(HL)");
}

void ADD_FromPC(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t value = readByteFromAddress(gb, cpu->pc);
	cpu->pc++;
	uint8_t originalValue = cpu->a;
	cpu->a = cpu->a + value;
	uint8_t hcarry = (((originalValue & 0xF) + (value & 0xF)) & 0x10) == 0x10;
	uint8_t carry = (originalValue > cpu->a);
	setFlags(gb, cpu->a, 0, hcarry, carry);
	cpu->currentIstructionCycles = 8;
	printf("ADD A,%02X", value);
}

void ADC_Value(gameboy_t* gb, uint8_t value)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t originalValue = cpu->a;
	uint8_t carryIn = isFlagSet(gb, FLAG_CARRY) ? 1 : 0;
	uint16_t result = (uint16_t)originalValue + value + carryIn;
	uint8_t hcarry = (((originalValue & 0xF) + (value & 0xF) + carryIn) & 0x10) == 0x10;
	uint8_t carry = result > 0xFF;
	cpu->a = (uint8_t)result;
	setFlags(gb, cpu->a, 0, hcarry, carry);
	cpu->currentIstructionCycles = 4;
}

void ADC_8BitReg(gameboy_t* gb, uint8_t* reg, const char* regName)
{
	cpu_t* cpu = &gb->cpu;
	ADC_Value(gb, *reg);
	printf("ADC A,%s", regName);
}

void ADC_FromHL(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t value = readByteFromAddress(gb, cpu->hl);
	ADC_Value(gb, value);
	cpu->currentIstructionCycles = 8;
	printf("ADC A,(HL)");
}

void ADC_FromPC(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t value = readByteFromAddress(gb, cpu->pc);
	cpu->pc++;
	ADC_Value(gb, value);
	cpu->currentIstructionCycles = 8;
	printf("ADC A,%02X", value);
}

void SUB_Value(gameboy_t* gb, uint8_t value)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t originalValue = cpu->a;
	cpu->a = cpu->a - value;
	uint8_t hcarry = (((originalValue & 0xF) - (value & 0xF)) & 0x10) == 0x10;
	uint8_t carry = (originalValue < value);
	setFlags(gb, cpu->a, 1, hcarry, carry);
	cpu->currentIstructionCycles = 4;
}

void SUB_8BitReg(gameboy_t* gb, uint8_t* reg, const char* regName)
{
	cpu_t* cpu = &gb->cpu;
	SUB_Value(gb, *reg);
	printf("SUB %s", regName);
}

void SUB_FromHL(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t value = readByteFromAddress(gb, cpu->hl);
	SUB_Value(gb, value);
	cpu->currentIstructionCycles = 8;
	printf("SUB (HL)");
}

void SUB_FromPC(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t value = readByteFromAddress(gb, cpu->pc);
	cpu->pc++;
	SUB_Value(gb, value);
	cpu->currentIstructionCycles = 8;
	printf("SUB %02X", value);
}

void SBC_Value(gameboy_t* gb, uint8_t value)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t originalValue = cpu->a;
	uint8_t carryIn = isFlagSet(gb, FLAG_CARRY) ? 1 : 0;
	int16_t result = (int16_t)originalValue - value - carryIn;
	uint8_t hcarry = (((int16_t)(originalValue & 0xF) - (value & 0xF) - carryIn) & 0x10) == 0x10;
	uint8_t carry = result < 0;
	cpu->a = (uint8_t)result;
	setFlags(gb, cpu->a, 1, hcarry, carry);
	cpu->currentIstructionCycles = 4;
}

void SBC_8BitReg(gameboy_t* gb, uint8_t* reg, const char* regName)
{
	cpu_t* cpu = &gb->cpu;
	SBC_Value(gb, *reg);
	printf("SBC A,%s", regName);
}

void SBC_FromHL(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t value = readByteFromAddress(gb, cpu->hl);
	SBC_Value(gb, value);
	cpu->currentIstructionCycles = 8;
	printf("SBC A,(HL)");
}

void SBC_FromPC(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t value = readByteFromAddress(gb, cpu->pc);
	cpu->pc++;
	SBC_Value(gb, value);
	cpu->currentIstructionCycles = 8;
	printf("SBC A,%02X", value);
}

void CP_Value(gameboy_t* gb, uint8_t value)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t compare = cpu->a - value;
	uint8_t hcarry = (((cpu->a & 0xF) - (value & 0xF)) & 0x10) == 0x10;
	uint8_t carry = (cpu->a < value);
	setFlags(gb, compare, 1, hcarry, carry);
	cpu->currentIstructionCycles = 4;
}

void CP_8BitReg(gameboy_t* gb, uint8_t* reg, const char* regName)
{
	cpu_t* cpu = &gb->cpu;
	CP_Value(gb, *reg);
	printf("CP %s", regName);
}

void CP_FromHL(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t value = readByteFromAddress(gb, cpu->hl);
	CP_Value(gb, value);
	cpu->currentIstructionCycles = 8;
	printf("CP (HL)");
}

void XOR_FromHL(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t value = readByteFromAddress(gb, cpu->hl);
	cpu->a = cpu->a ^ value;
	setFlags(gb, cpu->a, 0, 0, 0);
	cpu->currentIstructionCycles = 8;
	printf("XOR (HL)");
}

void XOR_FromPC(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t value = readByteFromAddress(gb, cpu->pc);
	cpu->pc++;
	cpu->a = cpu->a ^ value;
	setFlags(gb, cpu->a, 0, 0, 0);
	cpu->currentIstructionCycles = 8;
	printf("XOR %02X", value);
}

void INC_HLAddress(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t originalValue = readByteFromAddress(gb, cpu->hl);
	uint8_t value = originalValue + 1;
	writeByteToAddress(gb, cpu->hl, value);
	uint8_t hcarry = (((originalValue & 0xF) + 1) & 0x10) == 0x10;
	setFlags(gb, value, 0, hcarry, isFlagSet(gb, FLAG_CARRY));
	cpu->currentIstructionCycles = 12;
	printf("INC (HL)");
}

void DEC_HLAddress(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t originalValue = readByteFromAddress(gb, cpu->hl);
	uint8_t value = originalValue - 1;
	writeByteToAddress(gb, cpu->hl, value);
	uint8_t hcarry = (((originalValue & 0xF) - (value & 0xF)) & 0x10) == 0x10;
	setFlags(gb, value, 1, hcarry, isFlagSet(gb, FLAG_CARRY));
	cpu->currentIstructionCycles = 12;
	printf("DEC (HL)");
}

void Rotate_A_RRCA(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t origVal = cpu->a;
	uint8_t carry = origVal & 1;
	uint8_t value = (origVal >> 1) | (carry << 7);
	cpu->a = value;
	cpu->f = carry ? FLAG_CARRY : 0;
	cpu->currentIstructionCycles = 4;
	printf("RRCA");
}

void Rotate_A_RLA(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t origVal = cpu->a;
	uint8_t oldCarry = isFlagSet(gb, FLAG_CARRY) ? 1 : 0;
	uint8_t newCarry = (origVal >> 7) & 1;
	uint8_t value = (origVal << 1) | oldCarry;
	cpu->a = value;
	cpu->f = newCarry ? FLAG_CARRY : 0;
	cpu->currentIstructionCycles = 4;
	printf("RLA");
}

void Rotate_A_RRA(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t origVal = cpu->a;
	uint8_t oldCarry = isFlagSet(gb, FLAG_CARRY) ? 1 : 0;
	uint8_t newCarry = origVal & 1;
	uint8_t value = (origVal >> 1) | (oldCarry << 7);
	cpu->a = value;
	cpu->f = newCarry ? FLAG_CARRY : 0;
	cpu->currentIstructionCycles = 4;
	printf("RRA");
}

void DAA(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t a = cpu->a;
	uint8_t adjust = 0;
	uint8_t carry = 0;
	uint8_t subtract = isFlagSet(gb, FLAG_SUBTRACT) ? 1 : 0;

	if (isFlagSet(gb, FLAG_HALF_CARRY) || (!subtract && (a & 0x0F) > 9))
	{
		adjust |= 0x06;
	}
	if (isFlagSet(gb, FLAG_CARRY) || (!subtract && a > 0x99))
	{
		adjust |= 0x60;
		carry = 1;
	}

	a = subtract ? (a - adjust) : (a + adjust);
	cpu->a = a;

	setFlags(gb, a, subtract, 0, carry);
	cpu->currentIstructionCycles = 4;
	printf("DAA");
}

void SCF(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	cpu->f &= FLAG_ZERO;
	cpu->f |= FLAG_CARRY;
	cpu->currentIstructionCycles = 4;
	printf("SCF");
}

void CCF(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t carry = isFlagSet(gb, FLAG_CARRY) ? 0 : FLAG_CARRY;
	cpu->f &= FLAG_ZERO;
	cpu->f |= carry;
	cpu->currentIstructionCycles = 4;
	printf("CCF");
}

void JumpRelative(gameboy_t* gb, uint8_t condition, const char* mnemonic)
{
	cpu_t* cpu = &gb->cpu;
	int8_t offset = (int8_t)readByteFromAddress(gb, cpu->pc);
	cpu->pc++;
	if (condition)
	{
		cpu->pc += offset;
		cpu->currentIstructionCycles = 12;
	}
	else
	{
		cpu->currentIstructionCycles = 8;
	}

	if (mnemonic[0] != '\0')
	{
		printf("JR %s,%02X", mnemonic, (uint8_t)offset);
	}
	else
	{
		printf("JR %02X", (uint8_t)offset);
	}
}

void Ret_Conditional(gameboy_t* gb, uint8_t condition, const char* mnemonic)
{
	cpu_t* cpu = &gb->cpu;
	if (condition)
	{
		cpu->pc = PopWordFromStack(gb);
		cpu->currentIstructionCycles = 20;
	}
	else
	{
		cpu->currentIstructionCycles = 8;
	}
	printf("RET %s", mnemonic);
}

void Call_Conditional(gameboy_t* gb, uint8_t condition, const char* mnemonic)
{
	cpu_t* cpu = &gb->cpu;
	uint16_t addressToJump = readWordFromAddress(gb, cpu->pc);
	cpu->pc += 2;
	if (condition)
	{
		PushWordToStack(gb, cpu->pc);
		cpu->pc = addressToJump;
		cpu->currentIstructionCycles = 24;
	}
	else
	{
		cpu->currentIstructionCycles = 12;
	}
	printf("CALL %s,%04X", mnemonic, addressToJump);
}

void Jp_Conditional(gameboy_t* gb, uint8_t condition, const char* mnemonic)
{
	cpu_t* cpu = &gb->cpu;
	uint16_t address = readWordFromAddress(gb, cpu->pc);
	cpu->pc += 2;
	if (condition)
	{
		cpu->pc = address;
		cpu->currentIstructionCycles = 16;
	}
	else
	{
		cpu->currentIstructionCycles = 12;
	}
	printf("JP %s,%04X", mnemonic, address);
}

void ADD_SP_r8(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	int8_t offset = (int8_t)readByteFromAddress(gb, cpu->pc);
	cpu->pc++;
	uint16_t sp = cpu->sp;

	uint8_t hcarry = (((sp & 0xF) + (offset & 0xF)) & 0x10) == 0x10;
	uint8_t carry = (((sp & 0xFF) + (offset & 0xFF)) & 0x100) == 0x100;

	cpu->sp = sp + offset;
	cpu->f = (hcarry ? FLAG_HALF_CARRY : 0) | (carry ? FLAG_CARRY : 0);
	cpu->currentIstructionCycles = 16;
	printf("ADD SP,%02X", (uint8_t)offset);
}

void LD_HL_SPPlusR8(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	int8_t offset = (int8_t)readByteFromAddress(gb, cpu->pc);
	cpu->pc++;
	uint16_t sp = cpu->sp;

	uint8_t hcarry = (((sp & 0xF) + (offset & 0xF)) & 0x10) == 0x10;
	uint8_t carry = (((sp & 0xFF) + (offset & 0xFF)) & 0x100) == 0x100;

	cpu->hl = sp + offset;
	cpu->f = (hcarry ? FLAG_HALF_CARRY : 0) | (carry ? FLAG_CARRY : 0);
	cpu->currentIstructionCycles = 12;
	printf("LD HL,SP+%02X", (uint8_t)offset);
}

void LD_Address_SP(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	uint16_t address = readWordFromAddress(gb, cpu->pc);
	cpu->pc += 2;
	writeWordToAddress(gb, address, cpu->sp);
	cpu->currentIstructionCycles = 20;
	printf("LD (%04X),SP", address);
}

void LD_A_FromAddress(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	uint16_t address = readWordFromAddress(gb, cpu->pc);
	cpu->pc += 2;
	cpu->a = readByteFromAddress(gb, address);
	cpu->currentIstructionCycles = 16;
	printf("LD A,(%04X)", address);
}

void LD_RegAddressHighToReg(gameboy_t* gb, uint8_t* regDest, uint8_t* regAddress, const char* regDestName, const char* regAddressName)
{
	cpu_t* cpu = &gb->cpu;
	uint8_t value = readByteFromAddress(gb, 0xff00 + *regAddress);
	*regDest = value;
	cpu->currentIstructionCycles = 8;
	printf("LD %s,(FF00+%s)", regDestName, regAddressName);
}

uint8_t ReadCBOperand(gameboy_t* gb, uint8_t regIndex)
{
	cpu_t* cpu = &gb->cpu;
	switch (regIndex)
	{
		case 0: return cpu->b;
		case 1: return cpu->c;
		case 2: return cpu->d;
		case 3: return cpu->e;
		case 4: return cpu->h;
		case 5: return cpu->l;
		case 6: return readByteFromAddress(gb, cpu->hl);
		case 7: return cpu->a;
	}
	return 0;
}

void WriteCBOperand(gameboy_t* gb, uint8_t regIndex, uint8_t value)
{
	cpu_t* cpu = &gb->cpu;
	switch (regIndex)
	{
		case 0: cpu->b = value; break;
		case 1: cpu->c = value; break;
		case 2: cpu->d = value; break;
		case 3: cpu->e = value; break;
		case 4: cpu->h = value; break;
		case 5: cpu->l = value; break;
		case 6: writeByteToAddress(gb, cpu->hl, value); break;
		case 7: cpu->a = value; break;
	}
}

const char* CBOperandName(uint8_t regIndex)
{
	static const char* names[8] = { "B", "C", "D", "E", "H", "L", "(HL)", "A" };
	return names[regIndex & 0x07];
}

void ServiceInterrupt(gameboy_t* gb, uint8_t bit, uint16_t address)
{
	cpu_t* cpu = &gb->cpu;
	cpu->ime = 0;
	gb->hardwareRegisters.interruptFlag &= ~(1 << bit);
	PushWordToStack(gb, cpu->pc);
	cpu->pc = address;
	cpu->totalCycles += 20;
	printf(" [INTERRUPT -> %04X]", address);
}

void HandleInterrupts(gameboy_t* gb)
{
	cpu_t* cpu = &gb->cpu;
	if (!cpu->ime)
	{
		return;
	}

	uint8_t pending = gb->hardwareRegisters.interruptFlag & gb->hardwareRegisters.interruptEnable;
	if (pending == 0)
	{
		return;
	}

	if (pending & 0x01)
	{
		ServiceInterrupt(gb, 0, 0x0040);
	}
	else if (pending & 0x02)
	{
		ServiceInterrupt(gb, 1, 0x0048);
	}
	else if (pending & 0x04)
	{
		ServiceInterrupt(gb, 2, 0x0050);
	}
	else if (pending & 0x08)
	{
		ServiceInterrupt(gb, 3, 0x0058);
	}
	else if (pending & 0x10)
	{
		ServiceInterrupt(gb, 4, 0x0060);
	}
}

void cpuStep(gameboy_t* gb) {

	cpu_t& cpuInstance = gb->cpu;

	cpuInstance.currentIstructionOpCode = readByteFromAddress(gb, cpuInstance.pc);
	
	if (cpuInstance.currentIstructionOpCode == 0xcb)
	{
		cpuInstance.currentIstructionCBOpCode = readByteFromAddress(gb, cpuInstance.pc+1);
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
			LD_WordTo16BitReg(gb, &cpuInstance.bc, "BC");
			break;
		}
		case 0x02:
		{
			LD_RegTo16BitRegAddress(gb, cpuInstance.bc, cpuInstance.a, "BC", "A");
			break;
		}
		case 0x03:
		{
			INC_16BitReg(gb, &cpuInstance.bc, "BC");
			break;
		}
		case 0x04:
		{
			INC_8BitReg(gb, &cpuInstance.b, "B");
			break;
		}
		case 0x05:
		{
			DEC_8BitReg(gb, &cpuInstance.b, "B");
			break;
		}
		case 0x06:
		{
			LD_ByteToReg(gb, &cpuInstance.b, 'B');
			break;
		}
		case 0x07:
		{
			Rotate_A_RLCA(gb);
			break;
		}
		case 0x08:
		{
			LD_Address_SP(gb);
			break;
		}
		case 0x09:
		{
			ADD_16BitReg(gb, &cpuInstance.hl, &cpuInstance.bc, "HL", "BC");
			break;
		}
		case 0x12:
		{
			LD_RegTo16BitRegAddress(gb, cpuInstance.de, cpuInstance.a, "DE", "A");
			break;
		}
		case 0x0a:
		{
			LD_16BitRegAddressToReg(gb, &cpuInstance.a, cpuInstance.bc, "A", "BC");
			break;
		}
		case 0x0b:
		{
			DEC_16BitReg(gb, &cpuInstance.bc, "BC");
			break;
		}
		case 0x0c:
		{
			INC_8BitReg(gb, &cpuInstance.c, "C");
			break;
		}
		case 0x0d:
		{
			DEC_8BitReg(gb, &cpuInstance.c, "C");
			break;
		}
		case 0x0e:
		{
			LD_ByteToReg(gb, &cpuInstance.c, 'C');
			break;
		}
		case 0x0f:
		{
			Rotate_A_RRCA(gb);
			break;
		}
		case 0x10:
		{
			cpuInstance.pc++;
			cpuInstance.currentIstructionCycles = 4;
			printf("STOP (unimplemented - treated as NOP)");
			break;
		}
		case 0x11:
		{
			LD_WordTo16BitReg(gb, &cpuInstance.de, "DE");
			break;
		}
		case 0x13:
		{
			INC_16BitReg(gb, &cpuInstance.de, "DE");
			break;
		}
		case 0x14:
		{
			INC_8BitReg(gb, &cpuInstance.d, "D");
			break;
		}
		case 0x15:
		{
			DEC_8BitReg(gb, &cpuInstance.d, "D");
			break;
		}
		case 0x16:
		{
			LD_ByteToReg(gb, &cpuInstance.d, 'D');
			break;
		}
		case 0x17:
		{
			Rotate_A_RLA(gb);
			break;
		}
		case 0x18:
		{
			JumpRelative(gb, 1, "");
			break;
		}
		case 0x19:
		{
			ADD_16BitReg(gb, &cpuInstance.hl, &cpuInstance.de, "HL", "DE");
			break;
		}
		case 0x1a:
		{
			LD_16BitRegAddressToReg(gb, &cpuInstance.a, cpuInstance.de, "A", "DE");
			break;
		}
		case 0x1b:
		{
			DEC_16BitReg(gb, &cpuInstance.de, "DE");
			break;
		}
		case 0x1c:
		{
			INC_8BitReg(gb, &cpuInstance.e, "E");
			break;
		}
		case 0x1d:
		{
			DEC_8BitReg(gb, &cpuInstance.e, "E");
			break;
		}
		case 0x1e:
		{
			LD_ByteToReg(gb, &cpuInstance.e, 'E');
			break;
		}
		case 0x1f:
		{
			Rotate_A_RRA(gb);
			break;
		}
		case 0x20:
		{
			JumpRelative(gb, !isFlagSet(gb, FLAG_ZERO), "NZ");
			break;
		}
		case 0x21:
		{
			LD_WordTo16BitReg(gb, &cpuInstance.hl, "HL");
			break;
		}
		case 0x23:
		{
			INC_16BitReg(gb, &cpuInstance.hl, "HL");
			break;
		}
		case 0x22:
		{
			writeByteToAddress(gb, cpuInstance.hl, cpuInstance.a);
			cpuInstance.hl++;
			cpuInstance.currentIstructionCycles = 8;
			printf("LD (HL+),A");
			break;
		}
		case 0x24:
		{
			INC_8BitReg(gb, &cpuInstance.h, "H");
			break;
		}
		case 0x25:
		{
			DEC_8BitReg(gb, &cpuInstance.h, "H");
			break;
		}
		case 0x26:
		{
			LD_ByteToReg(gb, &cpuInstance.h, 'H');
			break;
		}
		case 0x27:
		{
			DAA(gb);
			break;
		}
		case 0x28:
		{
			JumpRelative(gb, isFlagSet(gb, FLAG_ZERO), "Z");
			break;
		}
		case 0x29:
		{
			ADD_16BitReg(gb, &cpuInstance.hl, &cpuInstance.hl, "HL", "HL");
			break;
		}
		case 0x2b:
		{
			DEC_16BitReg(gb, &cpuInstance.hl, "HL");
			break;
		}
		case 0x2a:
		{
			LD_ByteAtHLAddressToRegWithInc(gb, &cpuInstance.a, "A");
			break;
		}
		case 0x2c:
		{
			INC_8BitReg(gb, &cpuInstance.l, "L");
			break;
		}
		case 0x2d:
		{
			DEC_8BitReg(gb, &cpuInstance.l, "L");
			break;
		}
		case 0x2e:
		{
			LD_ByteToReg(gb, &cpuInstance.l, 'L');
			break;
		}
		case 0x2f:
		{
			CPL(gb);
			break;
		}
		case 0x30:
		{
			JumpRelative(gb, !isFlagSet(gb, FLAG_CARRY), "NC");
			break;
		}
		case 0x31:
		{
			LD_WordTo16BitReg(gb, &cpuInstance.sp, "SP");
			break;
		}
		case 0x32:
		{
			uint16_t address = cpuInstance.hl;
			writeByteToAddress(gb, address, cpuInstance.a);
			cpuInstance.hl--;
			cpuInstance.currentIstructionCycles = 8;
			printf("LD (HL-),A");
			break;
		}
		case 0x33:
		{
			INC_16BitReg(gb, &cpuInstance.sp, "SP");
			break;
		}
		case 0x34:
		{
			INC_HLAddress(gb);
			break;
		}
		case 0x35:
		{
			DEC_HLAddress(gb);
			break;
		}
		case 0x36:
		{
			LD_ByteToAddress(gb, cpuInstance.hl, "HL");
			break;
		}
		case 0x37:
		{
			SCF(gb);
			break;
		}
		case 0x38:
		{
			JumpRelative(gb, isFlagSet(gb, FLAG_CARRY), "C");
			break;
		}
		case 0x39:
		{
			ADD_16BitReg(gb, &cpuInstance.hl, &cpuInstance.sp, "HL", "SP");
			break;
		}
		case 0x3a:
		{
			cpuInstance.a = readByteFromAddress(gb, cpuInstance.hl);
			cpuInstance.hl--;
			cpuInstance.currentIstructionCycles = 8;
			printf("LD A,(HL-)");
			break;
		}
		case 0x3b:
		{
			DEC_16BitReg(gb, &cpuInstance.sp, "SP");
			break;
		}
		case 0x3c:
		{
			INC_8BitReg(gb, &cpuInstance.a, "A");
			break;
		}
		case 0x3d:
		{
			DEC_8BitReg(gb, &cpuInstance.a, "A");
			break;
		}
		case 0x3E:
		{
			LD_ByteToReg(gb, &cpuInstance.a, 'A');
			break;
		}
		case 0x3f:
		{
			CCF(gb);
			break;
		}



		case 0x40:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.b, &cpuInstance.b, "B", "B");
			break;
		}
		case 0x41:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.b, &cpuInstance.c, "B", "C");
			break;
		}
		case 0x42:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.b, &cpuInstance.d, "B", "D");
			break;
		}
		case 0x43:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.b, &cpuInstance.e, "B", "E");
			break;
		}
		case 0x44:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.b, &cpuInstance.h, "B", "H");
			break;
		}
		case 0x45:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.b, &cpuInstance.l, "B", "L");
			break;
		}
		case 0x46:
		{
			LD_ByteAtHLAddressToReg(gb, &cpuInstance.b, "B");
			break;
		}
		case 0x47:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.b, &cpuInstance.a, "B", "A");
			break;
		}



		case 0x48:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.c, &cpuInstance.b, "C", "B");
			break;
		}
		case 0x49:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.c, &cpuInstance.c, "C", "C");
			break;
		}
		case 0x4a:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.c, &cpuInstance.d, "C", "D");
			break;
		}
		case 0x4b:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.c, &cpuInstance.e, "C", "E");
			break;
		}
		case 0x4c:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.c, &cpuInstance.h, "C", "H");
			break;
		}
		case 0x4d:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.c, &cpuInstance.l, "C", "L");
			break;
		}
		case 0x4e:
		{
			LD_ByteAtHLAddressToReg(gb, &cpuInstance.c, "C");
			break;
		}
		case 0x4f:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.c, &cpuInstance.a, "C", "A");
			break;
		}



		case 0x50:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.d, &cpuInstance.b, "D", "B");
			break;
		}
		case 0x51:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.d, &cpuInstance.c, "D", "C");
			break;
		}
		case 0x52:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.d, &cpuInstance.d, "D", "D");
			break;
		}
		case 0x53:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.d, &cpuInstance.e, "D", "E");
			break;
		}
		case 0x54:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.d, &cpuInstance.h, "D", "H");
			break;
		}
		case 0x55:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.d, &cpuInstance.l, "D", "L");
			break;
		}
		case 0x56:
		{
			LD_ByteAtHLAddressToReg(gb, &cpuInstance.d, "D");
			break;
		}
		case 0x57:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.d, &cpuInstance.a, "D", "A");
			break;
		}



		case 0x58:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.e, &cpuInstance.b, "E", "B");
			break;
		}
		case 0x59:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.e, &cpuInstance.c, "E", "C");
			break;
		}
		case 0x5a:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.e, &cpuInstance.d, "E", "D");
			break;
		}
		case 0x5b:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.e, &cpuInstance.e, "E", "E");
			break;
		}
		case 0x5c:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.e, &cpuInstance.h, "E", "H");
			break;
		}
		case 0x5d:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.e, &cpuInstance.l, "E", "L");
			break;
		}
		case 0x5e:
		{
			LD_ByteAtHLAddressToReg(gb, &cpuInstance.e, "E");
			break;
		}
		case 0x5f:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.e, &cpuInstance.a, "E", "A");
			break;
		}



		case 0x60:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.h, &cpuInstance.b, "H", "B");
			break;
		}
		case 0x61:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.h, &cpuInstance.c, "H", "C");
			break;
		}
		case 0x62:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.h, &cpuInstance.d, "H", "D");
			break;
		}
		case 0x63:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.h, &cpuInstance.e, "H", "E");
			break;
		}
		case 0x64:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.h, &cpuInstance.h, "H", "H");
			break;
		}
		case 0x65:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.h, &cpuInstance.l, "H", "L");
			break;
		}
		case 0x66:
		{
			LD_ByteAtHLAddressToReg(gb, &cpuInstance.h, "H");
			break;
		}
		case 0x67:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.h, &cpuInstance.a, "H", "A");
			break;
		}



		case 0x68:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.l, &cpuInstance.b, "L", "B");
			break;
		}
		case 0x69:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.l, &cpuInstance.c, "L", "C");
			break;
		}
		case 0x6a:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.l, &cpuInstance.d, "L", "D");
			break;
		}
		case 0x6b:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.l, &cpuInstance.e, "L", "E");
			break;
		}
		case 0x6c:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.l, &cpuInstance.h, "L", "H");
			break;
		}
		case 0x6d:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.l, &cpuInstance.l, "L", "L");
			break;
		}
		case 0x6e:
		{
			LD_ByteAtHLAddressToReg(gb, &cpuInstance.l, "L");
			break;
		}
		case 0x6f:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.l, &cpuInstance.a, "L", "A");
			break;
		}


		case 0x70:
		{
			LD_RegTo16BitRegAddress(gb, cpuInstance.hl, cpuInstance.b, "HL", "B");
			break;
		}
		case 0x71:
		{
			LD_RegTo16BitRegAddress(gb, cpuInstance.hl, cpuInstance.c, "HL", "C");
			break;
		}
		case 0x72:
		{
			LD_RegTo16BitRegAddress(gb, cpuInstance.hl, cpuInstance.d, "HL", "D");
			break;
		}
		case 0x73:
		{
			LD_RegTo16BitRegAddress(gb, cpuInstance.hl, cpuInstance.e, "HL", "E");
			break;
		}
		case 0x74:
		{
			LD_RegTo16BitRegAddress(gb, cpuInstance.hl, cpuInstance.h, "HL", "H");
			break;
		}
		case 0x75:
		{
			LD_RegTo16BitRegAddress(gb, cpuInstance.hl, cpuInstance.l, "HL", "L");
			break;
		}
		case 0x76:
		{
			cpuInstance.currentIstructionCycles = 4;
			printf("HALT (unimplemented - treated as NOP)");
			break;
		}
		case 0x77:
		{
			LD_RegTo16BitRegAddress(gb, cpuInstance.hl, cpuInstance.a, "HL", "A");
			break;
		}
		case 0x78:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.a, &cpuInstance.b, "A", "B");
			break;
		}
		case 0x79:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.a, &cpuInstance.c, "A", "C");
			break;
		}
		case 0x7a:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.a, &cpuInstance.d, "A", "D");
			break;
		}
		case 0x7b:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.a, &cpuInstance.e, "A", "E");
			break;
		}
		case 0x7c:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.a, &cpuInstance.h, "A", "H");
			break;
		}
		case 0x7d:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.a, &cpuInstance.l, "A", "L");
			break;
		}
		case 0x7e:
		{
			LD_ByteAtHLAddressToReg(gb, &cpuInstance.a, "A");
			break;
		}
	
		case 0x7f:
		{
			LD_8bitRegTo8BitReg(gb, &cpuInstance.a, &cpuInstance.a,  "A", "A");
			break;
		}

		case 0x80:
		{
			ADD_8BitReg(gb, &cpuInstance.b, "B");
			break;
		}
		case 0x81:
		{
			ADD_8BitReg(gb, &cpuInstance.c, "C");
			break;
		}
		case 0x82:
		{
			ADD_8BitReg(gb, &cpuInstance.d, "D");
			break;
		}
		case 0x83:
		{
			ADD_8BitReg(gb, &cpuInstance.e, "E");
			break;
		}
		case 0x84:
		{
			ADD_8BitReg(gb, &cpuInstance.h, "H");
			break;
		}
		case 0x85:
		{
			ADD_8BitReg(gb, &cpuInstance.l, "L");
			break;
		}
		case 0x86:
		{
			ADD_FromHL(gb);
			break;
		}
		case 0x87:
		{
			ADD_8BitReg(gb, &cpuInstance.a, "A");
			break;
		}
		case 0x88:
		{
			ADC_8BitReg(gb, &cpuInstance.b, "B");
			break;
		}
		case 0x89:
		{
			ADC_8BitReg(gb, &cpuInstance.c, "C");
			break;
		}
		case 0x8a:
		{
			ADC_8BitReg(gb, &cpuInstance.d, "D");
			break;
		}
		case 0x8b:
		{
			ADC_8BitReg(gb, &cpuInstance.e, "E");
			break;
		}
		case 0x8c:
		{
			ADC_8BitReg(gb, &cpuInstance.h, "H");
			break;
		}
		case 0x8d:
		{
			ADC_8BitReg(gb, &cpuInstance.l, "L");
			break;
		}
		case 0x8e:
		{
			ADC_FromHL(gb);
			break;
		}
		case 0x8f:
		{
			ADC_8BitReg(gb, &cpuInstance.a, "A");
			break;
		}
		case 0x90:
		{
			SUB_8BitReg(gb, &cpuInstance.b, "B");
			break;
		}
		case 0x91:
		{
			SUB_8BitReg(gb, &cpuInstance.c, "C");
			break;
		}
		case 0x92:
		{
			SUB_8BitReg(gb, &cpuInstance.d, "D");
			break;
		}
		case 0x93:
		{
			SUB_8BitReg(gb, &cpuInstance.e, "E");
			break;
		}
		case 0x94:
		{
			SUB_8BitReg(gb, &cpuInstance.h, "H");
			break;
		}
		case 0x95:
		{
			SUB_8BitReg(gb, &cpuInstance.l, "L");
			break;
		}
		case 0x96:
		{
			SUB_FromHL(gb);
			break;
		}
		case 0x97:
		{
			SUB_8BitReg(gb, &cpuInstance.a, "A");
			break;
		}
		case 0x98:
		{
			SBC_8BitReg(gb, &cpuInstance.b, "B");
			break;
		}
		case 0x99:
		{
			SBC_8BitReg(gb, &cpuInstance.c, "C");
			break;
		}
		case 0x9a:
		{
			SBC_8BitReg(gb, &cpuInstance.d, "D");
			break;
		}
		case 0x9b:
		{
			SBC_8BitReg(gb, &cpuInstance.e, "E");
			break;
		}
		case 0x9c:
		{
			SBC_8BitReg(gb, &cpuInstance.h, "H");
			break;
		}
		case 0x9d:
		{
			SBC_8BitReg(gb, &cpuInstance.l, "L");
			break;
		}
		case 0x9e:
		{
			SBC_FromHL(gb);
			break;
		}
		case 0x9f:
		{
			SBC_8BitReg(gb, &cpuInstance.a, "A");
			break;
		}


		case 0xa0:
		{
			AND_8BitReg(gb, &cpuInstance.b, "B");
			break;
		}
		case 0xa1:
		{
			AND_8BitReg(gb, &cpuInstance.c, "C");
			break;
		}
		case 0xa2:
		{
			AND_8BitReg(gb, &cpuInstance.d, "D");
			break;
		}
		case 0xa3:
		{
			AND_8BitReg(gb, &cpuInstance.e, "E");
			break;
		}
		case 0xa4:
		{
			AND_8BitReg(gb, &cpuInstance.h, "H");
			break;
		}
		case 0xa5:
		{
			AND_8BitReg(gb, &cpuInstance.l, "L");
			break;
		}
		case 0xa6:
		{
			AND_FromHL(gb);
			break;
		}
		case 0xa7:
		{
			AND_8BitReg(gb, &cpuInstance.a, "A");
			break;
		}
		case 0xa8:
		{
			XOR_8BitReg(gb, &cpuInstance.b, "B");
			break;
		}
		case 0xa9:
		{
			XOR_8BitReg(gb, &cpuInstance.c, "C");
			break;
		}
		case 0xaa:
		{
			XOR_8BitReg(gb, &cpuInstance.d, "D");
			break;
		}
		case 0xab:
		{
			XOR_8BitReg(gb, &cpuInstance.e, "E");
			break;
		}
		case 0xac:
		{
			XOR_8BitReg(gb, &cpuInstance.h, "H");
			break;
		}
		case 0xad:
		{
			XOR_8BitReg(gb, &cpuInstance.l, "L");
			break;
		}
		case 0xae:
		{
			XOR_FromHL(gb);
			break;
		}
		case 0xaf:
		{
			XOR_8BitReg(gb, &cpuInstance.a, "A");
			break;
		}

		case 0xb0:
		{
			OR_8BitReg(gb, &cpuInstance.b, "B");
			break;
		}
		case 0xb1:
		{
			OR_8BitReg(gb, &cpuInstance.c, "C");
			break;
		}
		case 0xb2:
		{
			OR_8BitReg(gb, &cpuInstance.d, "D");
			break;
		}
		case 0xb3:
		{
			OR_8BitReg(gb, &cpuInstance.e, "E");
			break;
		}
		case 0xb4:
		{
			OR_8BitReg(gb, &cpuInstance.h, "H");
			break;
		}
		case 0xb5:
		{
			OR_8BitReg(gb, &cpuInstance.l, "L");
			break;
		}
		case 0xb6:
		{
			OR_HLAddress(gb);
			break;
		}
		case 0xb7:
		{
			OR_8BitReg(gb, &cpuInstance.a, "A");
			break;
		}
		case 0xb8:
		{
			CP_8BitReg(gb, &cpuInstance.b, "B");
			break;
		}
		case 0xb9:
		{
			CP_8BitReg(gb, &cpuInstance.c, "C");
			break;
		}
		case 0xba:
		{
			CP_8BitReg(gb, &cpuInstance.d, "D");
			break;
		}
		case 0xbb:
		{
			CP_8BitReg(gb, &cpuInstance.e, "E");
			break;
		}
		case 0xbc:
		{
			CP_8BitReg(gb, &cpuInstance.h, "H");
			break;
		}
		case 0xbd:
		{
			CP_8BitReg(gb, &cpuInstance.l, "L");
			break;
		}
		case 0xbe:
		{
			CP_FromHL(gb);
			break;
		}
		case 0xbf:
		{
			CP_8BitReg(gb, &cpuInstance.a, "A");
			break;
		}
		case 0xc0:
		{
			Ret_Conditional(gb, !isFlagSet(gb, FLAG_ZERO), "NZ");
			break;
		}
		case 0xc1:
		{
			POP_16BitReq(gb, &cpuInstance.bc, "BC");
			break;
		}
		case 0xc2:
		{
			Jp_Conditional(gb, !isFlagSet(gb, FLAG_ZERO), "NZ");
			break;
		}
		case 0xc3:
		{

			uint16_t address = readWordFromAddress(gb, cpuInstance.pc);
			cpuInstance.pc += 2;
			jump(gb, address);

			cpuInstance.currentIstructionCycles = 16;
			printf("jp %04X", address);
			break;
		}
		case 0xc4:
		{
			Call_Conditional(gb, !isFlagSet(gb, FLAG_ZERO), "NZ");
			break;
		}
		case 0xc5:
		{
			PUSH_16BitReq(gb, &cpuInstance.bc, "BC");
			break;
		}
		case 0xc6:
		{
			ADD_FromPC(gb);
			break;
		}
		case 0xc7:
		{
			RST(gb, 0x00);
			break;
		}
		case 0xc8:
		{
			Ret_Conditional(gb, isFlagSet(gb, FLAG_ZERO), "Z");
			break;
		}
		case 0xc9:
		{
			Ret(gb);
			break;
		}
		case 0xca:
		{
			Jp_Conditional(gb, isFlagSet(gb, FLAG_ZERO), "Z");
			break;
		}
		case 0xcb:
		{
			uint8_t cb = cpuInstance.currentIstructionCBOpCode;
			uint8_t regIndex = cb & 0x07;
			uint8_t opGroup = cb >> 6;
			uint8_t bitOrSub = (cb >> 3) & 0x07;
			const char* regName = CBOperandName(regIndex);
			uint8_t value = ReadCBOperand(gb, regIndex);
			uint8_t isHL = (regIndex == 6);

			if (opGroup == 1)
			{
				uint8_t bitSet = value & (1 << bitOrSub);
				setFlags(gb, bitSet, 0, 1, isFlagSet(gb, FLAG_CARRY));
				cpuInstance.currentIstructionCycles = isHL ? 12 : 8;
				printf("BIT %d,%s", bitOrSub, regName);
			}
			else if (opGroup == 2)
			{
				value &= ~(1 << bitOrSub);
				WriteCBOperand(gb, regIndex, value);
				cpuInstance.currentIstructionCycles = isHL ? 16 : 8;
				printf("RES %d,%s", bitOrSub, regName);
			}
			else if (opGroup == 3)
			{
				value |= (1 << bitOrSub);
				WriteCBOperand(gb, regIndex, value);
				cpuInstance.currentIstructionCycles = isHL ? 16 : 8;
				printf("SET %d,%s", bitOrSub, regName);
			}
			else
			{
				uint8_t result = 0;
				uint8_t carryOut = 0;
				uint8_t oldCarry = isFlagSet(gb, FLAG_CARRY) ? 1 : 0;

				switch (bitOrSub)
				{
					case 0:
						carryOut = (value >> 7) & 1;
						result = (value << 1) | carryOut;
						printf("RLC %s", regName);
						break;
					case 1:
						carryOut = value & 1;
						result = (value >> 1) | (carryOut << 7);
						printf("RRC %s", regName);
						break;
					case 2:
						carryOut = (value >> 7) & 1;
						result = (value << 1) | oldCarry;
						printf("RL %s", regName);
						break;
					case 3:
						carryOut = value & 1;
						result = (value >> 1) | (oldCarry << 7);
						printf("RR %s", regName);
						break;
					case 4:
						carryOut = (value >> 7) & 1;
						result = value << 1;
						printf("SLA %s", regName);
						break;
					case 5:
						carryOut = value & 1;
						result = (value >> 1) | (value & 0x80);
						printf("SRA %s", regName);
						break;
					case 6:
						carryOut = 0;
						result = ((value << 4) & 0xF0) | ((value >> 4) & 0x0F);
						printf("SWAP %s", regName);
						break;
					case 7:
						carryOut = value & 1;
						result = value >> 1;
						printf("SRL %s", regName);
						break;
				}

				setFlags(gb, result, 0, 0, carryOut);
				WriteCBOperand(gb, regIndex, result);
				cpuInstance.currentIstructionCycles = isHL ? 16 : 8;
			}
			break;
		}
		case 0xcc:
		{
			Call_Conditional(gb, isFlagSet(gb, FLAG_ZERO), "Z");
			break;
		}
		case 0xcd:
		{
			Call(gb);
			break;
		}
		case 0xce:
		{
			ADC_FromPC(gb);
			break;
		}
		case 0xcf:
		{
			RST(gb, 0x08);
			break;
		}
		case 0xd0:
		{
			Ret_Conditional(gb, !isFlagSet(gb, FLAG_CARRY), "NC");
			break;
		}
		case 0xd1:
		{
			POP_16BitReq(gb, &cpuInstance.de, "DE");
			break;
		}
		case 0xd2:
		{
			Jp_Conditional(gb, !isFlagSet(gb, FLAG_CARRY), "NC");
			break;
		}
		case 0xd4:
		{
			Call_Conditional(gb, !isFlagSet(gb, FLAG_CARRY), "NC");
			break;
		}
		case 0xd5:
		{
			PUSH_16BitReq(gb, &cpuInstance.de, "DE");
			break;
		}
		case 0xd6:
		{
			SUB_FromPC(gb);
			break;
		}
		case 0xd7:
		{
			RST(gb, 0x10);
			break;
		}
		case 0xd8:
		{
			Ret_Conditional(gb, isFlagSet(gb, FLAG_CARRY), "C");
			break;
		}
		case 0xd9:
		{
			Ret(gb);
			cpuInstance.ime = 1;
			printf(" (RETI)");
			break;
		}
		case 0xda:
		{
			Jp_Conditional(gb, isFlagSet(gb, FLAG_CARRY), "C");
			break;
		}
		case 0xdc:
		{
			Call_Conditional(gb, isFlagSet(gb, FLAG_CARRY), "C");
			break;
		}
		case 0xde:
		{
			SBC_FromPC(gb);
			break;
		}
		case 0xdf:
		{
			RST(gb, 0x18);
			break;
		}
		case 0xE0:
		{
			uint8_t addressOffset = readByteFromAddress(gb, cpuInstance.pc);
			cpuInstance.pc++;

			writeByteToAddress(gb, (addressOffset + 0xff00), cpuInstance.a);

			cpuInstance.currentIstructionCycles = 12;
			printf("LDH ($FF00+%04X), A", addressOffset);
			break;
		}
		case 0xe1:
		{
			POP_16BitReq(gb, &cpuInstance.hl, "HL");
			break;
		}
		case 0xE2:
		{
			LD_RegValueToRegAddressHigh(gb, &cpuInstance.a, &cpuInstance.c, "A", "C");
			break;
		}
		case 0xe5:
		{
			PUSH_16BitReq(gb, &cpuInstance.hl, "HL");
			break;
		}
		case 0xEa:
		{
			uint16_t address = readWordFromAddress(gb, cpuInstance.pc);
			cpuInstance.pc +=2;
			writeByteToAddress(gb, address, cpuInstance.a);

			cpuInstance.currentIstructionCycles = 16;
			printf("LD %04X, A", address);
			break;
		}
		case 0xE6:
		{
			AND_FromPC(gb);
			break;
		}
		case 0xE8:
		{
			ADD_SP_r8(gb);
			break;
		}
		case 0xE9:
		{
			Jump_HL(gb);
			break;
		}
		case 0xEe:
		{
			XOR_FromPC(gb);
			break;
		}
		case 0xEf:
		{
			RST(gb, 0x28);
			break;
		}
		case 0xE7:
		{
			RST(gb, 0x20);
			break;
		}
		case 0xf0:
		{
			uint8_t address = readByteFromAddress(gb, cpuInstance.pc);
			cpuInstance.pc++;
			uint8_t value = readByteFromAddress(gb, address + 0xff00);
			cpuInstance.a = value;
			cpuInstance.currentIstructionCycles = 12;
			printf("LDH A,($FF00+%04X)", address);
			break;
		}
		case 0xf1:
		{
			POP_16BitReq(gb, &cpuInstance.af, "AF");
			break;
		}
		case 0xf2:
		{
			LD_RegAddressHighToReg(gb, &cpuInstance.a, &cpuInstance.c, "A", "C");
			break;
		}
		case 0xf3:
		{
			cpuInstance.ime = 0;
			printf("DI");
			cpuInstance.currentIstructionCycles = 4;
			break;
		}
		case 0xf5:
		{
			PUSH_16BitReq(gb, &cpuInstance.af, "AF");
			break;
		}
		case 0xf7:
		{
			RST(gb, 0x30);
			break;
		}
		case 0xf8:
		{
			LD_HL_SPPlusR8(gb);
			break;
		}
		case 0xf9:
		{
			cpuInstance.sp = cpuInstance.hl;
			cpuInstance.currentIstructionCycles = 8;
			printf("LD SP,HL");
			break;
		}
		case 0xfa:
		{
			LD_A_FromAddress(gb);
			break;
		}
		case 0xfb:
		{
			cpuInstance.ime = 1;
			printf("EI");
			cpuInstance.currentIstructionCycles = 4;
			break;
		}
		case 0xf6:
		{
			OR_PCAddress(gb);
			break;
		}
		case 0xfe:
		{

			uint8_t value = readByteFromAddress(gb, cpuInstance.pc);
			uint8_t compare = cpuInstance.a - value;

			uint8_t hcarry = (((cpuInstance.a & 0xF) - (compare & 0xF)) & 0x10) == 0x10;
			uint8_t carry = (cpuInstance.a < value);
			setFlags(gb, compare, 1, hcarry, carry);

			cpuInstance.pc++;
			cpuInstance.currentIstructionCycles = 8;
			printf("CP %02X", value);
			break;
		}
		case 0xff:
		{
			RST(gb, 0x38);
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
	timerStep(&gb->timer, &gb->hardwareRegisters, cpuInstance.currentIstructionCycles);
	HandleInterrupts(gb);

	printf("\tA:%02X B:%02X C:%02X D:%02X E:%02X F:%02X HL:%04X SP:%04X\n", cpuInstance.a, cpuInstance.b, cpuInstance.c, cpuInstance.d, cpuInstance.e, cpuInstance.f, cpuInstance.hl, cpuInstance.sp);

}

