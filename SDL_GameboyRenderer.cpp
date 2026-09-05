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

	renderer->texture = SDL_CreateTexture(renderer->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT);
	if (!renderer->texture)
	{
		SDL_Log("SDL_CreateTexture failed: %s", SDL_GetError());
		return false;
	}

	return true;
}

void GameBoyRendererPresent(SDl_GameBoyRenderer_t* renderer)
{
	SDL_UpdateTexture(renderer->texture, NULL, renderer->framebuffer, GB_SCREEN_WIDTH * sizeof(uint32_t));

	SDL_RenderClear(renderer->renderer);
	SDL_RenderCopy(renderer->renderer, renderer->texture, NULL, NULL);
	SDL_RenderPresent(renderer->renderer);
}

void GameBoyRendererShutdown(SDl_GameBoyRenderer_t* renderer)
{
	SDL_DestroyTexture(renderer->texture);
	SDL_DestroyRenderer(renderer->renderer);
	SDL_DestroyWindow(renderer->window);
	SDL_Quit();
}
