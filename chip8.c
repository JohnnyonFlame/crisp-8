#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <SDL.h>

#include "shared.h"
#include "instruction.h"
#include "chip8.h"
#include "font.h"

SDL_Surface *surface;
SDLKey key_binds[16] = 
{
	SDLK_x, SDLK_1, SDLK_2, SDLK_3, SDLK_q, SDLK_w, SDLK_e, SDLK_a, 
	SDLK_s, SDLK_d, SDLK_z, SDLK_c, SDLK_4, SDLK_r, SDLK_f, SDLK_v
};

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

#define PHOSPHOR_FG_R (25 << 8)
#define PHOSPHOR_FG_G (192 << 8)
#define PHOSPHOR_FG_B (25 << 8)
#define PHOSPHOR_BG_R (25 << 8)
#define PHOSPHOR_BG_G (52 << 8)
#define PHOSPHOR_BG_B (25 << 8)
#define PHOSPHOR_C (255 << 8)

#define PHOSPHOR_DELTA 32

void chip8_flipSurface_fade(Chip8 *chip)
{
	int i, j, m, n;
	for (i=0; i<VID_HEIGHT; i++)
	{
		for (j=0; j<VID_WIDTH; j++)
		{
			uint32_t *pix = (uint32_t *)surface->pixels;
			pix += (surface->w * i * SIZE_SPR_H) + j * SIZE_SPR_W;
			
			//Phosphor fade effect
			uint32_t p = (*pix >> 24);
			p = (p < PHOSPHOR_DELTA) ? 0 : p - PHOSPHOR_DELTA;
			
			uint32_t c = (chip->vram[(i*VID_WIDTH) + j]) ? 0xFF : 0x0;
			uint32_t fg, bg;
			
			fg = (c | p) << 16;
			bg = ((PHOSPHOR_C << 8) - fg) / PHOSPHOR_C;
			fg /= PHOSPHOR_C;
			
			/*				
				RGB Channels are calculated as
					colour = (fg * coef) + (bg * inverse_coef)
				and then they are shifted and or'd, ignore the crazy bitshift
				insanity. it's used to do 'proper' divisions without making use
				of floating point math.
				
				We store the fade coeffient [0-0xFF] on the alpha channel for
				future fade passes.
			*/
			c = ((c|p) << 24)
				| ((((fg * PHOSPHOR_FG_R) >> 16) << 16) +
				   (((bg * PHOSPHOR_BG_R) >> 16) << 16))
				| ((((fg * PHOSPHOR_FG_G) >> 16) << 8) +
				   (((bg * PHOSPHOR_BG_G) >> 16) << 8))
				| ((((fg * PHOSPHOR_FG_B) >> 16)) +
				   (((bg * PHOSPHOR_BG_B) >> 16)));
			
			for (m=0; m<SIZE_SPR_H; m++)
			{
				for (n=0; n<SIZE_SPR_W; n++)
				{
					*pix++ = c;
				}
				pix += surface->w - SIZE_SPR_W;
			}
		}
	}
	
	SDL_Flip(surface);
}

void chip8_flipSurface_toggle(Chip8 *chip)
{
	int i, j, m, n;
	for (i=0; i<VID_HEIGHT; i++)
	{
		for (j=0; j<VID_WIDTH; j++)
		{
			uint32_t *pix = (uint32_t *)surface->pixels;
			pix += (surface->w * i * SIZE_SPR_H) + j * SIZE_SPR_W;
			
			//Ugly as sin ternary
			uint32_t c = (chip->vram[(i*VID_WIDTH) + j]) 
				//Foreground
				? (PHOSPHOR_FG_R << 16)
				| (PHOSPHOR_FG_G)
				| (PHOSPHOR_FG_B >> 8)
				//Background
				: (PHOSPHOR_BG_R << 16)
				| (PHOSPHOR_BG_G)
				| (PHOSPHOR_BG_B >> 8);

			
			for (m=0; m<SIZE_SPR_H; m++)
			{
				for (n=0; n<SIZE_SPR_W; n++)
				{
					*pix++ = c;
				}
				pix += surface->w - SIZE_SPR_W;
			}
		}
	}
	
	SDL_Flip(surface);
}

void chip8_flipSurface(Chip8 *chip)
{
	if (PHOSPHOR_DELTA > 129)	//If its more than half, it wont fade
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
			chip->beeper--;
			
		chip8_flipSurface(chip);
	}	
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