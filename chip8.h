#ifndef __CHIP8_H__
#define __CHIP8_H__

extern SDL_Surface *surface;

int chip8_loadRom(Chip8 *chip, char *file);
void chip8_reset(Chip8 *chip);
void chip8_generatePallete(uint32_t fg, uint32_t bg);
void chip8_flipSurface(Chip8 *chip);
void chip8_doTimers(Chip8 *chip);
uint8_t chip8_doEvents(Chip8 *chip, int wait);
uint8_t chip8_getKey(SDLKey key);

#define RGB_TO_U32(r, g, b) (((r&0xFF) << 16) | ((g&0xFF) << 8) | (b&0xFF))

#endif //__CHIP8_H__