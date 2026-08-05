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
	cpuInstance.f = cpuInstance.f | flag;
}

void resetFlag(uint8_t flag)
{
	cpuInstance.f = cpuInstance.f & ~flag;
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

void Jump_HL(cpu_t* cpu)
{
	cpu->pc = cpu->hl;
	cpuInstance.currentIstructionCycles = 4;
	printf("JP HL");
}

void POP_16BitReq(cpu_t* cpu, uint16_t* reg, const char* regName)
{
	uint16_t value = PopWordFromStack(cpu);
	*reg = value;
	cpuInstance.currentIstructionCycles = 12;

	printf("POP %s", regName);
}

void PUSH_16BitReq(cpu_t* cpu, uint16_t* reg, const char* regName)
{
	PushWordToStack(cpu, *reg);
	cpuInstance.currentIstructionCycles = 16;
	printf("PUSH %s", regName);
}


void ADD_16BitReg(cpu_t* cpu, uint16_t* destReg, uint16_t* sourceReg, const char* destRegName, const char* sourceRegName)
{
	uint16_t originalValue = (*destReg);
	uint16_t sourceValue = (*sourceReg);
	(*destReg) += sourceValue;

	uint8_t hcarry = (((originalValue & 0xFFF) + (sourceValue & 0xFFF)) & 0x1000) == 0x1000;
	uint8_t carry = (originalValue > (*destReg));
	setFlags((*destReg), 0, hcarry, carry);
	cpuInstance.currentIstructionCycles = 8;

	printf("ADD %s, %s", destRegName, sourceRegName);
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
	lowNibble = (lowNibble >> 4) & 0x0F;

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

void LD_RegTo16BitRegAddress(cpu_t* cpu, uint16_t addressReg, uint8_t regSource, const char* addressRegName, const char* regSourceName)
{
	writeByteToAddress(addressReg, regSource);
	cpuInstance.currentIstructionCycles = 8;
	printf("LD %s, %s", addressRegName, regSourceName);

}


void LD_16BitRegAddressToReg(cpu_t* cpu, uint8_t *regDest, uint16_t addressReg, const char* regDestName, const char* addressRegName)
{
	uint16_t value = readByteFromAddress(addressReg);
	*regDest = value;
	cpuInstance.currentIstructionCycles = 8;
	printf("LD %s, %s", regDestName, addressRegName);

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
	setFlags(*reg, 1, hcarry, isFlagSet(FLAG_CARRY));
	printf("DEC %s", regName);
}

void DEC_16BitReg(cpu_t* cpu, uint16_t* reg, const char* regName)
{
	(*reg)--;
	cpu->currentIstructionCycles = 8;
	printf("DEC %s", regName);
}

void INC_16BitReg(cpu_t* cpu, uint16_t* reg, const char* regName)
{
	(*reg)++;
	cpu->currentIstructionCycles = 8;
	printf("INC %s", regName);
}

void INC_8BitReg(cpu_t* cpu, uint8_t* reg, const char* regName)
{
	uint8_t originalValue = *reg;
	(*reg)++;

	uint8_t hcarry = (((originalValue & 0xF) + (*reg & 0xF)) & 0x10) == 0x10;
	cpu->currentIstructionCycles = 4;
	setFlags(*reg, 0, hcarry, isFlagSet(FLAG_CARRY));
	printf("INC %s", regName);
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

void Rotate_A_RLCA(cpu_t* cpu)
{	uint8_t origVal = cpu->a;
	uint8_t value = (origVal << 1) | (origVal >> 7);
	uint8_t carry = (origVal >> 7);
	cpu->f = carry ? FLAG_CARRY : 0;
	cpu->a = value;
	cpu->currentIstructionCycles = 4;
	printf("RLCA");
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

void ADD_FromHL(cpu_t* cpu)
{
	uint8_t value = readByteFromAddress(cpu->hl);
	uint8_t originalValue = cpu->a;
	cpu->a = cpu->a + value;
	uint8_t hcarry = (((originalValue & 0xF) + (value & 0xF)) & 0x10) == 0x10;
	uint8_t carry = (originalValue > cpu->a);
	setFlags(cpu->a, 0, hcarry, carry);
	cpu->currentIstructionCycles = 8;
	printf("ADD A,(HL)");
}

void ADD_FromPC(cpu_t* cpu)
{
	uint8_t value = readByteFromAddress(cpu->pc);
	cpu->pc++;
	uint8_t originalValue = cpu->a;
	cpu->a = cpu->a + value;
	uint8_t hcarry = (((originalValue & 0xF) + (value & 0xF)) & 0x10) == 0x10;
	uint8_t carry = (originalValue > cpu->a);
	setFlags(cpu->a, 0, hcarry, carry);
	cpu->currentIstructionCycles = 8;
	printf("ADD A,%02X", value);
}

void ADC_Value(cpu_t* cpu, uint8_t value)
{
	uint8_t originalValue = cpu->a;
	uint8_t carryIn = isFlagSet(FLAG_CARRY) ? 1 : 0;
	uint16_t result = (uint16_t)originalValue + value + carryIn;
	uint8_t hcarry = (((originalValue & 0xF) + (value & 0xF) + carryIn) & 0x10) == 0x10;
	uint8_t carry = result > 0xFF;
	cpu->a = (uint8_t)result;
	setFlags(cpu->a, 0, hcarry, carry);
	cpu->currentIstructionCycles = 4;
}

void ADC_8BitReg(cpu_t* cpu, uint8_t* reg, const char* regName)
{
	ADC_Value(cpu, *reg);
	printf("ADC A,%s", regName);
}

void ADC_FromHL(cpu_t* cpu)
{
	uint8_t value = readByteFromAddress(cpu->hl);
	ADC_Value(cpu, value);
	cpu->currentIstructionCycles = 8;
	printf("ADC A,(HL)");
}

void ADC_FromPC(cpu_t* cpu)
{
	uint8_t value = readByteFromAddress(cpu->pc);
	cpu->pc++;
	ADC_Value(cpu, value);
	cpu->currentIstructionCycles = 8;
	printf("ADC A,%02X", value);
}

void SUB_Value(cpu_t* cpu, uint8_t value)
{
	uint8_t originalValue = cpu->a;
	cpu->a = cpu->a - value;
	uint8_t hcarry = (((originalValue & 0xF) - (value & 0xF)) & 0x10) == 0x10;
	uint8_t carry = (originalValue < value);
	setFlags(cpu->a, 1, hcarry, carry);
	cpu->currentIstructionCycles = 4;
}

void SUB_8BitReg(cpu_t* cpu, uint8_t* reg, const char* regName)
{
	SUB_Value(cpu, *reg);
	printf("SUB %s", regName);
}

void SUB_FromHL(cpu_t* cpu)
{
	uint8_t value = readByteFromAddress(cpu->hl);
	SUB_Value(cpu, value);
	cpu->currentIstructionCycles = 8;
	printf("SUB (HL)");
}

void SUB_FromPC(cpu_t* cpu)
{
	uint8_t value = readByteFromAddress(cpu->pc);
	cpu->pc++;
	SUB_Value(cpu, value);
	cpu->currentIstructionCycles = 8;
	printf("SUB %02X", value);
}

void SBC_Value(cpu_t* cpu, uint8_t value)
{
	uint8_t originalValue = cpu->a;
	uint8_t carryIn = isFlagSet(FLAG_CARRY) ? 1 : 0;
	int16_t result = (int16_t)originalValue - value - carryIn;
	uint8_t hcarry = (((int16_t)(originalValue & 0xF) - (value & 0xF) - carryIn) & 0x10) == 0x10;
	uint8_t carry = result < 0;
	cpu->a = (uint8_t)result;
	setFlags(cpu->a, 1, hcarry, carry);
	cpu->currentIstructionCycles = 4;
}

void SBC_8BitReg(cpu_t* cpu, uint8_t* reg, const char* regName)
{
	SBC_Value(cpu, *reg);
	printf("SBC A,%s", regName);
}

void SBC_FromHL(cpu_t* cpu)
{
	uint8_t value = readByteFromAddress(cpu->hl);
	SBC_Value(cpu, value);
	cpu->currentIstructionCycles = 8;
	printf("SBC A,(HL)");
}

void SBC_FromPC(cpu_t* cpu)
{
	uint8_t value = readByteFromAddress(cpu->pc);
	cpu->pc++;
	SBC_Value(cpu, value);
	cpu->currentIstructionCycles = 8;
	printf("SBC A,%02X", value);
}

void CP_Value(cpu_t* cpu, uint8_t value)
{
	uint8_t compare = cpu->a - value;
	uint8_t hcarry = (((cpu->a & 0xF) - (value & 0xF)) & 0x10) == 0x10;
	uint8_t carry = (cpu->a < value);
	setFlags(compare, 1, hcarry, carry);
	cpu->currentIstructionCycles = 4;
}

void CP_8BitReg(cpu_t* cpu, uint8_t* reg, const char* regName)
{
	CP_Value(cpu, *reg);
	printf("CP %s", regName);
}

void CP_FromHL(cpu_t* cpu)
{
	uint8_t value = readByteFromAddress(cpu->hl);
	CP_Value(cpu, value);
	cpu->currentIstructionCycles = 8;
	printf("CP (HL)");
}

void XOR_FromHL(cpu_t* cpu)
{
	uint8_t value = readByteFromAddress(cpu->hl);
	cpu->a = cpu->a ^ value;
	setFlags(cpu->a, 0, 0, 0);
	cpu->currentIstructionCycles = 8;
	printf("XOR (HL)");
}

void XOR_FromPC(cpu_t* cpu)
{
	uint8_t value = readByteFromAddress(cpu->pc);
	cpu->pc++;
	cpu->a = cpu->a ^ value;
	setFlags(cpu->a, 0, 0, 0);
	cpu->currentIstructionCycles = 8;
	printf("XOR %02X", value);
}

void INC_HLAddress(cpu_t* cpu)
{
	uint8_t originalValue = readByteFromAddress(cpu->hl);
	uint8_t value = originalValue + 1;
	writeByteToAddress(cpu->hl, value);
	uint8_t hcarry = (((originalValue & 0xF) + 1) & 0x10) == 0x10;
	setFlags(value, 0, hcarry, isFlagSet(FLAG_CARRY));
	cpu->currentIstructionCycles = 12;
	printf("INC (HL)");
}

void DEC_HLAddress(cpu_t* cpu)
{
	uint8_t originalValue = readByteFromAddress(cpu->hl);
	uint8_t value = originalValue - 1;
	writeByteToAddress(cpu->hl, value);
	uint8_t hcarry = (((originalValue & 0xF) - (value & 0xF)) & 0x10) == 0x10;
	setFlags(value, 1, hcarry, isFlagSet(FLAG_CARRY));
	cpu->currentIstructionCycles = 12;
	printf("DEC (HL)");
}

void Rotate_A_RRCA(cpu_t* cpu)
{
	uint8_t origVal = cpu->a;
	uint8_t carry = origVal & 1;
	uint8_t value = (origVal >> 1) | (carry << 7);
	cpu->a = value;
	cpu->f = carry ? FLAG_CARRY : 0;
	cpu->currentIstructionCycles = 4;
	printf("RRCA");
}

void Rotate_A_RLA(cpu_t* cpu)
{
	uint8_t origVal = cpu->a;
	uint8_t oldCarry = isFlagSet(FLAG_CARRY) ? 1 : 0;
	uint8_t newCarry = (origVal >> 7) & 1;
	uint8_t value = (origVal << 1) | oldCarry;
	cpu->a = value;
	cpu->f = newCarry ? FLAG_CARRY : 0;
	cpu->currentIstructionCycles = 4;
	printf("RLA");
}

void Rotate_A_RRA(cpu_t* cpu)
{
	uint8_t origVal = cpu->a;
	uint8_t oldCarry = isFlagSet(FLAG_CARRY) ? 1 : 0;
	uint8_t newCarry = origVal & 1;
	uint8_t value = (origVal >> 1) | (oldCarry << 7);
	cpu->a = value;
	cpu->f = newCarry ? FLAG_CARRY : 0;
	cpu->currentIstructionCycles = 4;
	printf("RRA");
}

void DAA(cpu_t* cpu)
{
	uint8_t a = cpu->a;
	uint8_t adjust = 0;
	uint8_t carry = 0;
	uint8_t subtract = isFlagSet(FLAG_SUBTRACT) ? 1 : 0;

	if (isFlagSet(FLAG_HALF_CARRY) || (!subtract && (a & 0x0F) > 9))
	{
		adjust |= 0x06;
	}
	if (isFlagSet(FLAG_CARRY) || (!subtract && a > 0x99))
	{
		adjust |= 0x60;
		carry = 1;
	}

	a = subtract ? (a - adjust) : (a + adjust);
	cpu->a = a;

	setFlags(a, subtract, 0, carry);
	cpu->currentIstructionCycles = 4;
	printf("DAA");
}

void SCF(cpu_t* cpu)
{
	cpu->f &= FLAG_ZERO;
	cpu->f |= FLAG_CARRY;
	cpu->currentIstructionCycles = 4;
	printf("SCF");
}

void CCF(cpu_t* cpu)
{
	uint8_t carry = isFlagSet(FLAG_CARRY) ? 0 : FLAG_CARRY;
	cpu->f &= FLAG_ZERO;
	cpu->f |= carry;
	cpu->currentIstructionCycles = 4;
	printf("CCF");
}

void JumpRelative(cpu_t* cpu, uint8_t condition, const char* mnemonic)
{
	int8_t offset = (int8_t)readByteFromAddress(cpu->pc);
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

void Ret_Conditional(cpu_t* cpu, uint8_t condition, const char* mnemonic)
{
	if (condition)
	{
		cpu->pc = PopWordFromStack(cpu);
		cpu->currentIstructionCycles = 20;
	}
	else
	{
		cpu->currentIstructionCycles = 8;
	}
	printf("RET %s", mnemonic);
}

void Call_Conditional(cpu_t* cpu, uint8_t condition, const char* mnemonic)
{
	uint16_t addressToJump = readWordFromAddress(cpu->pc);
	cpu->pc += 2;
	if (condition)
	{
		PushWordToStack(cpu, cpu->pc);
		cpu->pc = addressToJump;
		cpu->currentIstructionCycles = 24;
	}
	else
	{
		cpu->currentIstructionCycles = 12;
	}
	printf("CALL %s,%04X", mnemonic, addressToJump);
}

void Jp_Conditional(cpu_t* cpu, uint8_t condition, const char* mnemonic)
{
	uint16_t address = readWordFromAddress(cpu->pc);
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

void ADD_SP_r8(cpu_t* cpu)
{
	int8_t offset = (int8_t)readByteFromAddress(cpu->pc);
	cpu->pc++;
	uint16_t sp = cpu->sp;

	uint8_t hcarry = (((sp & 0xF) + (offset & 0xF)) & 0x10) == 0x10;
	uint8_t carry = (((sp & 0xFF) + (offset & 0xFF)) & 0x100) == 0x100;

	cpu->sp = sp + offset;
	cpu->f = (hcarry ? FLAG_HALF_CARRY : 0) | (carry ? FLAG_CARRY : 0);
	cpu->currentIstructionCycles = 16;
	printf("ADD SP,%02X", (uint8_t)offset);
}

void LD_HL_SPPlusR8(cpu_t* cpu)
{
	int8_t offset = (int8_t)readByteFromAddress(cpu->pc);
	cpu->pc++;
	uint16_t sp = cpu->sp;

	uint8_t hcarry = (((sp & 0xF) + (offset & 0xF)) & 0x10) == 0x10;
	uint8_t carry = (((sp & 0xFF) + (offset & 0xFF)) & 0x100) == 0x100;

	cpu->hl = sp + offset;
	cpu->f = (hcarry ? FLAG_HALF_CARRY : 0) | (carry ? FLAG_CARRY : 0);
	cpu->currentIstructionCycles = 12;
	printf("LD HL,SP+%02X", (uint8_t)offset);
}

void LD_Address_SP(cpu_t* cpu)
{
	uint16_t address = readWordFromAddress(cpu->pc);
	cpu->pc += 2;
	writeWordToAddress(address, cpu->sp);
	cpu->currentIstructionCycles = 20;
	printf("LD (%04X),SP", address);
}

void LD_A_FromAddress(cpu_t* cpu)
{
	uint16_t address = readWordFromAddress(cpu->pc);
	cpu->pc += 2;
	cpu->a = readByteFromAddress(address);
	cpu->currentIstructionCycles = 16;
	printf("LD A,(%04X)", address);
}

void LD_RegAddressHighToReg(cpu_t* cpu, uint8_t* regDest, uint8_t* regAddress, const char* regDestName, const char* regAddressName)
{
	uint8_t value = readByteFromAddress(0xff00 + *regAddress);
	*regDest = value;
	cpu->currentIstructionCycles = 8;
	printf("LD %s,(FF00+%s)", regDestName, regAddressName);
}

uint8_t ReadCBOperand(cpu_t* cpu, uint8_t regIndex)
{
	switch (regIndex)
	{
		case 0: return cpu->b;
		case 1: return cpu->c;
		case 2: return cpu->d;
		case 3: return cpu->e;
		case 4: return cpu->h;
		case 5: return cpu->l;
		case 6: return readByteFromAddress(cpu->hl);
		case 7: return cpu->a;
	}
	return 0;
}

void WriteCBOperand(cpu_t* cpu, uint8_t regIndex, uint8_t value)
{
	switch (regIndex)
	{
		case 0: cpu->b = value; break;
		case 1: cpu->c = value; break;
		case 2: cpu->d = value; break;
		case 3: cpu->e = value; break;
		case 4: cpu->h = value; break;
		case 5: cpu->l = value; break;
		case 6: writeByteToAddress(cpu->hl, value); break;
		case 7: cpu->a = value; break;
	}
}

const char* CBOperandName(uint8_t regIndex)
{
	static const char* names[8] = { "B", "C", "D", "E", "H", "L", "(HL)", "A" };
	return names[regIndex & 0x07];
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
		case 0x02:
		{
			LD_RegTo16BitRegAddress(&cpuInstance, cpuInstance.bc, cpuInstance.a, "BC", "A");
			break;
		}
		case 0x03:
		{
			INC_16BitReg(&cpuInstance, &cpuInstance.bc, "BC");
			break;
		}
		case 0x04:
		{
			INC_8BitReg(&cpuInstance, &cpuInstance.b, "B");
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
		case 0x07:
		{
			Rotate_A_RLCA(&cpuInstance);
			break;
		}
		case 0x08:
		{
			LD_Address_SP(&cpuInstance);
			break;
		}
		case 0x09:
		{
			ADD_16BitReg(&cpuInstance, &cpuInstance.hl, &cpuInstance.bc, "HL", "BC");
			break;
		}
		case 0x12:
		{
			LD_RegTo16BitRegAddress(&cpuInstance, cpuInstance.de, cpuInstance.a, "DE", "A");
			break;
		}
		case 0x0a:
		{
			LD_16BitRegAddressToReg(&cpuInstance, &cpuInstance.a, cpuInstance.bc, "A", "BC");
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
		case 0x0f:
		{
			Rotate_A_RRCA(&cpuInstance);
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
			LD_WordTo16BitReg(&cpuInstance, &cpuInstance.de, "DE");
			break;
		}
		case 0x13:
		{
			INC_16BitReg(&cpuInstance, &cpuInstance.de, "DE");
			break;
		}
		case 0x14:
		{
			INC_8BitReg(&cpuInstance, &cpuInstance.d, "D");
			break;
		}
		case 0x15:
		{
			DEC_8BitReg(&cpuInstance, &cpuInstance.d, "D");
			break;
		}
		case 0x16:
		{
			LD_ByteToReg(&cpuInstance, &cpuInstance.d, 'D');
			break;
		}
		case 0x17:
		{
			Rotate_A_RLA(&cpuInstance);
			break;
		}
		case 0x18:
		{
			JumpRelative(&cpuInstance, 1, "");
			break;
		}
		case 0x19:
		{
			ADD_16BitReg(&cpuInstance, &cpuInstance.hl, &cpuInstance.de, "HL", "DE");
			break;
		}
		case 0x1a:
		{
			LD_16BitRegAddressToReg(&cpuInstance, &cpuInstance.a, cpuInstance.de, "A", "DE");
			break;
		}
		case 0x1b:
		{
			DEC_16BitReg(&cpuInstance, &cpuInstance.de, "DE");
			break;
		}
		case 0x1c:
		{
			INC_8BitReg(&cpuInstance, &cpuInstance.e, "E");
			break;
		}
		case 0x1d:
		{
			DEC_8BitReg(&cpuInstance, &cpuInstance.e, "E");
			break;
		}
		case 0x1e:
		{
			LD_ByteToReg(&cpuInstance, &cpuInstance.e, 'E');
			break;
		}
		case 0x1f:
		{
			Rotate_A_RRA(&cpuInstance);
			break;
		}
		case 0x20:
		{
			JumpRelative(&cpuInstance, !isFlagSet(FLAG_ZERO), "NZ");
			break;
		}
		case 0x21:
		{
			LD_WordTo16BitReg(&cpuInstance, &cpuInstance.hl, "HL");
			break;
		}
		case 0x23:
		{
			INC_16BitReg(&cpuInstance, &cpuInstance.hl, "HL");
			break;
		}
		case 0x22:
		{
			writeByteToAddress(cpuInstance.hl, cpuInstance.a);
			cpuInstance.hl++;
			cpuInstance.currentIstructionCycles = 8;
			printf("LD (HL+),A");
			break;
		}
		case 0x24:
		{
			INC_8BitReg(&cpuInstance, &cpuInstance.h, "H");
			break;
		}
		case 0x25:
		{
			DEC_8BitReg(&cpuInstance, &cpuInstance.h, "H");
			break;
		}
		case 0x26:
		{
			LD_ByteToReg(&cpuInstance, &cpuInstance.h, 'H');
			break;
		}
		case 0x27:
		{
			DAA(&cpuInstance);
			break;
		}
		case 0x28:
		{
			JumpRelative(&cpuInstance, isFlagSet(FLAG_ZERO), "Z");
			break;
		}
		case 0x29:
		{
			ADD_16BitReg(&cpuInstance, &cpuInstance.hl, &cpuInstance.hl, "HL", "HL");
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
		case 0x2c:
		{
			INC_8BitReg(&cpuInstance, &cpuInstance.l, "L");
			break;
		}
		case 0x2d:
		{
			DEC_8BitReg(&cpuInstance, &cpuInstance.l, "L");
			break;
		}
		case 0x2e:
		{
			LD_ByteToReg(&cpuInstance, &cpuInstance.l, 'L');
			break;
		}
		case 0x2f:
		{
			CPL(&cpuInstance);
			break;
		}
		case 0x30:
		{
			JumpRelative(&cpuInstance, !isFlagSet(FLAG_CARRY), "NC");
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
		case 0x33:
		{
			INC_16BitReg(&cpuInstance, &cpuInstance.sp, "SP");
			break;
		}
		case 0x34:
		{
			INC_HLAddress(&cpuInstance);
			break;
		}
		case 0x35:
		{
			DEC_HLAddress(&cpuInstance);
			break;
		}
		case 0x36:
		{
			LD_ByteToAddress(&cpuInstance, cpuInstance.hl, "HL");
			break;
		}
		case 0x37:
		{
			SCF(&cpuInstance);
			break;
		}
		case 0x38:
		{
			JumpRelative(&cpuInstance, isFlagSet(FLAG_CARRY), "C");
			break;
		}
		case 0x39:
		{
			ADD_16BitReg(&cpuInstance, &cpuInstance.hl, &cpuInstance.sp, "HL", "SP");
			break;
		}
		case 0x3a:
		{
			cpuInstance.a = readByteFromAddress(cpuInstance.hl);
			cpuInstance.hl--;
			cpuInstance.currentIstructionCycles = 8;
			printf("LD A,(HL-)");
			break;
		}
		case 0x3b:
		{
			DEC_16BitReg(&cpuInstance, &cpuInstance.sp, "SP");
			break;
		}
		case 0x3c:
		{
			INC_8BitReg(&cpuInstance, &cpuInstance.a, "A");
			break;
		}
		case 0x3d:
		{
			DEC_8BitReg(&cpuInstance, &cpuInstance.a, "A");
			break;
		}
		case 0x3E:
		{
			LD_ByteToReg(&cpuInstance, &cpuInstance.a, 'A');
			break;
		}
		case 0x3f:
		{
			CCF(&cpuInstance);
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
			break;
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


		case 0x70:
		{
			LD_RegTo16BitRegAddress(&cpuInstance, cpuInstance.hl, cpuInstance.b, "HL", "B");
			break;
		}
		case 0x71:
		{
			LD_RegTo16BitRegAddress(&cpuInstance, cpuInstance.hl, cpuInstance.c, "HL", "C");
			break;
		}
		case 0x72:
		{
			LD_RegTo16BitRegAddress(&cpuInstance, cpuInstance.hl, cpuInstance.d, "HL", "D");
			break;
		}
		case 0x73:
		{
			LD_RegTo16BitRegAddress(&cpuInstance, cpuInstance.hl, cpuInstance.e, "HL", "E");
			break;
		}
		case 0x74:
		{
			LD_RegTo16BitRegAddress(&cpuInstance, cpuInstance.hl, cpuInstance.h, "HL", "H");
			break;
		}
		case 0x75:
		{
			LD_RegTo16BitRegAddress(&cpuInstance, cpuInstance.hl, cpuInstance.l, "HL", "L");
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
			LD_RegTo16BitRegAddress(&cpuInstance, cpuInstance.hl, cpuInstance.a, "HL", "A");
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
		case 0x86:
		{
			ADD_FromHL(&cpuInstance);
			break;
		}
		case 0x87:
		{
			ADD_8BitReg(&cpuInstance, &cpuInstance.a, "A");
			break;
		}
		case 0x88:
		{
			ADC_8BitReg(&cpuInstance, &cpuInstance.b, "B");
			break;
		}
		case 0x89:
		{
			ADC_8BitReg(&cpuInstance, &cpuInstance.c, "C");
			break;
		}
		case 0x8a:
		{
			ADC_8BitReg(&cpuInstance, &cpuInstance.d, "D");
			break;
		}
		case 0x8b:
		{
			ADC_8BitReg(&cpuInstance, &cpuInstance.e, "E");
			break;
		}
		case 0x8c:
		{
			ADC_8BitReg(&cpuInstance, &cpuInstance.h, "H");
			break;
		}
		case 0x8d:
		{
			ADC_8BitReg(&cpuInstance, &cpuInstance.l, "L");
			break;
		}
		case 0x8e:
		{
			ADC_FromHL(&cpuInstance);
			break;
		}
		case 0x8f:
		{
			ADC_8BitReg(&cpuInstance, &cpuInstance.a, "A");
			break;
		}
		case 0x90:
		{
			SUB_8BitReg(&cpuInstance, &cpuInstance.b, "B");
			break;
		}
		case 0x91:
		{
			SUB_8BitReg(&cpuInstance, &cpuInstance.c, "C");
			break;
		}
		case 0x92:
		{
			SUB_8BitReg(&cpuInstance, &cpuInstance.d, "D");
			break;
		}
		case 0x93:
		{
			SUB_8BitReg(&cpuInstance, &cpuInstance.e, "E");
			break;
		}
		case 0x94:
		{
			SUB_8BitReg(&cpuInstance, &cpuInstance.h, "H");
			break;
		}
		case 0x95:
		{
			SUB_8BitReg(&cpuInstance, &cpuInstance.l, "L");
			break;
		}
		case 0x96:
		{
			SUB_FromHL(&cpuInstance);
			break;
		}
		case 0x97:
		{
			SUB_8BitReg(&cpuInstance, &cpuInstance.a, "A");
			break;
		}
		case 0x98:
		{
			SBC_8BitReg(&cpuInstance, &cpuInstance.b, "B");
			break;
		}
		case 0x99:
		{
			SBC_8BitReg(&cpuInstance, &cpuInstance.c, "C");
			break;
		}
		case 0x9a:
		{
			SBC_8BitReg(&cpuInstance, &cpuInstance.d, "D");
			break;
		}
		case 0x9b:
		{
			SBC_8BitReg(&cpuInstance, &cpuInstance.e, "E");
			break;
		}
		case 0x9c:
		{
			SBC_8BitReg(&cpuInstance, &cpuInstance.h, "H");
			break;
		}
		case 0x9d:
		{
			SBC_8BitReg(&cpuInstance, &cpuInstance.l, "L");
			break;
		}
		case 0x9e:
		{
			SBC_FromHL(&cpuInstance);
			break;
		}
		case 0x9f:
		{
			SBC_8BitReg(&cpuInstance, &cpuInstance.a, "A");
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
		case 0xae:
		{
			XOR_FromHL(&cpuInstance);
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
		case 0xb8:
		{
			CP_8BitReg(&cpuInstance, &cpuInstance.b, "B");
			break;
		}
		case 0xb9:
		{
			CP_8BitReg(&cpuInstance, &cpuInstance.c, "C");
			break;
		}
		case 0xba:
		{
			CP_8BitReg(&cpuInstance, &cpuInstance.d, "D");
			break;
		}
		case 0xbb:
		{
			CP_8BitReg(&cpuInstance, &cpuInstance.e, "E");
			break;
		}
		case 0xbc:
		{
			CP_8BitReg(&cpuInstance, &cpuInstance.h, "H");
			break;
		}
		case 0xbd:
		{
			CP_8BitReg(&cpuInstance, &cpuInstance.l, "L");
			break;
		}
		case 0xbe:
		{
			CP_FromHL(&cpuInstance);
			break;
		}
		case 0xbf:
		{
			CP_8BitReg(&cpuInstance, &cpuInstance.a, "A");
			break;
		}
		case 0xc0:
		{
			Ret_Conditional(&cpuInstance, !isFlagSet(FLAG_ZERO), "NZ");
			break;
		}
		case 0xc1:
		{
			POP_16BitReq(&cpuInstance, &cpuInstance.bc, "BC");
			break;
		}
		case 0xc2:
		{
			Jp_Conditional(&cpuInstance, !isFlagSet(FLAG_ZERO), "NZ");
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
		case 0xc4:
		{
			Call_Conditional(&cpuInstance, !isFlagSet(FLAG_ZERO), "NZ");
			break;
		}
		case 0xc5:
		{
			PUSH_16BitReq(&cpuInstance, &cpuInstance.bc, "BC");
			break;
		}
		case 0xc6:
		{
			ADD_FromPC(&cpuInstance);
			break;
		}
		case 0xc7:
		{
			RST(&cpuInstance, 0x00);
			break;
		}
		case 0xc8:
		{
			Ret_Conditional(&cpuInstance, isFlagSet(FLAG_ZERO), "Z");
			break;
		}
		case 0xc9:
		{
			Ret(&cpuInstance);
			break;
		}
		case 0xca:
		{
			Jp_Conditional(&cpuInstance, isFlagSet(FLAG_ZERO), "Z");
			break;
		}
		case 0xcb:
		{
			uint8_t cb = cpuInstance.currentIstructionCBOpCode;
			uint8_t regIndex = cb & 0x07;
			uint8_t opGroup = cb >> 6;
			uint8_t bitOrSub = (cb >> 3) & 0x07;
			const char* regName = CBOperandName(regIndex);
			uint8_t value = ReadCBOperand(&cpuInstance, regIndex);
			uint8_t isHL = (regIndex == 6);

			if (opGroup == 1)
			{
				uint8_t bitSet = value & (1 << bitOrSub);
				setFlags(bitSet, 0, 1, isFlagSet(FLAG_CARRY));
				cpuInstance.currentIstructionCycles = isHL ? 12 : 8;
				printf("BIT %d,%s", bitOrSub, regName);
			}
			else if (opGroup == 2)
			{
				value &= ~(1 << bitOrSub);
				WriteCBOperand(&cpuInstance, regIndex, value);
				cpuInstance.currentIstructionCycles = isHL ? 16 : 8;
				printf("RES %d,%s", bitOrSub, regName);
			}
			else if (opGroup == 3)
			{
				value |= (1 << bitOrSub);
				WriteCBOperand(&cpuInstance, regIndex, value);
				cpuInstance.currentIstructionCycles = isHL ? 16 : 8;
				printf("SET %d,%s", bitOrSub, regName);
			}
			else
			{
				uint8_t result = 0;
				uint8_t carryOut = 0;
				uint8_t oldCarry = isFlagSet(FLAG_CARRY) ? 1 : 0;

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

				setFlags(result, 0, 0, carryOut);
				WriteCBOperand(&cpuInstance, regIndex, result);
				cpuInstance.currentIstructionCycles = isHL ? 16 : 8;
			}
			break;
		}
		case 0xcc:
		{
			Call_Conditional(&cpuInstance, isFlagSet(FLAG_ZERO), "Z");
			break;
		}
		case 0xcd:
		{
			Call(&cpuInstance);
			break;
		}
		case 0xce:
		{
			ADC_FromPC(&cpuInstance);
			break;
		}
		case 0xcf:
		{
			RST(&cpuInstance, 0x08);
			break;
		}
		case 0xd0:
		{
			Ret_Conditional(&cpuInstance, !isFlagSet(FLAG_CARRY), "NC");
			break;
		}
		case 0xd1:
		{
			POP_16BitReq(&cpuInstance, &cpuInstance.de, "DE");
			break;
		}
		case 0xd2:
		{
			Jp_Conditional(&cpuInstance, !isFlagSet(FLAG_CARRY), "NC");
			break;
		}
		case 0xd4:
		{
			Call_Conditional(&cpuInstance, !isFlagSet(FLAG_CARRY), "NC");
			break;
		}
		case 0xd5:
		{
			PUSH_16BitReq(&cpuInstance, &cpuInstance.de, "DE");
			break;
		}
		case 0xd6:
		{
			SUB_FromPC(&cpuInstance);
			break;
		}
		case 0xd7:
		{
			RST(&cpuInstance, 0x10);
			break;
		}
		case 0xd8:
		{
			Ret_Conditional(&cpuInstance, isFlagSet(FLAG_CARRY), "C");
			break;
		}
		case 0xd9:
		{
			Ret(&cpuInstance);
			printf(" (RETI - IME not modeled)");
			break;
		}
		case 0xda:
		{
			Jp_Conditional(&cpuInstance, isFlagSet(FLAG_CARRY), "C");
			break;
		}
		case 0xdc:
		{
			Call_Conditional(&cpuInstance, isFlagSet(FLAG_CARRY), "C");
			break;
		}
		case 0xde:
		{
			SBC_FromPC(&cpuInstance);
			break;
		}
		case 0xdf:
		{
			RST(&cpuInstance, 0x18);
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
		case 0xe1:
		{
			POP_16BitReq(&cpuInstance, &cpuInstance.hl, "HL");
			break;
		}
		case 0xE2:
		{
			LD_RegValueToRegAddressHigh(&cpuInstance, &cpuInstance.a, &cpuInstance.c, "A", "C");
			break;
		}
		case 0xe5:
		{
			PUSH_16BitReq(&cpuInstance, &cpuInstance.hl, "HL");
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
		case 0xE8:
		{
			ADD_SP_r8(&cpuInstance);
			break;
		}
		case 0xE9:
		{
			Jump_HL(&cpuInstance);
			break;
		}
		case 0xEe:
		{
			XOR_FromPC(&cpuInstance);
			break;
		}
		case 0xEf:
		{
			RST(&cpuInstance, 0x28);
			break;
		}
		case 0xE7:
		{
			RST(&cpuInstance, 0x20);
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
		case 0xf1:
		{
			POP_16BitReq(&cpuInstance, &cpuInstance.af, "AF");
			break;
		}
		case 0xf2:
		{
			LD_RegAddressHighToReg(&cpuInstance, &cpuInstance.a, &cpuInstance.c, "A", "C");
			break;
		}
		case 0xf3:
		{
			printf("DI");
			cpuInstance.currentIstructionCycles = 4;
			break;
		}
		case 0xf5:
		{
			PUSH_16BitReq(&cpuInstance, &cpuInstance.af, "AF");
			break;
		}
		case 0xf7:
		{
			RST(&cpuInstance, 0x30);
			break;
		}
		case 0xf8:
		{
			LD_HL_SPPlusR8(&cpuInstance);
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
			LD_A_FromAddress(&cpuInstance);
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
		case 0xff:
		{
			RST(&cpuInstance, 0x38);
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

