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

void HandleJoypadKey(joypad_t* joypad, SDL_Keycode key, bool pressed)
{
	switch (key)
	{
	case SDLK_RIGHT:     joypadSetButton(joypad, JOYPAD_BUTTON_RIGHT, pressed); break;
	case SDLK_LEFT:      joypadSetButton(joypad, JOYPAD_BUTTON_LEFT, pressed); break;
	case SDLK_UP:        joypadSetButton(joypad, JOYPAD_BUTTON_UP, pressed); break;
	case SDLK_DOWN:      joypadSetButton(joypad, JOYPAD_BUTTON_DOWN, pressed); break;
	case SDLK_z:         joypadSetButton(joypad, JOYPAD_BUTTON_A, pressed); break;
	case SDLK_x:         joypadSetButton(joypad, JOYPAD_BUTTON_B, pressed); break;
	case SDLK_BACKSPACE: joypadSetButton(joypad, JOYPAD_BUTTON_SELECT, pressed); break;
	case SDLK_RETURN:    joypadSetButton(joypad, JOYPAD_BUTTON_START, pressed); break;
	default: break;
	}
}

void ProcessEvents(joypad_t* joypad)
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
			else
			{
				HandleJoypadKey(joypad, event.key.keysym.sym, true);
			}
			break;
		case SDL_KEYUP:
			HandleJoypadKey(joypad, event.key.keysym.sym, false);
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
	
	QueryPerformanceCounter(&FrameStart);
	while (isRunning)
	{

		QueryPerformanceCounter(&StartingTime);

		gameboyStep(&gb);

		if (gb.screen.frameReady)
		{
			gb.screen.frameReady = false;
			ProcessEvents(&gb.joypad);
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