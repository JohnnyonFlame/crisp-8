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

void config_loadGlobal();

#endif //__CONFIG_H__
