#pragma once

#include <stdint.h>
#include <stdio.h>
#include <malloc.h>

#define CART_RAM_ENABLE_END_ADDRESS 0x1FFF
#define CART_RAM_ENABLE_VALUE 0x0A

#define CART_RAM_START_ADDRESS 0xA000
#define CART_RAM_END_ADDRESS 0xBFFF
#define CART_RAM_SIZE 0x2000

typedef struct {
	uint8_t  entryPoint[4];
	uint8_t  nintendoLogo[48];
	char  title[16];

	//uint8_t  manufacturerCode[4];
	//uint8_t  gbcFLag;

	uint16_t  newLicenseCode;
	uint8_t  sgbFLag;
	uint8_t  cartridgeType;
	uint8_t  romSize;
	uint8_t  ramSize;
	uint8_t  destinationCode;
	uint8_t  oldLicenseode;
	uint8_t  maskRomVersion;
	uint8_t  headerChecksum;
	uint16_t  globalChecksum;

}
rom_header_t;

typedef struct {
	rom_header_t* header;
	uint8_t* romData;
	uint8_t romBank;
	bool ramEnabled;
	uint8_t ram[CART_RAM_SIZE];
} cart_t;

int loadCart(cart_t* cart, char* filename);
uint8_t readyByteFromCart(cart_t* cart, uint16_t address);
void writeByteToCart(cart_t* cart, uint16_t address, uint8_t value);
uint8_t readByteFromCartRam(cart_t* cart, uint16_t address);
void writeByteToCartRam(cart_t* cart, uint16_t address, uint8_t value);

	

