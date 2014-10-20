#ifndef __SHARED_H__
#define __SHARED_H__

extern uint32_t vid_width;
extern uint32_t vid_height;

typedef struct Chip8
{
	//main chip8 memory
	uint8_t  ram[4096];
	
	//registers
	uint8_t  reg[16];
	
	//RPL Registers
	uint8_t  rpl[8];
	
	//video surface
	uint8_t  vram[128 * 64];
	
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
	
	//hi-res mode
	uint8_t hires;

#ifdef DEBUG
	//Breakpoints
	uint16_t br_list[255];
	uint16_t br_count;
#endif
} Chip8;

#define HI0_4(a) (((a) >> 12) & 0x000F)
#define LO0_4(a) (((a) >> 8)  & 0x000F)
#define HI1_4(a) (((a) >> 4)  & 0x000F)
#define LO1_4(a)  ((a)        & 0x000F)
#define LO_12(a)  ((a)		  & 0x0FFF)
#define LO_8(a)   ((a)		  & 0x00FF)

#define RGB_TO_U32(r, g, b) (((r&0xFF) << 16) | ((g&0xFF) << 8) | (b&0xFF))

uint8_t chip8_doEvents(Chip8 *chip, int wait);

#endif //__SHARED_H__
