#ifndef __CONFIG_H__
#define __CONFIG_H__

enum {
	VID_STRETCH = 1 << 0,
	VID_STRETCH_ASPECT = 1 << 1,
	VID_STRETCH_INTEGER = 1 << 2,
};

typedef struct Config {
	int fgColor, bgColor;
	int stretch;
	int phosphor, phosphor_add, phosphor_sub;
	int key_binds[16];
} Config;

extern Config config;

int  config_loadGlobal(Config *cfg);
void config_loadGame(Chip8 *chip, Config *cfg);
void config_saveGlobal(Config *cfg);
void config_saveGame(Chip8* chip, Config *cfg);

#endif //__CONFIG_H__
