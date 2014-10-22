#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <SDL.h>
#include <SDL_image.h>

#include "shared.h"
#include "config.h"
#include "video.h"
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

static int font_getWidth(const char *text, const char *skips)
{
	int width = 0, i;

	for (i=0; (text[i] != '\0')&&(i<1024); i++)
	{
		char c = text[i];
		if (strchr(skips, c))
			break;

		switch(c)
		{
		case ' ':
			width += 3;
			break;
		case '\t':
			width += 6;
			break;
		default:
			width += font->starts[c - '!' + 1] - font->starts[c - '!'] - 1;
			break;
		}
	}

	return width;
}

#define do_xpos_render(a) \
	{\
		switch(pos)\
		{\
		case FONT_CENTERED:\
			_x = x - (font_getWidth(&text[(a)], "\n") / 2);\
			break;\
		case FONT_RIGHT:\
			_x = x - font_getWidth(&text[(a)], "\n");\
			break;\
		case FONT_LEFT:\
			_x = x;\
			break;\
		}\
	}

void font_renderText(int pos, int x, int y, const char *fmt, ...)
{
	char text[1024];
	va_list args;

	va_start(args, fmt);
		vsnprintf(text, 1024, fmt, args);
	va_end(args);

	uint32_t *dst = (uint32_t*)vid_surface->pixels;
	uint32_t *src = (uint32_t*)font->surface->pixels;

	int src_pitch = (font->surface->pitch / vid_surface->format->BytesPerPixel);
	int dst_pitch = (vid_surface->pitch / vid_surface->format->BytesPerPixel);

	int _x = 0, _y = y;
	char c;
	do_xpos_render(0);

	int cx, cw;

	int i, j, k;
	for (i=0; (i!=-1) && (i<1024); i++)
	{
		c = text[i];

		switch(c)
		{
		case '\0':
			i = -2;
			break;
		case '\n':
			do_xpos_render(i+1);
			_y += font->surface->h;
			break;
		case '\t':
			_x += 6;
			break;
		case ' ':
			_x += 3;
			break;
		default:
			if (c < '!')
				c = '?';

			cx = font->starts[c - '!'] + 1;
			cw = font->starts[c - '!' + 1] - font->starts[c - '!'] - 1;

			for (j=0; j<font->surface->h; j++)
			{
				if (j + _y >= vid_surface->h)
					break;
				if (j + _y < 0)
					continue;

				for (k=0; k<cw; k++)
				{
					if ((k + _x >= vid_surface->w) || (k + _x < 0))
						continue;

					if ((*(src + (src_pitch * j) + k + cx) & 0x00FFFFFF) == FONT_MASK)
						*(dst + (dst_pitch * (j + _y)) + k + _x) = vid_fgColors;
				}
			}

			_x += cw;
			break;
		}
	}
}
