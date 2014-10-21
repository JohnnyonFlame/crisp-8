#ifndef __FONT_H_
#define __FONT_H_

typedef struct {
	uint32_t height;
	uint32_t starts[94];
	SDL_Surface *surface;
} Font;

#define FONT_MARKERS RGB_TO_U32(255, 0,   255)
#define FONT_MASK    RGB_TO_U32(100, 100, 255)

int font_init();
void font_deinit();

extern Font *font;

#endif //__FONT_H__
