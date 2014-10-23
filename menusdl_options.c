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
static void optionsMenu_hashDraw(Chip8 *chip, int index);
static void optionsMenu_scaleSelectDraw(Chip8 *chip, int index);
static void optionsMenu_scaleSelectEv(Chip8* chip, SDL_Event *ev, int index);

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
			optionsMenu_hashDraw,
			NULL,
		},
		{
			NULL,
			generic_labelDraw,
			""
		},
		{
			optionsMenu_scaleSelectEv,
			optionsMenu_scaleSelectDraw,
			NULL
		},
		{
			optionsMenu_backEv,
			generic_buttonDraw,
			"Back"
		}

	},
	.selected = 3
};

static void optionsMenu_hashDraw(Chip8 *chip, int index)
{
	font_renderText(FONT_CENTERED, vid_surface->w/2, font->surface->h * index, "(ROM ID: %08X)", chip->crc_hash);
}

static void optionsMenu_scaleSelectDraw(Chip8 *chip, int index)
{
	int sel = (index == menu_current->selected);
	font_renderText(FONT_CENTERED, vid_surface->w/2, font->surface->h * index,
			"%sAspect: %s%s%s", (sel) ? "[ " : "",
			(vid_stretch & VID_STRETCH)
			? (vid_stretch & VID_STRETCH_ASPECT) ? "Aspect" : "Full"
			: "Off",
			(vid_stretch & VID_STRETCH_INTEGER) ? " & Integer" : "",
			(sel) ? " ]" : "");
}

static void optionsMenu_scaleSelectEv(Chip8* chip, SDL_Event *ev, int index)
{
	if ((ev->type == SDL_KEYDOWN) && (ev->key.keysym.sym == SDLK_RETURN))
	{
		if (vid_stretch == (VID_STRETCH | VID_STRETCH_INTEGER))
			vid_stretch = 0;
		else
		{
			if (!vid_stretch)
				vid_stretch = VID_STRETCH | VID_STRETCH_ASPECT;
			else if (vid_stretch & VID_STRETCH_INTEGER)
				vid_stretch ^= VID_STRETCH_INTEGER | VID_STRETCH_ASPECT;
			else
				vid_stretch ^= VID_STRETCH_INTEGER;
		}
	}

}

static void optionsMenu_backEv(Chip8* chip, SDL_Event *ev, int index)
{
	if ((ev->type == SDL_KEYDOWN) && (ev->key.keysym.sym == SDLK_RETURN))
		menu_current = &menu_mainMenu;
}
