#ifndef __CHIP8_H__
#define __CHIP8_H__

extern SDL_Surface *surface;

int chip8_loadRom(Chip8 *chip, char *file);
void chip8_reset(Chip8 *chip);
void chip8_generatePallete(uint32_t fg, uint32_t bg);
void vid_flipSurface(Chip8 *chip);
void chip8_doTimers(Chip8 *chip);
void chip8_doInstruction(Chip8 *chip, uint16_t ins);
uint8_t chip8_doEvents(Chip8 *chip, int wait);
uint8_t chip8_getKey(SDLKey key);

#endif //__CHIP8_H__