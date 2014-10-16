#include <stdio.h>
#include <stdint.h>
#include <SDL.h>

#include "shared.h"
#include "chip8.h"
#include "font.h"
#include "beeper.h"

int VID_WIDTH  = 64;
int VID_HEIGHT = 32;
int SIZE_SPR_W = 4;
int SIZE_SPR_H = 4;

#define PHOSPHOR_DELTA_ADD 160
#define PHOSPHOR_DELTA_SUB 24

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
	
	if (((chip->ram[0x200] << 8) | chip->ram[0x201]) == 0x1260)
	{
		VID_HEIGHT = 64;
		SIZE_SPR_W = 3;
		SIZE_SPR_H = 3;
		chip->ip = 0x2c0;
	}
	
	return 1;
}

void chip8_reset(Chip8 *chip)
{
	memset(chip,  0, sizeof(Chip8)); 
	memcpy(&chip->ram[0],    font,    16*5);
	memcpy(&chip->ram[16*5], bigfont, 16*10);

	chip->hires = 0;
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

__inline__ void chip8_clearScreen(Chip8 *chip)
{
	memset(&chip->vram[0], 0, VID_WIDTH * VID_HEIGHT);
}

void chip8_renderSprite_hi(Chip8 *chip, int r0, int r1, int op)
{	
	//Reset register Vf
	chip->reg[15] = 0;
	
	int i, j, x, y;
	uint8_t a, b, *p;
	for (i=0; i<op; i++)
	{
		uint16_t row = (chip->ram[chip->regi + (i*2)] << 8)
					 | (chip->ram[chip->regi + (i*2) + 1]);
					 
		for (j=0; j<16; j++)
		{
			x = (j + chip->reg[r0]) % VID_WIDTH;
			y = (i + chip->reg[r1]) % VID_HEIGHT;
			
			a = (row & (0x8000 >> j)) >> (15-j);
			p = &chip->vram[(y*VID_WIDTH) + x];
			b = *p;
			
			if (a & b)
				chip->reg[15] = 1;
				
			*p = a^b;
		}
	}
}

void chip8_renderSprite_lo(Chip8 *chip, int r0, int r1, int op)
{
	if (!op)
		op = 16;
		
	//Reset register Vf
	chip->reg[15] = 0;
	
	int i, j, x, y;
	uint8_t a, b, *p;
	for (i=0; i<op; i++)
	{
		uint8_t row = chip->ram[chip->regi + i];
		for (j=0; j<8; j++)
		{
			x = (j + chip->reg[r0]) % VID_WIDTH;
			y = (i + chip->reg[r1]) % VID_HEIGHT;
			
			a = (row & (0x80 >> j)) >> (7-j);
			p = &chip->vram[(y*VID_WIDTH) + x];
			b = *p;
			
			if (a & b)
				chip->reg[15] = 1;
				
			*p = a^b;
		}
	}
}

void chip8_scrollScreen_right(Chip8 *chip)
{
	int i, n;
	uint8_t *vram = &chip->vram[0];
	
	n = (chip->hires) ? 4 : 2;
	
	for (i=0; i<VID_HEIGHT; i++)
	{
		memmove(vram+n, vram, VID_WIDTH-n);
		memset(vram, 0, n);
		vram+=VID_WIDTH;
	}
}

void chip8_scrollScreen_left(Chip8 *chip)
{
	int i, n;
	uint8_t *vram = &chip->vram[0];
	
	n = (chip->hires) ? 4 : 2;
	
	for (i=0; i<VID_HEIGHT; i++)
	{
		memmove(vram, vram+n, VID_WIDTH-n);
		vram+=VID_WIDTH;
		memset(vram-n, 0, n);
	}
}

void chip8_scrollScreen_down(Chip8 *chip, int n)
{
	uint8_t *vram = &chip->vram[0];
	
	memmove(vram + (VID_WIDTH * n), vram, (VID_HEIGHT-n) * VID_WIDTH);
	memset(vram + (VID_WIDTH * (VID_HEIGHT - n)), 0, (VID_WIDTH * n));
}

void chip8_doInstruction(Chip8 *chip, uint16_t ins)
{
	uint16_t addr = LO_12(ins);
	uint8_t r0 = LO0_4(ins);
	uint8_t r1 = HI1_4(ins);
	uint8_t val = LO_8(ins);
	uint8_t op = LO1_4(ins);
	
	switch(HI0_4(ins))
	{
	//Calls to system subroutines
	//Emulated via high-level calls or simply ignored
	case 0x0:		
		switch(addr)
		{
		case 0x0E0: 	//Clears the display
			chip8_clearScreen(chip);
			break;
		case 0x0EE: 	//Returns from subroutines
			chip->ip = chip->stack[--chip->sp];
			break;
		case 0x0FB:
			chip8_scrollScreen_right(chip);
			break;
		case 0x0FC:
			chip8_scrollScreen_left(chip);
			break;
		case 0x0FD:
			chip->ip = 0x1000; //step outside bounds- kills the interpreter
			break;
		case 0x0FE:
			chip8_clearScreen(chip);
			VID_WIDTH  = 64;
			VID_HEIGHT = 32;
			SIZE_SPR_W = 4;
			SIZE_SPR_H = 4;
			
			chip->hires = 0;
			break;
		case 0x0FF:
			chip8_clearScreen(chip);
			VID_WIDTH  = 128;
			VID_HEIGHT = 64;
			SIZE_SPR_W = 2;
			SIZE_SPR_H = 2;
			
			chip->hires = 1;
			break;
		default:
			//TODO:: Print warning/error
			if (addr & 0x0C0)
				chip8_scrollScreen_down(chip, op);
				
			break;
		}
		break;
	//JP addr
	case 0x1:
		chip->ip = addr - 2;
		break;
	//CALL addr
	case 0x2:
		chip->stack[chip->sp++] = chip->ip;
		chip->ip = addr - 2;
		break;
	//SE Va, byte
	case 0x3:			
		if (chip->reg[r0] == val)
			chip->ip += 2;
		break;

	//SNE Va, byte
	case 0x4:
		if (chip->reg[r0] != val)
			chip->ip += 2;
		break;
	//SE Va, Vb
	case 0x5:	
		if (chip->reg[r0] == chip->reg[r1])
			chip->ip += 2;
		break;
	//LD Va, byte
	case 0x6:
		chip->reg[r0] = val;
		break;
	//ADD Va, byte
	case 0x7:
		chip->reg[r0] += val;
		break;
	//<...>
	case 0x8:
		switch(op)
		{
		case 0x0:	//LD Va, Vb
			chip->reg[r0] = chip->reg[r1];
			break;
		case 0x1:	//OR Va, Vb
			chip->reg[r0] |= chip->reg[r1];
			break;
		case 0x2:	//AND Va, Vb
			chip->reg[r0] &= chip->reg[r1];
			break;
		case 0x3:	//XOR Va, Vb
			chip->reg[r0] ^= chip->reg[r1];
			break;
		case 0x4:	//ADD Va, Vb (w/ carry)
			chip->reg[15] = ((chip->reg[r0] + chip->reg[r1]) > 255);			
			chip->reg[r0] += chip->reg[r1];
			break;
		case 0x5:	//SUB Va, Vb (w/ borrow)
			chip->reg[15] = !(chip->reg[r0] < chip->reg[r1]);
			chip->reg[r0] -= chip->reg[r1];
			break;
		case 0x6:	//SHR Va[, Vb]
			chip->reg[15] = (chip->reg[r0] & 0x01);
			chip->reg[r0] >>= 1;
			break;
		case 0x7:	//SUBN Va, Vb
			chip->reg[15] = !(chip->reg[r1] < chip->reg[r0]);
			chip->reg[r0] =  chip->reg[r1] - chip->reg[r0];
			break;
		case 0xE:	//SHL Va[, Vb] 
			chip->reg[15] = (chip->reg[r0] & 0x80) >> 7;
			chip->reg[r0] <<= 1;
			break;
		default:
			//TODO:: Print warning/error
			break;
		}
		break;
	//SNE Va, Vb
	case 0x9:
		if (chip->reg[r0] != chip->reg[r1])
			chip->ip += 2;
		break;
	//LD I, addr
	case 0xA:
		chip->regi = addr;
		break;
	//JP V0, addr
	case 0xB:
		chip->ip = addr + chip->reg[0] - 2;
		break;
	//RND Va, byte
	case 0xC:
		chip->reg[r0] = rand() & val;
		break;
	//DRW Va, Vb, N
	case 0xD:
		if (chip->hires && op == 0)
			chip8_renderSprite_hi(chip, r0, r1, 16);
		else
			chip8_renderSprite_lo(chip, r0, r1, op);
			
		break;
	//<...>
	case 0xE:
		switch(val)
		{
		case 0x9E: //SKP Vx
			if (chip->key[chip->reg[r0]])
				chip->ip += 2;
			break;
		case 0xA1: //SKNP Vx
			if (!chip->key[chip->reg[r0]])
				chip->ip += 2;
			break;
		default:
			//TODO:: Print warning/error
			break;
		}
		break;
	case 0xF:
		{
			int i;
			//TODO:: Implement everything here.
			switch(val)
			{
				case 0x07: //LD Va, DT
					chip->reg[r0] = chip->timer;
					break;
				case 0x0A: //LD Va, K
					chip->reg[r0] = chip8_doEvents(chip, 1);
					break;
				case 0x15: //LD DT, Va
					chip->timer = chip->reg[r0];
					break;
				case 0x18: //LD ST, Va
					//Beep for Va frames
					chip->beeper = chip->reg[r0];
					break;
				case 0x1E: //ADD I, Va
					chip->reg[15] = (chip->regi + chip->reg[r0] > 255);
					chip->regi += chip->reg[r0];
					break;
				case 0x29: //LD F, Va
					chip->regi = chip->reg[r0] * 5;
					break;
				case 0x30:
					chip->regi = (chip->reg[r0] * 10) + (16*5);
					break;
				case 0x33: //LD B, Va
					chip->ram[chip->regi+2] =  chip->reg[r0] % 10;
					chip->ram[chip->regi+1] = (chip->reg[r0] / 10) % 10;
					chip->ram[chip->regi  ] =  chip->reg[r0] / 100;
					break;
				case 0x55: //LD I, Va
					for (i=0; i<=r0; i++)
						chip->ram[chip->regi+i] = chip->reg[i];
					break;
				case 0x65: //LD Va, I
					for (i=0; i<=r0; i++)
						chip->reg[i] = chip->ram[chip->regi+i];
					break;
				case 0x75:
					if (r0 > 7)
						r0 = 0;
					
					for (i=0; i<=r0; i++)
						chip->rpl[i] = chip->reg[i];
					break;
				case 0x85:
					if (r0 > 7)
						r0 = 0;
					
					for (i=0; i<=r0; i++)
						chip->reg[i] = chip->rpl[i];
					break;
				default: 
					break;
			}
			break;
		}
	}
}