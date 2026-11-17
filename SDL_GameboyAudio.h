#pragma once
#include <SDL.h>
#include <stdint.h>

typedef struct
{
	SDL_AudioDeviceID device;
	int sampleRate;
} SDL_GameBoyAudio_t;

bool GameBoyAudioInit(SDL_GameBoyAudio_t* audio, int sampleRate);
void GameBoyAudioQueue(SDL_GameBoyAudio_t* audio, const float* samples, uint32_t sampleCount);
void GameBoyAudioShutdown(SDL_GameBoyAudio_t* audio);
