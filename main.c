#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <SDL.h>

#ifdef WINDOWS
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

#include "shared.h"
#include "config.h"
#include "video.h"
#include "chip8.h"
#include "beeper.h"
#include "menu.h"
#include "font.h"

int main(int argc, char* argv[])
{	
#if defined(WINDOWS) && defined(DEBUG)
	//Allocate debugging console
	int conHandle;
	long stdHandle;

	AllocConsole();

    stdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
    conHandle = _open_osfhandle(stdHandle, _O_TEXT);

    *stdout = *_fdopen( conHandle, "w" );

    stdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
    conHandle = _open_osfhandle(stdHandle, _O_TEXT);

    *stdin  = *_fdopen( conHandle, "r" );

    setvbuf( stdout, NULL, _IONBF, 0 );
    setvbuf( stdin, NULL, _IONBF, 0 );
#endif
	if (argc < 2) 
	{
		char *filename = strrchr(argv[0], '\\'); 
		if (!filename)
			filename = strrchr(argv[0], '/'); 
			
		if (*filename != '\0')
			filename++;
		
		printf("Crisp-8, Syntax:\n%s FILE\n", filename);
		return -1;
	}

	if (!vid_init())
		return -1;

	beeper_init();
	config_loadGlobal(&config);

	Chip8* chip = (Chip8*)calloc(1, sizeof(Chip8));
	if (chip8_loadRom(chip, argv[1]) == -1)
	{
		chip->status = CHIP8_EXIT;
		//TODO:: Create helper_sdl.c
		SDL_FillRect(vid_surface, 0, config.bgColor);
		font_renderText(FONT_CENTERED, vid_surface->w/2, 0, "Unable to open %s!\nPlease check if file exists.", argv[1]);
		SDL_Flip(vid_surface);
		SDL_Delay(3000);
	}
	
	while(chip->status != CHIP8_EXIT)
	{
		switch(chip->status)
		{
		case CHIP8_RUNNING:
			chip8_doStep(chip);
			break;
		case CHIP8_DEAD:
		case CHIP8_PAUSED:
			menu_doStep(&chip);
			break;
		default:
			break;
		}
	}
	
	vid_deinit();
	beeper_deinit();
	SDL_Quit();
	
	free(chip);
	
	return 0;
}
