#include <stdio.h>
#include <stdint.h>
#include <SDL.h>

#include "shared.h"
#include "chip8.h"
#include "font.h"
#include "beeper.h"

#define SIZE_SPR_W 5
#define SIZE_SPR_H 5

#define PHOSPHOR_DELTA_ADD 160
#define PHOSPHOR_DELTA_SUB 32

SDL_Surface *surface;
SDLKey key_binds[16] = 
{
	SDLK_x, SDLK_1, SDLK_2, SDLK_3, SDLK_q, SDLK_w, SDLK_e, SDLK_a, 
	SDLK_s, SDLK_d, SDLK_z, SDLK_c, SDLK_4, SDLK_r, SDLK_f, SDLK_v
};

static uint32_t chip8_vidPallete[256];

uint8_t chip8_getKey(SDLKey key)
{
	int i;
	for (i=0; i<16; i++)
	{
		if (key_binds[i] == key)
			return i;
	}
	
	return 0xFF;
}

int chip8_loadRom(Chip8 *chip, char *file)
{
	FILE *rom;
	if ((rom = fopen(file, "rb")) == NULL)
	{
		printf("Error: Unable to open file %s\n", file);
		return -1;
	}
	
	chip8_reset(chip);
	fread(&chip->ram[0] + 0x200, 1, 0xFFF - 0x200, rom);
	fclose(rom);
	
	return 1;
}

void chip8_reset(Chip8 *chip)
{
	memset(chip,  0, sizeof(Chip8)); 
	memcpy(&chip->ram[0], font, 16*5);

	chip->ip = 0x200;
	chip->sp = 0;
}

void chip8_generatePallete(uint32_t fg, uint32_t bg)
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

			chip8_vidPallete[i] = 
				   (((fg_coef * fg_r)  & 0x00FF0000)
				+  (((bg_coef * bg_r)  & 0x00FF0000)))
				| ((((fg_coef * fg_g) >> 16) << 8)
				+  (((bg_coef * bg_g) >> 16) << 8))
				| ((((fg_coef * fg_b) >> 16))
				+  (((bg_coef * bg_b) >> 16)));
	}
}

void chip8_flipSurface_fade(Chip8 *chip)
{
	int i, j, m, n, pitch1, pitch2, pitch3, _pitch;
	uint32_t final_color, *scr;
	uint32_t alpha;
	
	scr = (uint32_t *)surface->pixels;
	
	_pitch = surface->pitch/surface->format->BytesPerPixel;
	pitch1 = _pitch - SIZE_SPR_W;
	pitch2 = (_pitch * SIZE_SPR_H) - SIZE_SPR_W;
	pitch3 = (_pitch * SIZE_SPR_H) - (SIZE_SPR_W * VID_WIDTH);
	
	for (i=0; i<VID_HEIGHT; i++)
	{
		for (j=0; j<VID_WIDTH; j++)
		{
			//Find the first pixel of the rect			
			alpha = (*scr >> 24);

			if (chip->vram[(i*VID_WIDTH) + j])
				alpha = (alpha + PHOSPHOR_DELTA_ADD < 255)
							? alpha + PHOSPHOR_DELTA_ADD
                            : 255;
			else
				alpha = (alpha < PHOSPHOR_DELTA_SUB)
							? 0
							: alpha - PHOSPHOR_DELTA_SUB;
			
			//Store our coefficient into the alpha channel
			final_color = (alpha << 24)	| chip8_vidPallete[alpha];
			
			for (m=0; m<SIZE_SPR_H; m++)
			{
				for (n=0; n<SIZE_SPR_W; n++)
				{
					*scr++ = final_color;
				}
				scr += pitch1;
			}
			scr -= pitch2;
		}
		scr += pitch3;
	}
	
	SDL_Flip(surface);
}


void chip8_flipSurface_toggle(Chip8 *chip)
{
	int i, j, m, n, pitch1, pitch2, pitch3, _pitch;
	uint32_t final_color, *scr;
	
	scr = (uint32_t *)surface->pixels;
	
	_pitch = surface->pitch/surface->format->BytesPerPixel;
	pitch1 = _pitch - SIZE_SPR_W;
	pitch2 = (_pitch * SIZE_SPR_H) - SIZE_SPR_W;
	pitch3 = (_pitch * SIZE_SPR_H) - (SIZE_SPR_W * VID_WIDTH);
	
	for (i=0; i<VID_HEIGHT; i++)
	{
		for (j=0; j<VID_WIDTH; j++)
		{	
			final_color = chip8_vidPallete[chip->vram[(i*VID_WIDTH) + j] * 255];
			
			for (m=0; m<SIZE_SPR_H; m++)
			{
				for (n=0; n<SIZE_SPR_W; n++)
				{
					*scr++ = final_color;
				}
				scr += pitch1;
			}
			scr -= pitch2;
		}
		scr += pitch3;
	}
	
	SDL_Flip(surface);
}

void chip8_flipSurface(Chip8 *chip)
{
	if (PHOSPHOR_DELTA_SUB > 129)	//If its more than half, it wont fade
		chip8_flipSurface_toggle(chip);
	else
		chip8_flipSurface_fade(chip);
}

void chip8_doTimers(Chip8 *chip)
{
	static uint32_t time_d = 0, time_p = 0;
	time_d += SDL_GetTicks() - time_p;
	time_p  = SDL_GetTicks();
	
	if ((time_d) / 16)
	{
		time_d -= 16;
		
		if (chip->timer)
			chip->timer--;
			
		if (chip->beeper)
		{
			if (beeper_status == BEEPER_LOOPING)
				chip->beeper--;
			else if (beeper_status == BEEPER_PAUSED)
				beeper_startLoop();
		}
		else if (beeper_status == BEEPER_LOOPING)
			beeper_endLoop();
			
		chip8_flipSurface(chip);
	}
	
	SDL_Delay(1);
}

uint8_t chip8_doEvents(Chip8 *chip, int wait)
{
	uint8_t k;
	SDL_Event ev;
	
	while (1)
	{	
		if (wait)
		{
			//we gotta keep updating the display and counting down timers
			while(!SDL_PollEvent(&ev)) 
				chip8_doTimers(chip);
		}
		else if (!SDL_PollEvent(&ev))
			break;
			
		switch(ev.type)
		{
		case SDL_QUIT:
			exit(0);
			break;
		case SDL_KEYUP:
		case SDL_KEYDOWN:
			k = chip8_getKey(ev.key.keysym.sym);
			
			if (k<=0xF) //16 keys max, otherwise ignore
			{
				chip->key[k] = (ev.type == SDL_KEYDOWN);
				
				if (wait && (ev.type == SDL_KEYDOWN))
				{
					return k;
				}
			}
			break;
		default:
			break;
		}
	}
	
	return 0;
}