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

static SDL_Surface *prev_screen = NULL;

Menu *menu_current = NULL;

void generic_sliderEv(Chip8* chip, SDL_Event *ev, int index)
{
	SLIDER_USERDATA *usr = (SLIDER_USERDATA*)menu_current->entries[index].data;
	int *val = usr->ptr;

	int mod = (SDL_GetModState() & KMOD_LSHIFT) ? usr->step_m : usr->step;
	if (ev->type == SDL_KEYDOWN)
	{
		if (ev->key.keysym.sym == SDLK_LEFT)
			*val = (*val - usr->min < mod) ? usr->min : *val - mod;
		else if (ev->key.keysym.sym == SDLK_RIGHT)
			*val = (*val + mod > usr->max) ? usr->max : *val + mod;
	}
}

void generic_sliderDraw(Chip8 *chip, int index)
{
	SLIDER_USERDATA *usr = (SLIDER_USERDATA*)menu_current->entries[index].data;
	int *val = usr->ptr;

	int sel = (index == menu_current->selected);
	font_renderText(config.fgColor, FONT_CENTERED, vid_surface->w/2, font->surface->h * index,
			"%s%s: %03i%s", (sel) ? "[ " : "",
			usr->label,
			*val,
			(sel) ? " ]" : "");
}

void generic_labelDraw(Chip8 *chip, int index)
{
	font_renderText(config.fgColor, FONT_CENTERED, vid_surface->w/2, font->surface->h * index,
			menu_current->entries[index].data);
}

void generic_buttonDraw(Chip8* chip, int index)
{
	int sel = (index == menu_current->selected);
	font_renderText(config.fgColor, FONT_CENTERED, vid_surface->w/2, font->surface->h * index,
			"%s%s%s", (sel) ? "[ ": "", menu_current->entries[index].data, (sel) ? " ]": "");
}

static void menu_selectNext(Menu *menu)
{
	int i = menu->selected;
	while(menu->entries[i+1].callback_Draw || menu->entries[i+1].callback_Ev)
	{
		i++;

		if (menu->entries[i].callback_Ev)
		{
			menu->selected = i;
			break;
		}
	}

}

static void menu_selectPrevious(Menu *menu)
{
	int i = menu->selected;
	while((i > 0) && (menu->entries[i-1].callback_Draw || menu->entries[i-1].callback_Ev))
	{
		i--;

		if (menu->entries[i].callback_Ev)
		{
			menu->selected = i;
			break;
		}

	}
}

int  menu_doEvents(Chip8 *chip)
{
	SDL_Event ev;
	while (SDL_PollEvent(&ev))
	{
		switch(ev.type)
		{
		case SDL_QUIT:
			chip->status = CHIP8_EXIT;
			break;
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			switch (ev.key.keysym.sym)
			{
			case SDLK_DOWN:
				if (ev.type == SDL_KEYDOWN)
					menu_selectNext(menu_current);
				break;
			case SDLK_UP:
				if (ev.type == SDL_KEYDOWN)
					menu_selectPrevious(menu_current);
				break;
			case SDLK_ESCAPE:
				if ((ev.type == SDL_KEYDOWN) && (chip->status == CHIP8_PAUSED))
					chip8_invokeEmulator(chip);
				break;
			default:
				if ((menu_current) &&
					(menu_current->entries[menu_current->selected].callback_Ev))
					menu_current->entries[menu_current->selected].callback_Ev
						(chip, &ev, menu_current->selected);
				break;
			}
			break;
		default:
			if ((menu_current) &&
				(menu_current->entries[menu_current->selected].callback_Ev))
				menu_current->entries[menu_current->selected].callback_Ev
					(chip, &ev, menu_current->selected);
			break;
		}
	}
	return 1;
}

#define RGBA_LOWERBITS 0xFCFCFCFC
void menu_cacheScreen(Chip8 *chip)
{
	vid_flipSurface(chip);

	//Cache screen
	if (prev_screen)
		SDL_FreeSurface(prev_screen);

	prev_screen = SDL_CreateRGBSurface(0, vid_surface->w, vid_surface->h, vid_surface->format->BitsPerPixel,
			vid_surface->format->Rmask, vid_surface->format->Gmask, vid_surface->format->Bmask, vid_surface->format->Amask);
	SDL_BlitSurface(vid_surface, 0, prev_screen, 0);

	int pitch = (prev_screen->pitch / prev_screen->format->BytesPerPixel) - prev_screen->w;
	uint32_t *img = (uint32_t*)prev_screen->pixels;
	int i, j;

	//Dim the screen cache by a factor.
	for (i=0; i<vid_surface->h; i++)
	{
		for (j=0; j<vid_surface->w; j++)
		{
			*img &= RGBA_LOWERBITS;
			*img >>= 2;
			img++;
		}
		img += pitch;
	}
}


void menu_invokeMenu(Chip8 *chip)
{
	menu_current = &menu_main;

	//We dont want endless annoying sounds, do we?
	if (beeper_status == BEEPER_LOOPING)
		beeper_endLoop();

	menu_cacheScreen(chip);
}

void menu_flipSurface(Chip8 *chip)
{
	SDL_BlitSurface(prev_screen, 0, vid_surface, 0);

	int i = 0;
	while (menu_current->entries[i].callback_Draw != NULL)
	{
		menu_current->entries[i].callback_Draw(chip, i);
		i++;
	}

	SDL_Flip(vid_surface);
}

void menu_doStep(Chip8 **chip)
{
	menu_doEvents(*chip);
	menu_flipSurface(*chip);
	SDL_Delay(33);
}
