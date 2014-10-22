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

typedef struct MenuEntry
{
	void (*callback_Ev)(SDL_Event *ev, int index);
	void (*callback_Draw)(int index);
	void *data;
} MenuEntry;

typedef struct Menu
{
	MenuEntry entries[20];
	int selected;
} Menu;

Menu *menu_current = NULL;

static void mainMenu_resumeEv(SDL_Event *ev, int index);
static void mainMenu_resumeDraw(int index);

Menu menu_main =
{
	.entries = {
		{
			mainMenu_resumeEv,
			mainMenu_resumeDraw,
			NULL
		},
		{
			NULL,
			NULL,
			NULL,
		}
	},
	.selected = 0
};

static void mainMenu_resumeEv(SDL_Event *ev, int index)
{
	printf("ok");
}

static void mainMenu_resumeDraw(int index)
{
	printf("ok");
}

static SDL_Surface *prev_screen = NULL;

int  menu_doEvents(Chip8 *chip)
{
	SDL_Event ev;
	while (SDL_PollEvent(&ev))
	{
		switch(ev.type)
		{
		case SDL_QUIT:
			return 0;
			break;
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			switch (ev.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				if ((ev.type == SDL_KEYDOWN) && (chip->status == CHIP8_PAUSED))
					chip8_invokeEmulation(chip);
				break;
			default:

				break;
			}
			break;
		default:
			break;
		}
	}
	return 1;
}

#define RGBA_LOWERBITS 0xFCFCFCFC
void menu_invokeMenu()
{
	menu_current = &menu_main;

	//We dont want endless annoying sounds, do we?
	if (beeper_status == BEEPER_LOOPING)
		beeper_endLoop();

	//Cache screen
	if (prev_screen)
		SDL_FreeSurface(prev_screen);

	prev_screen = SDL_CreateRGBSurface(0, vid_surface->w, vid_surface->h, vid_surface->format->BitsPerPixel,
			vid_surface->format->Rmask, vid_surface->format->Gmask, vid_surface->format->Bmask, vid_surface->format->Amask);
	SDL_BlitSurface(vid_surface, 0, prev_screen, 0);

	int pitch = (prev_screen->pitch / prev_screen->format->BytesPerPixel) - prev_screen->w;
	uint32_t *img = (uint32_t*)prev_screen->pixels;
	int i, j;

	//Dim the screen cache by a two factor.
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

void menu_flipSurface()
{
	SDL_BlitSurface(prev_screen, 0, vid_surface, 0);
	SDL_Flip(vid_surface);
}

int menu_doStep(Chip8 **chip)
{
	int ret;
	ret = menu_doEvents(*chip);
	menu_flipSurface();

	return ret;
}
