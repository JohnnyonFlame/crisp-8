#ifndef __CONFIG_H__
#define __CONFIG_H__

enum {
	VID_STRETCH = 1 << 0,
	VID_STRETCH_ASPECT = 1 << 1,
	VID_STRETCH_INTEGER = 1 << 2,
};

extern uint32_t vid_fgColors;
extern uint32_t vid_bgColors;
extern uint32_t vid_stretch;
extern int 	 	vid_phosphor;
extern int      vid_phosphor_add;
extern int		vid_phosphor_sub;

void config_loadGlobal();

#endif //__CONFIG_H__
