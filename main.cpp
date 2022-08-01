#include <stdio.h>
#include <stdlib.h>
#include "Cart.h"
#include "cpu.h"
#include "GameBoy.h"
#include <Windows.h>

int main(int argc, char* args[])
{
	LARGE_INTEGER StartingTime, EndingTime, ElapsedNanoseconds;
	LARGE_INTEGER Frequency;

	char str1[20];
	char buffer[100];
	//printf("running.....\n");
	if (argc < 2)
	{
		//printf("no rom file selected");
		return 0;
	}
	loadCart(args[1]);
	cpuInit();
	
	QueryPerformanceFrequency(&Frequency);

	while (true)
	{
		

		
		QueryPerformanceCounter(&StartingTime);

		cpuStep();

		QueryPerformanceCounter(&EndingTime);
		ElapsedNanoseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;

		ElapsedNanoseconds.QuadPart *= 1000000000;
		ElapsedNanoseconds.QuadPart /= Frequency.QuadPart;

		uint64_t nanoSecsforInst = (uint64_t)cpuCurrentIstructionCycles() * NANOSECONDS_PER_TICK;
		if (nanoSecsforInst < ElapsedNanoseconds.QuadPart)
		{
			//printf("over elapsed time");
			sprintf_s(buffer, "nanoSecsforInst: %lld over elapsed time:  %lld\n", nanoSecsforInst, ElapsedNanoseconds.QuadPart);
			OutputDebugStringA(buffer);
		}
		else
		{
			
			sprintf_s(buffer, "nanoSecsforInst: %lld under elapsed time:  %lld\n", nanoSecsforInst, ElapsedNanoseconds.QuadPart);
			OutputDebugStringA(buffer);
			while (ElapsedNanoseconds.QuadPart < nanoSecsforInst)
			{
				QueryPerformanceCounter(&EndingTime);
				ElapsedNanoseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;

				ElapsedNanoseconds.QuadPart *= 1000000000;
				ElapsedNanoseconds.QuadPart /= Frequency.QuadPart;
			}
		}
	}

	//printf("press enter to exit\n");
	scanf_s("19%s", str1, (unsigned)_countof(str1));

	return 0;
}