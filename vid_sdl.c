#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL.h>

#include "shared.h"
#include "config.h"
#include "video.h"
#include "font.h"

uint32_t vid_palette[256];
uint8_t  vid_fadeBuffer[128 * 64];

SDL_Surface *vid_surface = NULL;

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

static __inline__ void vid_fillRect(uint32_t *scr, uint32_t col, int pitch, int w, int h)
{
	int i, j;
	for (i=0; i<h; i++)
	{
		for (j=0; j<w; j++)
		{
			*scr++ = col;
		}
		scr += pitch;
	}
}

#define FIX8_ONE (1<<8)
static void vid_flipSurface_stretch(Chip8 *chip)
{
	int i, j, pitch1, pitch2;
	int scale_w, scale_h, coef_x, coef_y;
	int t_x, t_y;
	uint32_t alpha, *scr;
	uint32_t acc_x = 0, acc_y = 0;

	scr = (uint32_t *)vid_surface->pixels;
	pitch1 = vid_surface->pitch / vid_surface->format->BytesPerPixel;

	//Scaling factors
	scale_w = (vid_surface->w << 8) / vid_width;
	scale_h = (vid_surface->h << 8) / vid_height;

	if (vid_stretch & VID_STRETCH_ASPECT)
	{
		if (scale_w > scale_h)
			scale_w = scale_h;
		else
			scale_h = scale_w;
	}

	if (vid_stretch & VID_STRETCH_INTEGER)
	{
		scale_w &= 0xFFFFFF00;
		scale_h &= 0xFFFFFF00;
	}

	//Coefficients (in fix_8)
	coef_x = (scale_w & 0x000000FF);
	coef_y = (scale_h & 0x000000FF);

	pitch2 = vid_surface->w - ((scale_w * vid_width) >> 8);


	scr += (vid_surface->w - ((scale_w * vid_width) >> 8)) / 2;
	scr += pitch1 * ((vid_surface->h - ((scale_h * vid_height) >> 8)) / 2);

	scale_w >>= 8;
	scale_h >>= 8;

	for (i=0; i<vid_height; i++)
	{
		acc_y += coef_y;
		t_y = acc_y / FIX8_ONE;

		for (j=0; j<vid_width; j++)
		{
			//Accumulate
			acc_x += coef_x;
			t_x = acc_x / FIX8_ONE;

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

			vid_fillRect(scr, vid_palette[alpha], pitch1 - scale_w - t_x, scale_w + t_x, scale_h + t_y);

			scr += scale_w + t_x;
			acc_x = acc_x % FIX8_ONE;
		}

		scr += pitch1 * ((scale_h - 1) + t_y);
		scr += pitch2;
		acc_y = acc_y % FIX8_ONE;
	}
}

static void vid_flipSurface_original(Chip8 *chip)
{
	int i, j, pitch;
	uint32_t alpha, *scr;
	
	scr = (uint32_t *)vid_surface->pixels;
	pitch = (vid_surface->pitch / vid_surface->format->BytesPerPixel) - vid_width;

	scr += (vid_surface->w - vid_width) / 2;
	scr += (pitch + vid_width) * ((vid_surface->h - vid_height) / 2);

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
}


static inline void vid_clearFadeBuffer()
{
	memset(vid_fadeBuffer, 0, 128*64);
}

int vid_init()
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_WM_SetCaption("Crisp-8", NULL);
	SDL_ShowCursor(SDL_DISABLE);

	vid_surface = SDL_SetVideoMode(320, 240, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
	
	if (!vid_surface)
	{
		printf("Error, %s.\n", SDL_GetError());
		return 0;
	}
	
	vid_generatePalette(vid_fgColors, vid_bgColors);
	vid_clearFadeBuffer();
	
	if (font_init() == -1)
		return 0;

	return 1;
}

void vid_deinit()
{
	font_deinit();
	SDL_Quit();
}

void vid_updateSize(uint32_t w, uint32_t h)
{
	vid_clearFadeBuffer();
	//TODO:: Update screen size
}



void vid_flipSurface(Chip8 *chip)
{
	SDL_FillRect(vid_surface, 0, vid_bgColors);

	if (vid_stretch & VID_STRETCH)
		vid_flipSurface_stretch(chip);
	else
		vid_flipSurface_original(chip);

	SDL_Flip(vid_surface);
	//TODO:: Flip surface code
}
