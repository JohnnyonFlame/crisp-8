#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL.h>
#include <SDL_image.h>

#include "shared.h"
#include "font.h"

Font *font;

int font_init()
{
	SDL_Surface *temp_img = NULL;
#ifndef WINDOWS
	//Give precedence to anything present on $HOME if possible.
	char font_path[256];
	strncpy(font_path, getenv("HOME"), 256);
	snprintf(font_path, 256, "%s/.crisp8/font.png", getenv("HOME"));
	temp_img = IMG_Load(font_path);
#endif
	if (!temp_img)
		temp_img = IMG_Load("font.png");

	if (!temp_img)
	{
		printf("Error, %s.\n", IMG_GetError());
		return -1;
	}
	else
	{
		font = calloc(1, sizeof(Font));
		if (!font)
			return -1;

		font->surface = SDL_DisplayFormat(temp_img);
		SDL_FreeSurface(temp_img);

		if (!font->surface)
			return -1;

		uint32_t *font_pix = (uint32_t*)font->surface->pixels;

		int i, marker;
		for (i=0, marker=0; i<font->surface->w; i++)
		{
			if (marker > 94)
				break;

			if (((*font_pix++) & 0x00FFFFFF) == FONT_MARKERS)
			{
				font->starts[marker++] = i;
				i++; font_pix++;
			}
		}
	}

	return 1;
}

void font_deinit()
{
	if (font)
	{
		if (font->surface)
			SDL_FreeSurface(font->surface);

		free(font);
	}
}
