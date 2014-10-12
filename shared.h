#ifndef __SHARED_H__
#define __SHARED_H__

#define VID_WIDTH  64
#define VID_HEIGHT 32

typedef struct Chip8
{
	//main chip8 memory
	uint8_t  ram[4096];
	
	//registers
	uint8_t  reg[16];
	
	//video surface
	uint8_t  vram[VID_WIDTH * VID_HEIGHT];
	
	//keypad states
	uint8_t  key[16];
	
	//instruction pointer stack
	uint16_t stack[16];
	
	//special I register
	uint16_t regi;
	
	//instruction & stack pointers
	uint16_t ip;
	uint16_t sp;
	
	//sound & delay timers
	uint8_t beeper;
	uint8_t timer;
} Chip8;

#define HI0_4(a) (((a) >> 12) & 0x000F)
#define LO0_4(a) (((a) >> 8)  & 0x000F)
#define HI1_4(a) (((a) >> 4)  & 0x000F)
#define LO1_4(a)  ((a)        & 0x000F)
#define LO_12(a)  ((a)		  & 0x0FFF)
#define LO_8(a)   ((a)		  & 0x00FF)

uint8_t chip8_doEvents(Chip8 *chip, int wait);

#endif //__SHARED_H__