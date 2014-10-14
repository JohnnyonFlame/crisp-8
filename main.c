#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <SDL.h>

#include "shared.h"
#include "chip8.h"

int main(int argc, char* argv[])
{
	srand(time(NULL));
	SDL_Init(SDL_INIT_VIDEO);
	surface = SDL_SetVideoMode(320, 240, 32, SDL_SWSURFACE);
	SDL_WM_SetCaption ("SHIT-8", NULL);

	if (argc < 2) 
	{
		char *filename = strrchr(argv[0], '\\'); 
		if (!filename)
			filename = strrchr(argv[0], '/'); 
			
		if (*filename != '\0')
			filename++;
		
		printf("SHIT-8, Syntax:\n%s FILE\n", filename);
		return -1;
	}
	
	Chip8* chip = (Chip8*)calloc(1, sizeof(Chip8));
	chip8_loadRom(chip, argv[1]);
	chip8_generatePallete(RGB_TO_U32(52, 172, 32), RGB_TO_U32(24, 32, 12));
	
	while((chip->ip > 0) && (chip->ip < 0xFFF))
	{	
		chip8_doTimers(chip);
		chip8_doEvents(chip, 0);
		
		//fetch instruction
		uint16_t ins = (chip->ram[chip->ip+1]) | (chip->ram[chip->ip] << 8);
		
		//decode instruction
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
			case 0x00E0: 	//Clears the display
				memset(&chip->vram[0], 0, VID_WIDTH * VID_HEIGHT);
				break;
			case 0x00EE: 	//Returns from subroutines
				chip->ip = chip->stack[--chip->sp];
				break;
			default: 		//Unhandled situation.
				//TODO:: Print warning/error
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
				chip->reg[15] = (chip->reg[r0] > chip->reg[r1]);
				chip->reg[r0] -= chip->reg[r1];
				break;
			case 0x6:	//SHR Va[, Vb]
				chip->reg[15] = (chip->reg[r0] & 0x01);
				chip->reg[r0] >>= 1;
				break;
			case 0x7:	//SUBN Va, Vb
				chip->reg[15] = (chip->reg[r1] > chip->reg[r0]);
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
		{
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
			break;
		}
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
						chip->beeper = chip->reg[r0];
						break;
					case 0x1E: //ADD I, Va
						chip->regi += chip->reg[r0];
						break;
					case 0x29: //LD F, Va
						chip->regi = chip->reg[r0] * 5;
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
					default: 
						break;
				}
				break;
			}
		}
		
		//walk instruction pointer
		chip->ip += 2;
		
		//SDL_Delay(1);
	}
	
	return 0;
}