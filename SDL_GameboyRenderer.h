#pragma once
#include <SDL.h>

typedef struct
{
	SDL_Window* window;
	SDL_Renderer* renderer;

} SDl_GameBoyRenderer_t;

bool GameBoyRendererInit(SDl_GameBoyRenderer_t* renderer);

