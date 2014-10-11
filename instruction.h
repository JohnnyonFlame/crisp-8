#ifndef __INSTRUCTION_H__
#define __INSTRUCTION_H__

typedef struct
{
	void (*instruction)(Chip8 *chip, uint16_t ins);
} chip_jtable;

extern chip_jtable chip8_jumpTable[];

#endif //__INSTRUCTION_H__