#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL.h>

#include "shared.h"
#include "config.h"
#include "chip8.h"
#include "beeper.h"
#include "video.h"
#include "font.h"
#include "menu_sdl.h"

static void mainMenu_resumeEv(Chip8* chip, SDL_Event *ev, int index);
static void mainMenu_resetEv(Chip8* chip, SDL_Event *ev, int index);
static void mainMenu_exitEv(Chip8* chip, SDL_Event *ev, int index);

Menu menu_mainMenu =
{
	.entries = {
		{
			NULL,
			generic_labelDraw,
			"Crisp-8 Main Menu",
		},
		{
			NULL,
			generic_labelDraw,
			"",
		},
		{
			mainMenu_resumeEv,
			generic_buttonDraw,
			"Resume Emulator"
		},
		{
			mainMenu_resetEv,
			generic_buttonDraw,
			"Reset Emulator"
		},
		{
			mainMenu_exitEv,
			generic_buttonDraw,
			"Exit Emulator"
		},
		{
			NULL,
			NULL,
			NULL,
		}
	},
	.selected = 2
};

static void mainMenu_resumeEv(Chip8* chip, SDL_Event *ev, int index)
{
	if ((ev->type == SDL_KEYDOWN) && (ev->key.keysym.sym == SDLK_RETURN))
		chip8_invokeEmulator(chip);
}


static void mainMenu_exitEv(Chip8* chip, SDL_Event *ev, int index)
{
	if ((ev->type == SDL_KEYDOWN) && (ev->key.keysym.sym == SDLK_RETURN))
		chip->status = CHIP8_EXIT;
}

static void mainMenu_resetEv(Chip8* chip, SDL_Event *ev, int index)
{
	if ((ev->type == SDL_KEYDOWN) && (ev->key.keysym.sym == SDLK_RETURN))
		chip8_reset(chip);
}
