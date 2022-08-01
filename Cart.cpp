#include "Cart.h"

cart_t cart = { 0 };

int loadCart(char* filename)
{
	
	long cartSize = 0;
	FILE* cartFile;
	errno_t fopenresult = fopen_s(&cartFile, filename, "r");
	if (fopenresult != 0)
	{
		//printf("error opening %s file result %d", filename, fopenresult);
	}
	
	fseek(cartFile, 0L, SEEK_END);
	cartSize = ftell(cartFile);
	rewind(cartFile);

	cart.romData = (uint8_t*)malloc(cartSize);
	
	fread(cart.romData, 1, cartSize, cartFile);

	fclose(cartFile);

	cart.header = (rom_header_t*)(cart.romData + 0x100);

	//printf("Cartridge Loaded:\n");
	//printf("Title    : %s\n", cart.header->title);

	return 0;
	
}

uint8_t readyByteFromCart(uint16_t address)
{
	return cart.romData[address];
}