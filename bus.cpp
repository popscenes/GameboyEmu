#include "bus.h"
#include "cart.h"

internalMemory_t internalMemory = { 0 };

uint16_t readWordFromAddress(uint16_t adderss) {
	
	uint8_t loByte = 0;
	uint8_t hiByte = 0;

	if (adderss <= CART_END_ADDRESS)
	{
		loByte = readyByteFromCart(adderss);
		hiByte = readyByteFromCart(adderss + 1);
	}
	else if (adderss >= HIGH_RAM_START_ADDRESS && adderss <= HIGH_RAM_END_ADDRESS)
	{
		adderss -= HIGH_RAM_START_ADDRESS;

		loByte = internalMemory.highRam[adderss];
		hiByte = internalMemory.highRam[adderss+1];
	}

	uint16_t value = (hiByte << 8) | (loByte);
	return value;
}
uint8_t readByteFromAddress(uint16_t adderss)
{
	uint8_t byte = 0;
	if (adderss <= CART_END_ADDRESS)
	{
		byte = readyByteFromCart(adderss);
	}

	else if (adderss >= HIGH_RAM_START_ADDRESS && adderss <= HIGH_RAM_END_ADDRESS)
	{
		adderss -= HIGH_RAM_START_ADDRESS;
		byte = internalMemory.highRam[adderss];
	}

	return byte;
}

void writeWordToAddress(uint16_t adderss, uint16_t value)
{

}

void writeByteToAddress(uint16_t adderss, uint8_t value)
{
	uint8_t byte = 0;
	if (adderss <= CART_END_ADDRESS)
	{
	}

	else if (adderss >= HIGH_RAM_START_ADDRESS && adderss <= HIGH_RAM_END_ADDRESS)
	{
		
		adderss -= HIGH_RAM_START_ADDRESS;
		internalMemory.highRam[adderss] = value;
	}
}