#ifndef __CHIP8_H__
#define __CHIP8_H__

extern SDL_Surface *surface;

int chip8_loadRom(Chip8 *chip, char *file);
void chip8_reset(Chip8 *chip);
void chip8_flipSurface(Chip8 *chip);
void chip8_doTimers(Chip8 *chip);
uint8_t chip8_doEvents(Chip8 *chip, int wait);
uint8_t chip8_getKey(SDLKey key);

#define SIZE_SPR_W 5
#define SIZE_SPR_H 5

#endif //__CHIP8_H__