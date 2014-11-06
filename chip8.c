#include <stdio.h>
#include <stdint.h>
#include <SDL.h>

#include "shared.h"
#include "chip8.h"
#include "config.h"
#include "font_embedded.h"
#include "beeper.h"
#include "video.h"
#include "menu.h"
#include "crc32.h"

#if defined(DEBUG)
#define printf_debug(fmt, ...) printf("%04X@%04X: " fmt, ins, chip->ip, ##__VA_ARGS__)
#else
#define printf_debug
#endif

uint32_t vid_width  = 64;
uint32_t vid_height = 32;
static uint32_t time_d = 0, time_p = 0;

#ifdef DEBUG
static uint32_t slow = 0;
#endif

uint8_t chip8_getKey(SDLKey key)
{
	int i;
	for (i=0; i<16; i++)
	{
		if (config.key_binds[i] == key)
			return i;
	}
	
	return 0xFF;
}

void chip8_reset(Chip8 *chip)
{
	chip8_loadRom(chip, chip->rom);
}

void chip8_zeroChip(Chip8 *chip)
{
	memset(chip,  0, sizeof(Chip8));
	memcpy(&chip->ram[0],    embedded_fontSmall, 16*5);
	memcpy(&chip->ram[16*5], embedded_fontBig,   16*10);

	chip->hires = 0;
	chip->ip = 0x200;
	chip->sp = 0;
}

//Stops sudden time jumps
void chip8_zeroTimers()
{
	time_p = SDL_GetTicks();
}

void chip8_invokeEmulator(Chip8 *chip)
{
	//Avoid keys getting stuck
	int i;
	for (i=0; i<16; i++)
		chip->key[i] = 0;

	chip8_zeroTimers();
	chip->status = CHIP8_RUNNING;
}

int chip8_loadRom(Chip8 *chip, char *file)
{
	FILE *rom;
	if ((rom = fopen(file, "rb")) == NULL)
	{
		printf("Error: Unable to open file %s\n", file);
		return -1;
	}

	chip8_zeroChip(chip);
	fread(&chip->ram[0] + 0x200, 1, 0xFFF - 0x200, rom);
	fclose(rom);
	
	//Calculate CRC32 rom lookup ID, load appropriate game settings
	chip->crc_hash = crc32(0xDEADBEEF, &chip->ram[0] + 0x200, 0xFFF - 0x200);
	config_loadGame(chip, &config);

	if (((chip->ram[0x200] << 8) | chip->ram[0x201]) == 0x1260)
	{
		vid_height = 64;
		chip->ip = 0x2c0;
	}
	else
	{
		vid_width = 64;
		vid_height = 32;
	}

	if (chip->rom && (strcmp(chip->rom, file)))
	{
		free(chip->rom);
		chip->rom = strdup(file);
	}
	else if (!chip->rom)
		chip->rom = strdup(file);

	chip->status = CHIP8_RUNNING;

	chip8_zeroTimers();
	vid_updateScreen();

	return 1;
}

void chip8_doTimers(Chip8 *chip)
{
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
			
		vid_flipSurface(chip);
	}
	
#ifdef DEBUG
	if (slow)
		SDL_Delay(160);
#endif
	SDL_Delay(1);
}

#ifdef DEBUG
void chip8_invokeDebug(Chip8 *chip)
{
	fflush(stdin);
	char cmd[40];

	while (fscanf(stdin, "%s", cmd))
	{
		if (!strcmp(cmd, "inject"))
		{
			uint32_t ins;
			if (fscanf(stdin, "%04X", &ins))
				chip8_doInstruction(chip, ins);
		}
		else if (!strcmp(cmd, "br"))
		{
			uint32_t addr;
			if (fscanf(stdin, "%03X", &addr))
			{
				chip->br_list[chip->br_count++] = addr & 0x0FFF;
				printf("Added breakpoint %i at %03X\n", chip->br_count-1, addr);
			}

		}
		else if (!strcmp(cmd, "del"))
		{
			int index;
			if (fscanf(stdin, "%03i", &index))
			{
				if (index >= chip->br_count)
				{
					printf("WARNING: Trying to delete breakpoint index out-of-bounds.\n");
					break;
				}

				int i;
				for (i=0; i<chip->br_count-1; i++)
				{
					chip->br_list[i] = chip->br_list[i+1];
				}

				chip->br_count--;
			}
		}
		else if (!strcmp(cmd, "reg"))
		{
			int i;
			for (i=0; i<16; i++)
			{
				printf("REG V%x := dec %05i hex %04X\n", i, chip->reg[i], chip->reg[i]);
			}

			printf("REG I  := dec %05i hex %04X\n", chip->regi, chip->regi);
		}
		else if (!strcmp(cmd, "continue"))
			break;
		else if (!strcmp(cmd, "quit") || !strcmp(cmd, "exit"))
		{
			chip->ip = 0x1000;
			break;
		}
		else
		{
			printf("Unknown command %s.\n", cmd);
			continue;
		}
	}
}
#endif

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
			switch(ev.key.keysym.sym)
			{
#ifdef DEBUG
			case SDLK_SPACE:
				slow = (ev.type == SDL_KEYDOWN);
				break;
			case SDLK_i:
				if (ev.type == SDL_KEYDOWN)
					chip8_invokeDebug(chip);
				break;
#endif
			case SDLK_ESCAPE:
				if (ev.type == SDL_KEYDOWN)
				{
					menu_invokeMenu(chip);
					chip->status = CHIP8_PAUSED;
				}
				break;
			default:
				k = chip8_getKey(ev.key.keysym.sym);

				if (k<=0xF) //16 keys max, otherwise ignore
				{
					chip->key[k] = (ev.type == SDL_KEYDOWN);

					if (wait && (ev.type == SDL_KEYDOWN))
					{
						return k;
					}
				}
			}
			break;
		default:
			break;
		}
	}
	
	return 0;
}

static inline void chip8_clearScreen(Chip8 *chip)
{
	memset(&chip->vram[0], 0, vid_width * vid_height);
}

static inline void chip8_renderSprite_hi(Chip8 *chip, int r0, int r1, int op)
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
			x = (j + chip->reg[r0]) % vid_width;
			y = (i + chip->reg[r1]) % vid_height;
			
			a = (row & (0x8000 >> j)) >> (15-j);
			p = &chip->vram[(y*vid_width) + x];
			b = *p;
			
			if (a & b)
				chip->reg[15] = 1;
				
			*p = a^b;
		}
	}
}

static inline void chip8_renderSprite_lo(Chip8 *chip, int r0, int r1, int op)
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
			x = (j + chip->reg[r0]) % vid_width;
			y = (i + chip->reg[r1]) % vid_height;
			
			a = (row & (0x80 >> j)) >> (7-j);
			p = &chip->vram[(y*vid_width) + x];
			b = *p;
			
			if (a & b)
				chip->reg[15] = 1;
				
			*p = a^b;
		}
	}
}

static inline void chip8_scrollScreen_right(Chip8 *chip)
{
	int i, n;
	uint8_t *vram = &chip->vram[0];
	
	n = (chip->hires) ? 4 : 2;
	
	for (i=0; i<vid_height; i++)
	{
		memmove(vram+n, vram, vid_width-n);
		memset(vram, 0, n);
		vram+=vid_width;
	}
}

static inline void chip8_scrollScreen_left(Chip8 *chip)
{
	int i, n;
	uint8_t *vram = &chip->vram[0];
	
	n = (chip->hires) ? 4 : 2;
	
	for (i=0; i<vid_height; i++)
	{
		memmove(vram, vram+n, vid_width-n);
		vram+=vid_width;
		memset(vram-n, 0, n);
	}
}

void chip8_scrollScreen_down(Chip8 *chip, int n)
{
	uint8_t *vram = &chip->vram[0];
	
	memmove(vram + (vid_width * n), vram, (vid_height-n) * vid_width);
	memset(vram, 0, (vid_width * n));
}

void chip8_doInstruction(Chip8 *chip, uint16_t ins)
{
	uint16_t addr = LO_12(ins);
	uint8_t r0 = LO0_4(ins);
	uint8_t r1 = HI1_4(ins);
	uint8_t val = LO_8(ins);
	uint8_t op = LO1_4(ins);

	int f_set;

	switch(HI0_4(ins))
	{
	//Calls to system subroutines
	//Emulated via high-level calls or simply ignored
	case 0x0:		
		switch(addr)
		{
		case 0x0E0: 	//Clears the display
			printf_debug("CLS\n");
			chip8_clearScreen(chip);
			break;
		case 0x0EE: 	//Returns from subroutines
			printf_debug("RET; <--\n");
			chip->ip = chip->stack[--chip->sp];
			break;
		case 0x0FB:
			printf_debug("SCRR\n");
			chip8_scrollScreen_right(chip);
			break;
		case 0x0FC:
			printf_debug("SCRL\n");
			chip8_scrollScreen_left(chip);
			break;
		case 0x0FD:
			printf_debug("EXIT\n");
			chip->ip = 0x1000; //step outside bounds- kills the interpreter
			break;
		case 0x0FE:
			printf_debug("MODE 1\n");
			chip8_clearScreen(chip);
			vid_width  = 64;
			vid_height = 32;
			vid_updateScreen();
			
			chip->hires = 0;
			break;
		case 0x0FF:
			printf_debug("MODE 2\n");
			chip8_clearScreen(chip);
			vid_width  = 128;
			vid_height = 64;
			vid_updateScreen();
			
			chip->hires = 1;
			break;
		default:
			//TODO:: Print warning/error
			if (addr & 0x0C0)
			{
				printf_debug("SCR %i\n", op);
				chip8_scrollScreen_down(chip, op);
			}
			else
				printf_debug(";Unknown instruction %04X\n", ins);
			break;
		}
		break;
	//JP addr
	case 0x1:
		printf_debug("JP %03Xh ;<--\n", addr, chip->ip);
		chip->ip = addr - 2;
		break;
	//CALL addr
	case 0x2:
		printf_debug("CALL %03Xh ;<--\n", addr);
		chip->stack[chip->sp++] = chip->ip;
		if (chip->sp > 16)
		{
			printf("WARNING! STACK POINTER OUT OF BOUNDS.\n");
			chip->ip = 0x1000;
		}

		chip->ip = addr - 2;
		break;
	//SE Va, byte
	case 0x3:			
		printf_debug("SE V%x, %03i\n", r0, val);
		if (chip->reg[r0] == val)
			chip->ip += 2;
		break;

	//SNE Va, byte
	case 0x4:
		printf_debug("SNE V%x, %03i\n", r0, val);
		if (chip->reg[r0] != val)
			chip->ip += 2;
		break;
	//SE Va, Vb
	case 0x5:	
		printf_debug("SE V%x, V%x\n", r0, r1);
		if (chip->reg[r0] == chip->reg[r1])
			chip->ip += 2;
		break;
	//LD Va, byte
	case 0x6:
		printf_debug("LD V%x, %03i\n", r0, val);
		chip->reg[r0] = val;
		break;
	//ADD Va, byte
	case 0x7:
		printf_debug("ADD V%x [%03i], %03i\n", r0, chip->reg[r0], val);
		chip->reg[r0] += val;
		break;
	//<...>
	case 0x8:
		switch(op)
		{
		case 0x0:	//LD Va, Vb
			printf_debug("LD V%x, V%x\n", r0, r1);
			chip->reg[r0] = chip->reg[r1];
			break;
		case 0x1:	//OR Va, Vb
			printf_debug("OR V%x [%03i], V%x [%03i]\n", r0, chip->reg[r0], r1, chip->reg[r1]);
			chip->reg[r0] |= chip->reg[r1];
			break;
		case 0x2:	//AND Va, Vb
			printf_debug("AND V%x [%03i], V%x [%03i]\n", r0, chip->reg[r0], r1, chip->reg[r1]);
			chip->reg[r0] &= chip->reg[r1];
			break;
		case 0x3:	//XOR Va, Vb
			printf_debug("XOR V%x [%03i], V%x [%03i]\n", r0, chip->reg[r0], r1, chip->reg[r1]);
			chip->reg[r0] ^= chip->reg[r1];
			break;
		case 0x4:	//ADD Va, Vb (w/ carry)
			printf_debug("ADD V%x [%03i], V%x [%03i]\n", r0, chip->reg[r0], r1, chip->reg[r1]);
			f_set = ((chip->reg[r0] + chip->reg[r1]) > 255);
			chip->reg[r0] += chip->reg[r1];
			chip->reg[15] = f_set;
			break;
		case 0x5:	//SUB Va, Vb (w/ borrow)
			printf_debug("SUB V%x [%03i], V%x [%03i]\n", r0, chip->reg[r0], r1, chip->reg[r1]);
			f_set = !(chip->reg[r0] < chip->reg[r1]);
			chip->reg[r0] -= chip->reg[r1];
			chip->reg[15] = f_set;
			break;
		case 0x6:	//SHR Va[, Vb]
			printf_debug("SHR V%x [%02Xh]\n", r0, chip->reg[r0]);
			chip->reg[15] = (chip->reg[r0] & 0x01);
			chip->reg[r0] >>= 1;
			break;
		case 0x7:	//SUBN Va, Vb
			printf_debug("SUBN V%x [%03i], V%x [%03i]\n", r0, chip->reg[r0], r1, chip->reg[r1]);
			f_set = !(chip->reg[r1] < chip->reg[r0]);
			chip->reg[r0] =  chip->reg[r1] - chip->reg[r0];
			chip->reg[15] = f_set;
			break;
		case 0xE:	//SHL Va[, Vb] 
			printf_debug("SHL V%x [%02Xh]\n", r0, chip->reg[r0]);
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
		printf_debug("SNE V%x, V%x\n", r0, r1);
		if (chip->reg[r0] != chip->reg[r1])
			chip->ip += 2;
		break;
	//LD I, addr
	case 0xA:
		printf_debug("LD I, %03Xh\n", r0, addr);
		chip->regi = addr;
		break;
	//JP V0, addr
	case 0xB:
		printf_debug("JP V0, %03Xh\n", addr);
		chip->ip = addr + chip->reg[0] - 2;
		break;
	//RND Va, byte
	case 0xC:
		printf_debug("RAND V%x, %02Xh\n", r0, val);
		chip->reg[r0] = rand() & val;
		break;
	//DRW Va, Vb, N
	case 0xD:
		printf_debug("DRW V%x, V%x, %2i\n", r0, r1, (op) ? op : 16);
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
			printf_debug("SKP V%x\n", r0);
			if (chip->key[chip->reg[r0]])
				chip->ip += 2;
			break;
		case 0xA1: //SKNP Vx
			printf_debug("SKNP V%x\n", r0);
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
					printf_debug("LD V%x, DT\n", r0);
					chip->reg[r0] = chip->timer;
					break;
				case 0x0A: //LD Va, K
					printf_debug("LD V%x, K\n", r0);
					chip->reg[r0] = chip8_doEvents(chip, 1);
					break;
				case 0x15: //LD DT, Va
					printf_debug("LD DT, V%x\n", r0);
					chip->timer = chip->reg[r0];
					break;
				case 0x18: //LD ST, Va
					printf_debug("LD B, V%x\n", r0);
					//Beep for Va frames
					chip->beeper = chip->reg[r0];
					break;
				case 0x1E: //ADD I, Va
					printf_debug("ADD I [%03Xh], V%x [%02Xh]\n", chip->regi, r0, chip->reg[r0]);
					chip->reg[15] = (chip->regi + chip->reg[r0] > 0x0FFF);
					chip->regi += chip->reg[r0];

					if (chip->reg[15])
					{
						chip->regi -= 0xFFF;
					}
					break;
				case 0x29: //LD F, Va
					printf_debug("LD V%x, LF\n", r0);
					chip->regi = chip->reg[r0] * 5;
					break;
				case 0x30:
					printf_debug("LD V%x, HF\n", r0);
					chip->regi = (chip->reg[r0] * 10) + (16*5);
					break;
				case 0x33: //LD B, Va
					printf_debug("LD BCD, V%x\n", r0);
					chip->ram[chip->regi+2] =  chip->reg[r0] % 10;
					chip->ram[chip->regi+1] = (chip->reg[r0] / 10) % 10;
					chip->ram[chip->regi  ] =  chip->reg[r0] / 100;
					break;
				case 0x55: //LD I, Va
					printf_debug("LD I, V%x\n", r0);
					for (i=0; i<=r0; i++)
						chip->ram[chip->regi+i] = chip->reg[i];
					break;
				case 0x65: //LD Va, I
					printf_debug("LD V%x, I\n", r0);
					for (i=0; i<=r0; i++)
						chip->reg[i] = chip->ram[chip->regi+i];
					break;
				case 0x75:
					if (r0 > 0xF)
						r0 = 0xF;
					
					printf_debug("LD VX, V%x\n", r0);
					for (i=0; i<=r0; i++)
						chip->rpl[i] = chip->reg[i];
					break;
				case 0x85:
					if (r0 > 0xF)
						r0 = 0xF;
					
					printf_debug("LD V%x, VX\n", r0);
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

void chip8_doStep(Chip8 *chip)
{
	if (!((chip->ip >= 0x200) && (chip->ip < 0xFFF)))
	{
		chip->status = CHIP8_DEAD;
		return;
	}

	chip8_doEvents(chip, 0);

#ifdef DEBUG
	uint16_t old_ip = chip->ip;
#endif

	//fetch instruction
	uint16_t ins = (chip->ram[chip->ip+1]) | (chip->ram[chip->ip] << 8);

	//Decode & Execute instruction
	chip8_doInstruction(chip, ins);

#ifdef DEBUG
	int i;
	for (i=0; i<chip->br_count; i++)
	{
		if (old_ip == chip->br_list[i])
		{
			printf("Reached breakpoint %i\n", i);
			chip8_invokeDebug(chip);
		}
	}
#endif

	//walk instruction pointer
	chip->ip += 2;

	//Finish frame, deal with timers.
	chip8_doTimers(chip);
}
