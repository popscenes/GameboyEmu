#pragma once
#include <SDL.h>
#include <stdint.h>

#define GB_SCREEN_WIDTH 160
#define GB_SCREEN_HEIGHT 144

typedef struct
{
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* texture;

} SDl_GameBoyRenderer_t;

bool GameBoyRendererInit(SDl_GameBoyRenderer_t* renderer);
void GameBoyRendererPresent(SDl_GameBoyRenderer_t* renderer, const uint32_t* framebuffer);
void GameBoyRendererShutdown(SDl_GameBoyRenderer_t* renderer);


