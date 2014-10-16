#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL.h>
#include <SDL_image.h>

#include "shared.h"
#include "config.h"
#include "video.h"

uint32_t vid_palette[256];
uint8_t  vid_fadeBuffer[128 * 64];

SDL_Surface *surface = NULL;

void vid_generatePalette(uint32_t fg, uint32_t bg)
{
	uint32_t fg_r, fg_g, fg_b, fg_coef;
	uint32_t bg_r, bg_g, bg_b, bg_coef;

	fg_r = ((fg & 0x00FF0000) >> 16) << 8;
	fg_g = ((fg & 0x0000FF00) >> 8)  << 8; 
	fg_b = ((fg & 0x000000FF))       << 8;
	
	bg_r = ((bg & 0x00FF0000) >> 16) << 8;
	bg_g = ((bg & 0x0000FF00) >> 8)  << 8; 
	bg_b = ((bg & 0x000000FF))       << 8;
	
	uint32_t i;
	for (i=0; i<256; i++)
	{
			//Calculate the color cross-fading coefficients
			fg_coef = ( i       << 16) / 0x0000FF00;
			bg_coef = ((i^0xFF) << 16) / 0x0000FF00;
			
			/*
				RGB Channels are calculated as
						color = (fg * coef) + (bg * inverse_coef)
				and then they are shifted and or'd, ignore the crazy bitshift
				insanity. it's used to do 'proper' divisions without making use
				of floating point math.

				We store the fade coeffient [0-0xFF] on the alpha channel for
				future fade passes.
			*/

			vid_palette[i] = 
				   (((fg_coef * fg_r)  & 0x00FF0000)
				+  (((bg_coef * bg_r)  & 0x00FF0000)))
				| ((((fg_coef * fg_g) >> 16) << 8)
				+  (((bg_coef * bg_g) >> 16) << 8))
				| ((((fg_coef * fg_b) >> 16))
				+  (((bg_coef * bg_b) >> 16)));
	}
}

void vid_flipSurface_fade(Chip8 *chip)
{
	int i, j, pitch;
	uint32_t alpha, *scr;
	
	pitch = (surface->pitch / surface->format->BytesPerPixel)- vid_width;

	scr = (uint32_t *)surface->pixels;

	for (i=0; i<vid_height; i++)
	{
		for (j=0; j<vid_width; j++)
		{
			//Find the first pixel of the rect
			alpha = vid_fadeBuffer[(i*vid_width) + j];
			
			if (chip->vram[(i*vid_width) + j])
				alpha = (alpha + vid_phosphor_add < 255)
						? alpha + vid_phosphor_add
						: 255;
			else
				alpha = (alpha < vid_phosphor_sub)
						? 0
						: alpha - vid_phosphor_sub;

			vid_fadeBuffer[(i*vid_width) + j] = alpha;
			
			*scr++ = vid_palette[alpha];
		}
		
		scr += pitch;
	}

	SDL_Flip(surface);
}


static inline void vid_clearFadeBuffer()
{
	memset(vid_fadeBuffer, 0, 128*64);
}

int vid_init()
{
	SDL_Init(SDL_INIT_VIDEO);
	surface = SDL_SetVideoMode(320, 240, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
	
	if (!surface)
	{
		printf("Error, %s.\n", SDL_GetError());
		return 0;
	}
	
	vid_generatePalette(vid_fgColors, vid_bgColors);
	vid_clearFadeBuffer();
	
	return 1;
}

void vid_deinit()
{
	SDL_Quit();
}

void vid_updateSize(uint32_t w, uint32_t h)
{
	vid_clearFadeBuffer();
	//TODO:: Update screen size
}



void vid_flipSurface(Chip8 *chip)
{
	vid_flipSurface_fade(chip);
	//TODO:: Flip surface code
}