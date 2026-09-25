#include "Cart.h"

int loadCart(cart_t* cart, char* filename)
{

	long cartSize = 0;
	FILE* cartFile;
	errno_t fopenresult = fopen_s(&cartFile, filename, "rb");
	if (fopenresult != 0 || cartFile == NULL)
	{
		printf("error opening %s file result %d", filename, fopenresult);
		return 1;
	}

	fseek(cartFile, 0L, SEEK_END);
	cartSize = ftell(cartFile);
	rewind(cartFile);

	cart->romData = (uint8_t*)malloc(cartSize);

	fread(cart->romData, 1, cartSize, cartFile);

	fclose(cartFile);

	cart->romBank = 1;

	cart->header = (rom_header_t*)(cart->romData + 0x100);

	printf("Cartridge Loaded:\n");
	printf("Title    : %s\n", cart->header->title);

	return 0;

}

uint8_t readyByteFromCart(cart_t* cart, uint16_t address)
{
	if (address < 0x4000)
	{
		return cart->romData[address];
	}

	uint32_t bankAddress = ((cart->romBank-1) * 0x4000) + address;

	return cart->romData[bankAddress];
}

void writeByteToCart(cart_t* cart, uint16_t address, uint8_t value)
{
	if (address < 0x2000 || address > 0x3FFF)
	{
		return;
	}

	value = value & 0x1F;


	if (value == 0)
	{
		value = 1;
	}

	uint8_t maxBanks = 2 << cart->header->romSize;
	value = value & (maxBanks - 1);

	cart->romBank = value & 0x1F;

}
