#include "SDL_GameboyRenderer.h"

bool GameBoyRendererInit(SDl_GameBoyRenderer_t* renderer)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		SDL_Log("SDL_Init failed: %s", SDL_GetError());
		return false;
	}

	renderer->window = SDL_CreateWindow("GameboyEmu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 576, SDL_WINDOW_BORDERLESS);
	if (!renderer->window)
	{
		SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
		return false;
	}

	renderer->renderer = SDL_CreateRenderer(renderer->window, -1, 0);
	if (!renderer->renderer)
	{
		SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
		return false;
	}

	return true;
}
