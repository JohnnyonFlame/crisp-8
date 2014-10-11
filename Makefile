SRC=main.c instruction.c font.c chip8.c
OBJ=$(patsubst %.c, %.o, $(SRC))

SDL_CFLAGS=$(shell sdl-config --cflags)
SDL_LDFLAGS=$(shell sdl-config --libs)

CFLAGS+=$(SDL_CFLAGS) -O2 -ggdb -Wall -Wno-missing-braces
LDFLAGS+=$(SDL_LDFLAGS)

ifneq (, $(findstring MINGW32, $(shell uname -s)))
	CFLAGS+=-DWINDOWS
	TARGET=shit8.exe
	CC=mingw32-gcc
endif

ifneq (, $(findstring Linux, $(shell uname -s)))
	TARGET=shit8
	CC=gcc
endif

all: $(OBJ)
	$(CC) -o $(TARGET) $(OBJ) $(LDFLAGS) 

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
	
clean:
	@rm -rf $(OBJ) $(TARGET)