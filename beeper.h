#ifndef __BEEPER_H__
#define __BEEPER_H__

typedef enum {
	BEEPER_ERRROR = -1,
	BEEPER_PAUSED,
	BEEPER_LOOPING,
} BEEPER_STATUS;

extern BEEPER_STATUS beeper_status;

void beeper_init();
void beeper_deinit();
void beeper_startLoop();
void beeper_endLoop();

#endif //__BEEPER_H__
