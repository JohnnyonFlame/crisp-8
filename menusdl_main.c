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

static void mainMenu_statusDraw(Chip8 *chip, int index)
{
	font_renderText(config.fgColor, FONT_CENTERED, vid_surface->w/2, font->surface->h * index,
			"Status: %s", (chip->status == CHIP8_PAUSED) ? "paused" : "dead" );
}

static void mainMenu_resumeEv(Chip8* chip, SDL_Event *ev, int index)
{
	if ((ev->type == SDL_KEYDOWN) && (ev->key.keysym.sym == SDLK_RETURN))
	{
		if (chip->status == CHIP8_PAUSED)
			chip8_invokeEmulator(chip);
		else
			chip8_reset(chip);
	}
}

static void mainMenu_optionsEv(Chip8* chip, SDL_Event *ev, int index)
{
	if ((ev->type == SDL_KEYDOWN) && (ev->key.keysym.sym == SDLK_RETURN))
		menu_current = &menu_optionsMenu;
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
			mainMenu_statusDraw,
			NULL,
		},
		{
			NULL,
			generic_labelDraw,
			"",
		},
		{
			mainMenu_resumeEv,
			generic_buttonDraw,
			"Resume"
		},
		{
			mainMenu_optionsEv,
			generic_buttonDraw,
			"Options"
		},
		{
			mainMenu_resetEv,
			generic_buttonDraw,
			"Reset"
		},
		{
			mainMenu_exitEv,
			generic_buttonDraw,
			"Exit"
		},
		{
			NULL,
			NULL,
			NULL,
		}
	},
	.selected = 3
};
