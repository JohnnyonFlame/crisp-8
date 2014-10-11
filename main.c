#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <SDL.h>

#include "shared.h"
#include "instruction.h"
#include "font.h"

SDL_Surface *surface;
int32_t key_bindings[SDLK_LAST];

void chip8_reset(Chip8 *chip)
{
	memset(&chip->ram[0],  0, 4096); 
	memcpy(&chip->ram[0],  font, 16*5);
	memset(&chip->reg[0],  0, 16);
	memset(&chip->vram[0], 0, VID_WIDTH * VID_HEIGHT);
	memset(&chip->key[0],  0, 16);

	chip->ip = 0x200;
	chip->sp = 0;
}

#define SIZE_SPR_W 5
#define SIZE_SPR_H 5

void chip8_flipSurface(Chip8 *chip)
{
	int i, j, m, n;
	for (i=0; i<VID_HEIGHT; i++)
	{
		for (j=0; j<VID_WIDTH; j++)
		{
			uint32_t *pix = (uint32_t *)surface->pixels;
			pix += (surface->w * i * SIZE_SPR_H) + j * SIZE_SPR_W;
			
			uint32_t p = (*pix >> 24);
			p = (p < 32) ? 0 : p - 32;
			p = (p << 24) | (p << 16) | (p << 8) | p;
			
			uint32_t c = (chip->vram[(i*VID_WIDTH) + j]) ? 0xFF : 0x0;
			c = (c << 24) | (c << 16) | (c << 8) | c;
			
			for (m=0; m<SIZE_SPR_H; m++)
			{
				for (n=0; n<SIZE_SPR_W; n++)
				{
					*pix++ = c | p;
				}
				pix += surface->w - SIZE_SPR_W;
			}
		}
	}
	
	SDL_Flip(surface);
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
	int32_t k;
	SDL_Event ev;
	
	while (1)
	{	
		//continue counting while waiting for key
		if (wait)
			while(!SDL_PollEvent(&ev)) chip8_doTimers(chip);
		else if (!SDL_PollEvent(&ev))
			break;
			
		switch(ev.type)
		{
		case SDL_QUIT:
			exit(0);
			break;
		case SDL_KEYUP:
		case SDL_KEYDOWN:
			k = key_bindings[ev.key.keysym.sym];
			if (k >= 0x0 && k <= 0xF)
			{
				chip->key[k] = (ev.type == SDL_KEYDOWN);
				
				if (wait && (ev.type == SDL_KEYDOWN))
				{
					return (uint8_t)k;
				}
			}
			break;
		default:
			break;
		}
	}
	
	return 0;
}

int main(int argc, char* argv[])
{
	srand(time(NULL));
	SDL_Init(SDL_INIT_VIDEO);
	surface = SDL_SetVideoMode(320, 240, 32, SDL_SWSURFACE);
	SDL_WM_SetCaption ("Shit8", NULL);
	
	memset(key_bindings, -1, sizeof(key_bindings));
	
	key_bindings[SDLK_z] = 0xA;
	key_bindings[SDLK_x] = 0x0;
	key_bindings[SDLK_c] = 0xB;
	key_bindings[SDLK_v] = 0xF;
	
	key_bindings[SDLK_a] = 0x7;
	key_bindings[SDLK_s] = 0x8;
	key_bindings[SDLK_d] = 0x9;
	key_bindings[SDLK_f] = 0xE;
	
	key_bindings[SDLK_q] = 0x4;
	key_bindings[SDLK_w] = 0x5;
	key_bindings[SDLK_e] = 0x6;
	key_bindings[SDLK_r] = 0xD;
	
	key_bindings[SDLK_1] = 0x1;
	key_bindings[SDLK_2] = 0x2;
	key_bindings[SDLK_3] = 0x3;
	key_bindings[SDLK_4] = 0xC;
	
	if (argc < 2) 
	{
		char *filename = strrchr(argv[0], '\\'); 
		if (!filename)
			filename = strrchr(argv[0], '/'); 
			
		if (*filename != '\0')
			filename++;
		
		printf("Shit8, Syntax:\n%s FILE\n", filename);
		return -1;
	}
	
	FILE *rom;
	if ((rom = fopen(argv[1], "rb")) == NULL)
	{
		printf("Error: Unable to open file %s\n", argv[1]);
		return -1;
	}

	Chip8 *chip = calloc(1, sizeof(Chip8));
	
	chip8_reset(chip);
	fread(&chip->ram[0] + 0x200, 1, 0xFFF - 0x200, rom);
	fclose(rom);
	
	while((chip->ip > 0) && (chip->ip < 0xFFF))
	{	
		chip8_doTimers(chip);
		chip8_doEvents(chip, 0);
		
		//fetch instruction
		uint16_t ins = (chip->ram[chip->ip+1]) | (chip->ram[chip->ip] << 8);
		
		//Decode & Execute
		chip8_jumpTable[HI0_4(ins)].instruction(chip, ins);
		
		//walk instruction pointer
		chip->ip += 2;
		SDL_Delay(1);
	}
	
	return 0;
}