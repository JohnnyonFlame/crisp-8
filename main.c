#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <SDL.h>

#include "shared.h"
#include "instruction.h"
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