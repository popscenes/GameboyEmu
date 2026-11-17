#include "SDL_GameboyAudio.h"

// Upper limit on buffered audio, in tenths of a second. Until the emulator is
// paced by the audio queue, it runs slightly fast and would build up lag.
#define AUDIO_MAX_QUEUED_TENTHS 1

bool GameBoyAudioInit(SDL_GameBoyAudio_t* audio, int sampleRate)
{
	audio->device = 0;
	audio->sampleRate = sampleRate;

	if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
	{
		SDL_Log("SDL_InitSubSystem(audio) failed: %s", SDL_GetError());
		return false;
	}

	SDL_AudioSpec want;
	SDL_zero(want);
	want.freq = sampleRate;
	want.format = AUDIO_F32SYS;
	want.channels = 1;
	want.samples = 1024;
	want.callback = NULL;

	audio->device = SDL_OpenAudioDevice(NULL, 0, &want, NULL, 0);
	if (audio->device == 0)
	{
		SDL_Log("SDL_OpenAudioDevice failed: %s", SDL_GetError());
		return false;
	}

	SDL_PauseAudioDevice(audio->device, 0);
	return true;
}

void GameBoyAudioQueue(SDL_GameBoyAudio_t* audio, const float* samples, uint32_t sampleCount)
{
	if (audio->device == 0 || sampleCount == 0)
	{
		return;
	}

	uint32_t maxQueuedBytes = (audio->sampleRate / 10) * AUDIO_MAX_QUEUED_TENTHS * sizeof(float);
	if (SDL_GetQueuedAudioSize(audio->device) > maxQueuedBytes)
	{
		return;
	}

	SDL_QueueAudio(audio->device, samples, sampleCount * sizeof(float));
}

void GameBoyAudioShutdown(SDL_GameBoyAudio_t* audio)
{
	if (audio->device != 0)
	{
		SDL_CloseAudioDevice(audio->device);
		audio->device = 0;
	}
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}
