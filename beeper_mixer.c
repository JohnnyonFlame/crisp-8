#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_mixer.h>

#include "shared.h"
#include "beeper.h"

#define BEEPER_LOOPCHANNEL 	1

Mix_Chunk *beeper_loop = NULL;
int beeper_status = BEEPER_PAUSED;

void beeper_init()
{
	if (Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 1, 1024 ) == -1)
	{
		printf("Error: Unable to initialize audio, %\n.", Mix_GetError());
		beeper_status = BEEPER_ERRROR;
		return;
	}
	
#ifndef WINDOWS
	//Give precedence to anything present on $HOME if possible.
	char loop_path[256];
	strncpy(path, getenv("HOME"), 256);
	snprintf(loop_path, 256, "%s/.shit8/loop.wav", getenv("HOME"));
	
	if ((beeper_loop = Mix_LoadWAV(loop_path)) != NULL)
		return;
#endif
	if ((beeper_loop = Mix_LoadWAV("loop.wav")) == NULL)
	{
		printf("Error, %s.\n", Mix_GetError());
		beeper_status = BEEPER_ERRROR;
	}
}

void beeper_startLoop()
{
	if (beeper_loop)
			Mix_PlayChannel(BEEPER_LOOPCHANNEL, beeper_loop, -1);
		
	beeper_status = BEEPER_LOOPING;
}

void beeper_endLoop()
{
	if (beeper_loop)
		beeper_status = Mix_HaltChannel(BEEPER_LOOPCHANNEL);
	
	beeper_status = BEEPER_PAUSED;
}

void beeper_deinit()
{
	while( Mix_Init(0) )
		Mix_Quit();
}