#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "shared.h"
#include "config.h"

/*
	DEFAULT SETTINGS
*/

uint32_t vid_fgColors     = RGB_TO_U32(52, 172, 32);
uint32_t vid_bgColors     = RGB_TO_U32(24, 32, 12);
uint32_t vid_stretch      = VID_STRETCH | VID_STRETCH_ASPECT;
int 	 vid_phosphor     = 1;
int      vid_phosphor_add = 160;
int		 vid_phosphor_sub = 24;
