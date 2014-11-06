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

static void optionsMenu_saveGlobalEv(Chip8* chip, SDL_Event *ev, int index)
{
	if ((ev->type == SDL_KEYDOWN) && (ev->key.keysym.sym == SDLK_RETURN))
		config_saveGlobal(&config);
}

static void optionsMenu_saveGameEv(Chip8* chip, SDL_Event *ev, int index)
{
	if ((ev->type == SDL_KEYDOWN) && (ev->key.keysym.sym == SDLK_RETURN))
		config_saveGame(chip, &config);
}

static void optionsMenu_hashDraw(Chip8 *chip, int index)
{
	font_renderText(config.fgColor, FONT_CENTERED, vid_surface->w/2, font->surface->h * index, "(ROM ID: %08X)", chip->crc_hash);
}

static void optionsMenu_phosphorSelectEv(Chip8* chip, SDL_Event *ev, int index)
{
	if ((ev->type == SDL_KEYDOWN) && (ev->key.keysym.sym == SDLK_RETURN))
		config.phosphor = !config.phosphor;
}

static void optionsMenu_phosphorSelectDraw(Chip8 *chip, int index)
{
	int sel = (index == menu_current->selected);
	font_renderText(config.fgColor, FONT_CENTERED, vid_surface->w/2, font->surface->h * index,
			"%sPhosphor Effect: %s%s", (sel) ? "[ " : "",
			(config.phosphor) ? "On" : "Off",
			(sel) ? " ]" : "");
}

static void optionsMenu_scaleSelectDraw(Chip8 *chip, int index)
{
	int sel = (index == menu_current->selected);
	font_renderText(config.fgColor, FONT_CENTERED, vid_surface->w/2, font->surface->h * index,
			"%sAspect: %s%s%s", (sel) ? "[ " : "",
			(config.stretch & VID_STRETCH)
			? (config.stretch & VID_STRETCH_ASPECT) ? "Aspect" : "Full"
			: "Off",
			(config.stretch & VID_STRETCH_INTEGER) ? " & Integer" : "",
			(sel) ? " ]" : "");
}

static void optionsMenu_scaleSelectEv(Chip8* chip, SDL_Event *ev, int index)
{
	if ((ev->type == SDL_KEYDOWN) && (ev->key.keysym.sym == SDLK_RETURN))
	{
		if (config.stretch == (VID_STRETCH | VID_STRETCH_INTEGER))
			config.stretch = 0;
		else
		{
			if (!config.stretch)
				config.stretch = VID_STRETCH | VID_STRETCH_ASPECT;
			else if (config.stretch & VID_STRETCH_INTEGER)
				config.stretch ^= VID_STRETCH_INTEGER | VID_STRETCH_ASPECT;
			else
				config.stretch ^= VID_STRETCH_INTEGER;
		}

		menu_cacheScreen(chip);
	}
}

static void optionsMenu_fgEv(Chip8* chip, SDL_Event *ev, int index)
{
	if ((ev->type == SDL_KEYDOWN) && (ev->key.keysym.sym == SDLK_RETURN))
		colorMenu_invokePicker("Foreground Color", &config.fgColor);
}

static void optionsMenu_bgEv(Chip8* chip, SDL_Event *ev, int index)
{
	if ((ev->type == SDL_KEYDOWN) && (ev->key.keysym.sym == SDLK_RETURN))
		colorMenu_invokePicker("Background Color", &config.bgColor);
}

static void optionsMenu_backEv(Chip8* chip, SDL_Event *ev, int index)
{
	if ((ev->type == SDL_KEYDOWN) && (ev->key.keysym.sym == SDLK_RETURN))
		menu_current = &menu_main;
}

Menu menu_options = {
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
			optionsMenu_phosphorSelectEv,
			optionsMenu_phosphorSelectDraw,
			NULL
		},
		{
			generic_sliderEv,
			generic_sliderDraw,
			&(struct SLIDER_USERDATA)
			{
				.min = 0,
				.max = 255,
				.step = 1,
				.step_m = 5,
				.ptr = &(config.phosphor_add),
				.label = "Phosphor Fade In",
			}
		},
		{
				generic_sliderEv,
				generic_sliderDraw,
				&(struct SLIDER_USERDATA)
				{
					.min = 0,
					.max = 255,
					.step = 1,
					.step_m = 5,
					.ptr = &(config.phosphor_sub),
					.label = "Phosphor Fade Out",
				}
		},
		{
			optionsMenu_fgEv,
			generic_buttonDraw,
			"Foreground Color",
		},
		{
			optionsMenu_bgEv,
			generic_buttonDraw,
			"Background Colour",
		},
		{
			NULL,
			generic_labelDraw,
			"",
		},
		{
			optionsMenu_saveGlobalEv,
			generic_buttonDraw,
			"Save to Global Settings",
		},
		{
			optionsMenu_saveGameEv,
			generic_buttonDraw,
			"Save to Game Settings",
		},
		{
			optionsMenu_backEv,
			generic_buttonDraw,
			"Back"
		}

	},
	.selected = 3
};
