#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL.h>

#include "shared.h"
#include "instruction.h"

static void ins_0h(Chip8 *chip, uint16_t ins);
static void ins_1h(Chip8 *chip, uint16_t ins);
static void ins_2h(Chip8 *chip, uint16_t ins);
static void ins_3h(Chip8 *chip, uint16_t ins);
static void ins_4h(Chip8 *chip, uint16_t ins);
static void ins_5h(Chip8 *chip, uint16_t ins);
static void ins_6h(Chip8 *chip, uint16_t ins);
static void ins_7h(Chip8 *chip, uint16_t ins);
static void ins_8h(Chip8 *chip, uint16_t ins);
static void ins_9h(Chip8 *chip, uint16_t ins);
static void ins_Ah(Chip8 *chip, uint16_t ins);
static void ins_Bh(Chip8 *chip, uint16_t ins);
static void ins_Ch(Chip8 *chip, uint16_t ins);
static void ins_Dh(Chip8 *chip, uint16_t ins);
static void ins_Eh(Chip8 *chip, uint16_t ins);
static void ins_Fh(Chip8 *chip, uint16_t ins);

chip_jtable chip8_jumpTable[] = 
{
	ins_0h, ins_1h, ins_2h, ins_3h, ins_4h, ins_5h, ins_6h, ins_7h, 
	ins_8h, ins_9h, ins_Ah, ins_Bh, ins_Ch, ins_Dh, ins_Eh, ins_Fh,
};

//Calls to system subroutines
//Emulated via high-level calls or simply ignored
static void ins_0h(Chip8 *chip, uint16_t ins)
{
	uint16_t addr = LO_12(ins);
	
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
}

//JP addr
static void ins_1h(Chip8 *chip, uint16_t ins)
{
	//Jump
	uint16_t addr = LO_12(ins);
	chip->ip = addr - 2;
}

//CALL addr
static void ins_2h(Chip8 *chip, uint16_t ins)
{
	//Jump and Link
	uint16_t addr = LO_12(ins);
	
	chip->stack[chip->sp++] = chip->ip;
	chip->ip = addr - 2;
}

//SE Va, byte
static void ins_3h(Chip8 *chip, uint16_t ins)
{
	uint8_t r0 = LO0_4(ins);
	uint8_t val = LO_8(ins);
	
	if (chip->reg[r0] == val)
		chip->ip += 2;
}

//SNE Va, byte
static void ins_4h(Chip8 *chip, uint16_t ins)
{
	uint8_t r0 = LO0_4(ins);
	uint8_t val = LO_8(ins);
	
	if (chip->reg[r0] != val)
		chip->ip += 2;
}

//SE Va, Vb
static void ins_5h(Chip8 *chip, uint16_t ins)
{
	uint8_t r0 = LO0_4(ins);
	uint8_t r1 = HI1_4(ins);
	
	if (chip->reg[r0] == chip->reg[r1])
		chip->ip += 2;
}

//LD Va, byte
static void ins_6h(Chip8 *chip, uint16_t ins)
{
	uint8_t r0 = LO0_4(ins);
	uint8_t val = LO_8(ins);
	
	chip->reg[r0] = val;
}

//ADD Va, byte
static void ins_7h(Chip8 *chip, uint16_t ins)
{
	uint8_t r0 = LO0_4(ins);
	uint8_t val = LO_8(ins);
	
	chip->reg[r0] += val;
}

//<...>
static void ins_8h(Chip8 *chip, uint16_t ins)
{
	uint8_t r0 = LO0_4(ins);
	uint8_t r1 = HI1_4(ins);
	uint8_t op = LO1_4(ins);
	
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
}

//SNE Va, Vb
static void ins_9h(Chip8 *chip, uint16_t ins)
{
	uint8_t r0 = LO0_4(ins);
	uint8_t r1 = HI1_4(ins);

	if (chip->reg[r0] != chip->reg[r1])
		chip->ip += 2;
}

//LD I, addr
static void ins_Ah(Chip8 *chip, uint16_t ins)
{
	uint16_t addr = LO_12(ins);
	chip->regi = addr;
}

//JP V0, addr
static void ins_Bh(Chip8 *chip, uint16_t ins)
{
	uint16_t addr = LO_12(ins);
	chip->ip = addr + chip->reg[0] - 2;
}

//RND Va, byte
static void ins_Ch(Chip8 *chip, uint16_t ins)
{
	uint8_t r0 = LO0_4(ins);
	uint8_t val = LO_8(ins);
	
	chip->reg[r0] = rand() & val;
}

//DRW Va, Vb, N
static void ins_Dh(Chip8 *chip, uint16_t ins)
{
	uint8_t r0 = LO0_4(ins);
	uint8_t r1 = HI1_4(ins);
	uint8_t n =  LO1_4(ins);
	
	//Reset register Vf
	chip->reg[15] = 0;
	
	int i, j, x, y;
	uint8_t a, b, *p;
	for (i=0; i<n; i++)
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


//<...>
static void ins_Eh(Chip8 *chip, uint16_t ins)
{
	uint8_t r0 = LO0_4(ins);
	uint8_t val = LO_8(ins);
	
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
}

static void ins_Fh(Chip8 *chip, uint16_t ins)
{
	uint8_t r0 = LO0_4(ins);
	uint8_t val = LO_8(ins);
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
}
