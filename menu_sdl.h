#ifndef __MENU_SDL_H__
#define __MENU_SDL_H__

typedef struct MenuEntry
{
	void (*callback_Ev)(Chip8* chip, SDL_Event *ev, int index);
	void (*callback_Draw)(Chip8* chip, int index);
	void *data;
} MenuEntry;

typedef struct Menu
{
	MenuEntry entries[20];
	int selected;
} Menu;

extern Menu menu_main;
extern Menu menu_options;
extern Menu *menu_current;

typedef struct SLIDER_USERDATA
{
	int step, step_m;
	int min, max;
	int *ptr;
	char *label;
} SLIDER_USERDATA;

void generic_labelDraw(Chip8* chip, int index);
void generic_buttonDraw(Chip8* chip, int index);
void generic_sliderEv(Chip8* chip, SDL_Event *ev, int index);
void generic_sliderDraw(Chip8 *chip, int index);
void colorMenu_invokePicker(char *title, int *color);
void menu_cacheScreen(Chip8 *chip);

#endif //__MENU_SDL_H__/
