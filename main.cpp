#include <stdio.h>
#include <stdlib.h>
#include "Cart.h"
#include "cpu.h"
#include "GameBoy.h"
#include <Windows.h>
#include <queue>
using namespace std;


#define        THRD_MESSAGE_SOMEWORK        WM_USER + 1
#define        THRD_MESSAGE_EXIT            WM_USER + 2

DWORD WINAPI ThrdFunc(LPVOID n)
{
	int        TNumber = (int)n;
	while (1)
	{
		MSG    msg;

		BOOL    MsgReturn = GetMessage(&msg, NULL,
			THRD_MESSAGE_SOMEWORK, THRD_MESSAGE_EXIT);

		if (MsgReturn)
		{
			switch (msg.message)
			{
			case THRD_MESSAGE_SOMEWORK:
				printf("Working Message.... for Thread Number\n");
				break;
			case THRD_MESSAGE_EXIT:
				printf("Exit Message.... for Thread Number\n");
				return 0;
			}
		}
	}
	return 0;
}

int main(int argc, char* args[])
{
	

	HANDLE        hThrd;
	DWORD        Id;

	queue<int> gquiz;

	hThrd = CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)ThrdFunc,
		(LPVOID)0, 0, &Id);

	LARGE_INTEGER StartingTime, EndingTime, ElapsedNanoseconds;
	LARGE_INTEGER Frequency;

	char str1[20];
	char buffer[100];
	if (argc < 2)
	{
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
			
			sprintf_s(buffer, "nanoSecsforInst: %lld over elapsed time:  %lld\n", nanoSecsforInst, ElapsedNanoseconds.QuadPart);
			OutputDebugStringA(buffer);
		}
		else
		{
			
			sprintf_s(buffer, "nanoSecsforInst: %lld under elapsed time:  %lld\n", nanoSecsforInst, ElapsedNanoseconds.QuadPart);
			OutputDebugStringA(buffer);
			//while (ElapsedNanoseconds.QuadPart < nanoSecsforInst)
			//{
			//	QueryPerformanceCounter(&EndingTime);
			//	ElapsedNanoseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;

			//	ElapsedNanoseconds.QuadPart *= 1000000000;
			//	ElapsedNanoseconds.QuadPart /= Frequency.QuadPart;
			//}
		}
	}

	printf("press enter to exit\n");
	scanf_s("19%s", str1, (unsigned)_countof(str1));

	return 0;
}