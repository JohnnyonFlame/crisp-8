TARGET=crisp8

DEBUG?=0
SRC=main.c font_embedded.c font.c chip8.c beeper_mixer.c vid_sdl.c config.c menu_sdl.c menusdl_main.c menusdl_options.c menusdl_color.c crc32.c token.c
OBJ=$(patsubst %.c, %.o, $(SRC))

SDL_CFLAGS=$(shell sdl-config --cflags)
SDL_LDFLAGS=$(shell sdl-config --libs)

CFLAGS+=$(SDL_CFLAGS) -O2 -Wall -Wno-missing-braces -Wno-unused-value
LDFLAGS+=$(SDL_LDFLAGS) -lSDL_mixer -lSDL_image

ifneq (, $(findstring MINGW32, $(shell uname -s)))	
	CFLAGS+=-DWINDOWS
	CC=mingw32-gcc
	ifeq (1, $(DEBUG))
		CFLAGS+=-DDEBUG -ggdb
	endif
endif

ifneq (, $(findstring Linux, $(shell uname -s)))
	CC=gcc
endif

all: $(OBJ)
	$(CC) -o $(TARGET) $(OBJ) $(LDFLAGS) 

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
	
clean:
	@rm -rf $(OBJ) $(TARGET)