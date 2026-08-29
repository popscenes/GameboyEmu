#include <stdio.h>
#include <stdlib.h>
#include "Cart.h"
#include "cpu.h"
#include "GameBoy.h"
#include <Windows.h>
#include <queue>

#include "SDL_GameboyRenderer.h"

using namespace std;

bool isRunning;

void ProcessEvents()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			isRunning = false;
			break;
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE)
			{
				isRunning = false;
			}
			break;
		default:
			break;
		}
	}
}


int main(int argc, char* args[])
{

	LARGE_INTEGER StartingTime, EndingTime, ElapsedNanoseconds;
	LARGE_INTEGER Frequency;

	char str1[20];
	char buffer[100];
	if (argc < 2)
	{
		return 0;
	}

	SDl_GameBoyRenderer_t renderer;
	if(!GameBoyRendererInit(&renderer))
	{
		sprintf_s(buffer, "GameBoyRendererInit failed");
		return 0;
	}

	gameboy_t gb = { 0 };

	loadCart(&gb.cart, args[1]);
	cpuInit(&gb);

	QueryPerformanceFrequency(&Frequency);

	isRunning = true;



	while (isRunning)
	{
		
		QueryPerformanceCounter(&StartingTime);
		ProcessEvents();

		gameboyStep(&gb);
		QueryPerformanceCounter(&EndingTime);
		ElapsedNanoseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;

		ElapsedNanoseconds.QuadPart *= 1000000000;
		ElapsedNanoseconds.QuadPart /= Frequency.QuadPart;

		uint64_t nanoSecsforInst = (uint64_t)cpuCurrentIstructionCycles(&gb) * NANOSECONDS_PER_TICK;
		if (nanoSecsforInst < ElapsedNanoseconds.QuadPart)
		{
			
			sprintf_s(buffer, "nanoSecsforInst: %lld over elapsed time:  %lld\n", nanoSecsforInst, ElapsedNanoseconds.QuadPart);
			OutputDebugStringA(buffer);
		}
		else
		{
			
			sprintf_s(buffer, "nanoSecsforInst: %lld under elapsed time:  %lld\n", nanoSecsforInst, ElapsedNanoseconds.QuadPart);
			OutputDebugStringA(buffer);
		}
	}

	printf("press enter to exit\n");
	scanf_s("19%s", str1, (unsigned)_countof(str1));

	return 0;
}