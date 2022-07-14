#include "cpu.h""
#include "cart.h""

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

void cpuStep() {
	uint8_t opCode = readyByteFromCart(cpuInstance.pc);
	cpuInstance.pc++;
	instruction_t inst = { 0 };
	if (opCode == 0x00)
	{
		printf("noop\n");
	}
	else if (opCode == 0x31)
	{
		
		uint8_t loByte = readyByteFromCart(cpuInstance.pc);
		uint8_t hiByte = readyByteFromCart(cpuInstance.pc + 1);
		
		uint16_t value = (hiByte << 8) | (loByte);

		cpuInstance.pc += 2;
		cpuInstance.sp = value;
		printf("LD SP,%04X\n", value);
	}
	else if (opCode == 0x3E)
	{
		
		uint8_t byte = readyByteFromCart(cpuInstance.pc);

		cpuInstance.pc++;
		cpuInstance.a = byte;
		printf("LD A,#%d\n", byte);
	}
	else if (opCode == 0xc3)
	{
		
		uint8_t loByte =  readyByteFromCart(cpuInstance.pc);
		uint8_t hiByte = readyByteFromCart(cpuInstance.pc+1);

		uint16_t address = (hiByte << 8) | (loByte);

		jump(address);
		printf("jp %04X\n", address);
	}
	else if (opCode == 0xf3)
	{
		printf("DI\n");
	}
	else
	{
		printf("Unknown\n");
	}

}

