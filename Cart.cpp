#include "Cart.h"
#include <string.h>

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
	
	if (address <= CART_RAM_ENABLE_END_ADDRESS)
	{
		if ((value & 0x0F) == CART_RAM_ENABLE_VALUE)
		{
			cart->ramEnabled = true;
		}
		else
		{
			cart->ramEnabled = false;
		}
		return;
	}

	if (address >= 0x2000 && address <= 0x3FFF)
	{
		value = value & 0x1F;


		if (value == 0)
		{
			value = 1;
		}

		uint8_t maxBanks = 2 << cart->header->romSize;
		value = value & (maxBanks - 1);

		cart->romBank = value & 0x1F;
		return;
	}

	
}

uint8_t readByteFromCartRam(cart_t* cart, uint16_t address)
{
	if (cart->ramEnabled)
	{
		return cart->ram[address - CART_RAM_START_ADDRESS];
	}
	return 0xFF;
}

void writeByteToCartRam(cart_t* cart, uint16_t address, uint8_t value)
{
	if (cart->ramEnabled)
	{
		cart->ram[address - CART_RAM_START_ADDRESS] = value;
	}
}

void cartBuildSavePath(cart_t* cart, const char* romPath)
{
	const char* newExtension = ".sav";
	
	strcpy_s(cart->savePath, CART_SAVE_PATH_MAX, romPath);
	char* extension = strrchr(cart->savePath, '.');
	size_t len = CART_SAVE_PATH_MAX - (extension - cart->savePath);
	
	strcpy_s(extension, len, newExtension);
}

bool cartHasBattery(cart_t* cart)
{
	// TODO
	return cart->header->cartridgeType == CART_TYPE_MBC1_RAM_BATTERY;
}

void cartLoadRam(cart_t* cart)
{
	if (!cartHasBattery(cart))
	{
		return;
	}

	FILE* saveFile;
	errno_t fopenresult = fopen_s(&saveFile, cart->savePath, "rb");
	if (fopenresult != 0 || saveFile == NULL)
	{
		printf("no save file found %s %d\n", cart->savePath, fopenresult);
		return;
	}

	size_t readBytes =  fread(cart->ram, 1, CART_RAM_SIZE, saveFile);
	if (readBytes != CART_RAM_SIZE)
	{
		printf("save file no right size. %s\n", cart->savePath);
	}
	fclose(saveFile);
}

void cartSaveRam(cart_t* cart)
{
	if (!cartHasBattery(cart))
	{
		return;
	}

	FILE* saveFile;
	errno_t fopenresult = fopen_s(&saveFile, cart->savePath, "wb");
	if (fopenresult != 0 || saveFile == NULL)
	{
		printf("error writing save file %s %d\n", cart->savePath, fopenresult);
		return;
	}

	size_t writeBytes = fwrite(cart->ram, 1, CART_RAM_SIZE, saveFile);
	if (writeBytes != CART_RAM_SIZE)
	{
		printf("not all bytes written on save. %s\n", cart->savePath);
	}

	fclose(saveFile);
}
