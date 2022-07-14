#pragma once

#include <stdint.h>
#include <stdio.h>
#include <malloc.h>

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
} cart_t;

int loadCart(char* filename);
uint8_t readyByteFromCart(uint16_t address);

	

