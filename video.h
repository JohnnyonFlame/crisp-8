#ifndef __VID_SHARED_H__
#define __VID_SHARED_H__

int vid_init();
void vid_deinit();
void vid_updateScreen();
void vid_flipSurface(Chip8 *chip);

extern SDL_Surface *vid_surface;

#endif //__VID_SHARED_H__
