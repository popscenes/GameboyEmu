#include <stdio.h>
#include <stdlib.h>
#include "Cart.h"
#include "cpu.h"
#include <Windows.h>

int main(int argc, char* args[])
{
	LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
	LARGE_INTEGER Frequency;

	char str1[20];
	printf("running.....\n");
	if (argc < 2)
	{
		printf("no rom file selected");
		return 0;
	}
	loadCart(args[1]);
	cpuInit();

	while (true)
	{
		

		QueryPerformanceFrequency(&Frequency);
		QueryPerformanceCounter(&StartingTime);

		cpuStep();

		QueryPerformanceCounter(&EndingTime);
		ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;

		ElapsedMicroseconds.QuadPart *= 1000000;
		ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
	}

	printf("press enter to exit\n");
	scanf_s("19%s", str1, (unsigned)_countof(str1));

	return 0;
}