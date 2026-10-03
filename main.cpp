#include <stdio.h>
#include <stdlib.h>
#include "Cart.h"
#include "cpu.h"
#include "GameBoy.h"
#include <Windows.h>
#include <queue>

#include "SDL_GameboyRenderer.h"
#include "debug.h"
#include <string.h>

using namespace std;

bool isRunning;
bool debugTrace = false;

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
			else if (event.key.keysym.sym == SDLK_F1 && event.key.repeat == 0)
			{
				debugTrace = !debugTrace;
				printf("\nDebug trace %s\n", debugTrace ? "ON" : "OFF");
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
	LARGE_INTEGER Frequency, FrameStart, FrameEnd;

	char str1[20];
	char buffer[100];
	if (argc < 2)
	{
		return 0;
	}

	for (int i = 2; i < argc; i++)
	{
		if (strcmp(args[i], "--debug") == 0)
		{
			debugTrace = true;
		}
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
	uint32_t frameCycles = 0;
	
	QueryPerformanceCounter(&FrameStart);
	while (isRunning)
	{

		QueryPerformanceCounter(&StartingTime);

		gameboyStep(&gb);

		frameCycles += cpuCurrentIstructionCycles(&gb);
		if (frameCycles >= CYCLES_PER_FRAME)
		{
			frameCycles -= CYCLES_PER_FRAME;
			ProcessEvents();
			screenRenderBackground(&gb.screen, renderer.framebuffer);
			GameBoyRendererPresent(&renderer);

			QueryPerformanceCounter(&FrameEnd);

			ElapsedNanoseconds.QuadPart = FrameEnd.QuadPart - FrameStart.QuadPart;

			ElapsedNanoseconds.QuadPart *= 1000000000;
			ElapsedNanoseconds.QuadPart /= Frequency.QuadPart;

			int64_t timeLeftInFrame = 16740000 - ElapsedNanoseconds.QuadPart;
			if (timeLeftInFrame > 0)
			{
				SDL_Delay(timeLeftInFrame / 1000000);
			}

			QueryPerformanceCounter(&FrameStart);
		}

		QueryPerformanceCounter(&EndingTime);
		ElapsedNanoseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;

		ElapsedNanoseconds.QuadPart *= 1000000000;
		ElapsedNanoseconds.QuadPart /= Frequency.QuadPart;

		if (debugTrace)
		{
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

		
	}

	GameBoyRendererShutdown(&renderer);

	printf("press enter to exit\n");
	scanf_s("19%s", str1, (unsigned)_countof(str1));

	return 0;
}