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

	//status
	uint8_t status;

	//ROM DATA
	char *rom;
	uint32_t crc_hash;

#ifdef DEBUG
	//Breakpoints
	uint16_t br_list[255];
	uint16_t br_count;
#endif
} Chip8;

enum
{
	CHIP8_DEAD = 0,
	CHIP8_RUNNING,
	CHIP8_PAUSED,
	CHIP8_EXIT,
};

#define HI0_4(a) (((a) >> 12) & 0x000F)
#define LO0_4(a) (((a) >> 8)  & 0x000F)
#define HI1_4(a) (((a) >> 4)  & 0x000F)
#define LO1_4(a)  ((a)        & 0x000F)
#define LO_12(a)  ((a)		  & 0x0FFF)
#define LO_8(a)   ((a)		  & 0x00FF)

#define RGB_TO_U32(r, g, b) (((r&0xFF) << 16) | ((g&0xFF) << 8) | (b&0xFF))

#endif //__SHARED_H__
