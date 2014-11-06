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

static char* colorMenu_title = NULL;
static int * colorMenu_ptr = NULL;
static int   colorMenu_r, colorMenu_g, colorMenu_b;

MenuEntry *entries_color[3];

static void colorMenu_applyChangesEv(Chip8* chip, SDL_Event *ev, int index)
{
	if ((ev->type == SDL_KEYDOWN) && (ev->key.keysym.sym == SDLK_RETURN))
	{
		*colorMenu_ptr = RGB_TO_U32(colorMenu_r, colorMenu_g, colorMenu_b);
		menu_current = &menu_options;
		vid_generatePalette(config.fgColor, config.bgColor);
		menu_cacheScreen(chip);
	}
}

static void colorMenu_cancelEv(Chip8* chip, SDL_Event *ev, int index)
{
	if ((ev->type == SDL_KEYDOWN) && (ev->key.keysym.sym == SDLK_RETURN))
		menu_current = &menu_options;
}


void colorMenu_newLabelDraw(Chip8 *chip, int index)
{
	font_renderText(~RGB_TO_U32(colorMenu_r, colorMenu_g, colorMenu_b),
			FONT_CENTERED, vid_surface->w/2, font->surface->h * index,
			menu_current->entries[index].data);
}

static void colorMenu_colornameLabelDraw(Chip8* chip, int index)
{
	font_renderText(~RGB_TO_U32(colorMenu_r, colorMenu_g, colorMenu_b),
			FONT_CENTERED, vid_surface->w/2, font->surface->h * index, "%s: %08X",
			colorMenu_title, RGB_TO_U32(colorMenu_r, colorMenu_g, colorMenu_b));
}

void colorMenu_newSliderDraw(Chip8 *chip, int index)
{
	SLIDER_USERDATA *usr = (SLIDER_USERDATA*)menu_current->entries[index].data;
	int *val = usr->ptr;

	int sel = (index == menu_current->selected);
	font_renderText(~RGB_TO_U32(colorMenu_r, colorMenu_g, colorMenu_b),
			FONT_CENTERED, vid_surface->w/2, font->surface->h * index,
			"%s%s: %03i%s", (sel) ? "[ " : "",
			usr->label,
			*val,
			(sel) ? " ]" : "");
}

static void colorMenu_drawLabelFirst(Chip8 *chip, int index)
{
	uint32_t *vid = (uint32_t*)vid_surface->pixels;
	int pitch = (vid_surface->pitch / vid_surface->format->BytesPerPixel) - vid_surface->w;
	uint32_t col = RGB_TO_U32(colorMenu_r, colorMenu_g, colorMenu_b);

	int i, j;
	for (i=0; i<vid_surface->h; i++)
	{
		for (j=0; j<vid_surface->w; j++)
		{
			*vid++ = col;
		}
		vid += pitch;
	}

	colorMenu_newLabelDraw(chip, index);
}

void colorMenu_newButtonDraw(Chip8* chip, int index)
{
	int sel = (index == menu_current->selected);
	font_renderText(~RGB_TO_U32(colorMenu_r, colorMenu_g, colorMenu_b),
			FONT_CENTERED, vid_surface->w/2, font->surface->h * index,
			"%s%s%s", (sel) ? "[ ": "", menu_current->entries[index].data, (sel) ? " ]": "");
}


Menu menu_colors =
{
	.entries =
	{
		{
			NULL,
			colorMenu_drawLabelFirst,
			"Crisp-8 Color Picker"
		},
		{
			NULL,
			colorMenu_colornameLabelDraw,
			NULL,
		},
		{
			NULL,
			generic_labelDraw,
			"",
		},
		generic_sliderEv,
		colorMenu_newSliderDraw,
		&(struct SLIDER_USERDATA)
		{
			.min = 0,
			.max = 255,
			.step = 1,
			.step_m = 5,
			.ptr = &(colorMenu_r),
			.label = "Red",
		},
		generic_sliderEv,
		colorMenu_newSliderDraw,
		&(struct SLIDER_USERDATA)
		{
			.min = 0,
			.max = 255,
			.step = 1,
			.step_m = 5,
			.ptr = &(colorMenu_g),
			.label = "Green",
		},
		generic_sliderEv,
		colorMenu_newSliderDraw,
		&(struct SLIDER_USERDATA)
		{
			.min = 0,
			.max = 255,
			.step = 1,
			.step_m = 5,
			.ptr = &(colorMenu_b),
			.label = "Blue",
		},
		{
			colorMenu_applyChangesEv,
			colorMenu_newButtonDraw,
			"Apply"
		},
		{
			colorMenu_cancelEv,
			colorMenu_newButtonDraw,
			"Cancel"
		},
		{NULL, NULL, NULL}
	},
	.selected = 3
};


void colorMenu_invokePicker(char *title, int *color)
{
	colorMenu_title = title;
	colorMenu_ptr = color;
	colorMenu_r = (*color & 0x00FF0000) >> 16;
	colorMenu_g = (*color & 0x0000FF00) >> 8;
	colorMenu_b = (*color & 0x000000FF);

	menu_current = &menu_colors;
}
