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

static void optionsMenu_backEv(Chip8* chip, SDL_Event *ev, int index);
static void optionsMenu_scaleEnableDraw(Chip8 *chip, int index);
static void optionsMenu_scaleEnableEv(Chip8* chip, SDL_Event *ev, int index);
static void optionsMenu_scaleAspectDraw(Chip8 *chip, int index);
static void optionsMenu_scaleAspectEv(Chip8* chip, SDL_Event *ev, int index);

Menu menu_optionsMenu = {
	.entries =
	{
		{
			NULL,
			generic_labelDraw,
			"Crisp-8 Options",
		},
		{
			NULL,
			generic_labelDraw,
			""
		},
		{
			optionsMenu_scaleEnableEv,
			optionsMenu_scaleEnableDraw,
			NULL
		},
		{
			optionsMenu_scaleAspectEv,
			optionsMenu_scaleAspectDraw,
			NULL
		},
		{
			optionsMenu_backEv,
			generic_buttonDraw,
			"Back"
		}

	},
	.selected = 2
};

static void optionsMenu_scaleEnableDraw(Chip8 *chip, int index)
{
	int sel = (index == menu_current->selected);
	font_renderText(FONT_CENTERED, vid_surface->w/2, font->surface->h * index,
			"%sScaling: %s%s", (sel) ? "[ " : "",
			(vid_stretch & VID_STRETCH) ? "enabled" : "disabled",
			(sel) ? " ]" : "");
}

static void optionsMenu_scaleEnableEv(Chip8* chip, SDL_Event *ev, int index)
{
	if ((ev->type == SDL_KEYDOWN) && (ev->key.keysym.sym == SDLK_RETURN))
		vid_stretch ^= VID_STRETCH;
}

static void optionsMenu_scaleAspectDraw(Chip8 *chip, int index)
{
	int sel = (index == menu_current->selected);
	font_renderText(FONT_CENTERED, vid_surface->w/2, font->surface->h * index,
			"%sAspect: %s%s%s", (sel) ? "[ " : "",
			(vid_stretch & VID_STRETCH_ASPECT) ? "Aspect" : "Full",
			(vid_stretch & VID_STRETCH_INTEGER) ? " & Integer" : "",
			(sel) ? " ]" : "");
}

static void optionsMenu_scaleAspectEv(Chip8* chip, SDL_Event *ev, int index)
{
	if ((ev->type == SDL_KEYDOWN) && (ev->key.keysym.sym == SDLK_RETURN))
	{
		if (vid_stretch & VID_STRETCH_INTEGER)
			vid_stretch ^= VID_STRETCH_INTEGER | VID_STRETCH_ASPECT;
		else
			vid_stretch ^= VID_STRETCH_INTEGER;
	}

}

static void optionsMenu_backEv(Chip8* chip, SDL_Event *ev, int index)
{
	if ((ev->type == SDL_KEYDOWN) && (ev->key.keysym.sym == SDLK_RETURN))
		menu_current = &menu_mainMenu;
}
