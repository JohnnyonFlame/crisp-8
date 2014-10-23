#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>

#include <SDL/SDL.h>

#include "shared.h"
#include "config.h"
#include "token.h"

/*
	DEFAULT GLOBAL SETTINGS
*/

uint32_t vid_fgColors     = RGB_TO_U32(52, 172, 32);
uint32_t vid_bgColors     = RGB_TO_U32(24, 32, 12);
uint32_t vid_stretch      = VID_STRETCH | VID_STRETCH_ASPECT | VID_STRETCH_INTEGER;
int 	 vid_phosphor     = 1;
int      vid_phosphor_add = 160;
int		 vid_phosphor_sub = 24;

int key_binds[16] =
{
	SDLK_x, SDLK_1, SDLK_2, SDLK_3, SDLK_q, SDLK_w, SDLK_e, SDLK_a,
	SDLK_s, SDLK_d, SDLK_z, SDLK_c, SDLK_4, SDLK_r, SDLK_f, SDLK_v
};

enum {
	SETTINGS_INT,
	SETTINGS_STRING,
};

typedef struct Settings
{
	char *name;
	int  type;
	void *data;
} Settings;

Settings settings[] = {
	{
		"vid_fgColors",
		SETTINGS_INT,
		&vid_fgColors
	},
	{
		"vid_bgColors",
		SETTINGS_INT,
		&vid_bgColors
	},
	{
		"vid_stretch",
		SETTINGS_INT,
		&vid_stretch,
	},
	{
		"vid_phosphor",
		SETTINGS_INT,
		&vid_phosphor
	},
	{
		"vid_phosphor_add",
		SETTINGS_INT,
		&vid_phosphor_add
	},
	{
		"vid_phosphor_sub",
		SETTINGS_INT,
		&vid_phosphor_sub
	},
	{
		"key_binds.0",
		SETTINGS_INT,
		&key_binds[0],
	},
	{
		"key_binds.1",
		SETTINGS_INT,
		&key_binds[1],
	},
	{
		"key_binds.2",
		SETTINGS_INT,
		&key_binds[2],
	},
	{
		"key_binds.3",
		SETTINGS_INT,
		&key_binds[3],
	},
	{
		"key_binds.4",
		SETTINGS_INT,
		&key_binds[4],
	},
	{
		"key_binds.5",
		SETTINGS_INT,
		&key_binds[5],
	},
	{
		"key_binds.6",
		SETTINGS_INT,
		&key_binds[6],
	},
	{
		"key_binds.7",
		SETTINGS_INT,
		&key_binds[7],
	},
	{
		"key_binds.8",
		SETTINGS_INT,
		&key_binds[8],
	},
	{
		"key_binds.9",
		SETTINGS_INT,
		&key_binds[9],
	},
	{
		"key_binds.A",
		SETTINGS_INT,
		&key_binds[10],
	},
	{
		"key_binds.B",
		SETTINGS_INT,
		&key_binds[11],
	},
	{
		"key_binds.C",
		SETTINGS_INT,
		&key_binds[12],
	},
	{
		"key_binds.D",
		SETTINGS_INT,
		&key_binds[13],
	},
	{
		"key_binds.E",
		SETTINGS_INT,
		&key_binds[14],
	},
	{
		"key_binds.F",
		SETTINGS_INT,
		&key_binds[15],
	},
	{NULL, 0, NULL}
};

static int settings_find(const char *str)
{
	int i;
	for (i=0;; i++)
	{
		if (settings[i].name == NULL)
			return -1;
		if (!stricmp(settings[i].name, str))
			return i;
	}

	return -1;
}

static int config_matchToken(FILE *file, int type)
{
	Token token;
	return (token_readToken(file, &token) == type);
}

static int config_doSettings(FILE *file, int opt)
{
	if (!config_matchToken(file, TOKEN_EQ))
	{
		printf("Syntax error, unmatched equal token.\n");
		return 1;
	}
	else
	{
		Token value;
		token_readToken(file, &value);

		if (value.type == TOKEN_STRING)
		{
			switch(settings[opt].type)
			{
			case SETTINGS_INT:
				if (!sscanf(value.str, "0x%x", (int*)settings[opt].data))
					if (!sscanf(value.str, "%i", (int*)settings[opt].data))
						printf("Warning, unable to parse value for %s!\n", settings[opt].name);

				break;
			case SETTINGS_STRING:
				strncpy(settings[opt].data, value.str, 80);
				break;
			}
		}
		else
		{
			printf("Syntax error, unexpected token.\n");
			return 1;
		}
	}

	return 0;
}

void config_parseFile(FILE *file)
{
	Token token;
	int opt, stop_parse=0;

	while(!stop_parse)
	{
		token_readToken(file, &token);

		switch(token.type)
		{
		case TOKEN_STRING:
			opt = settings_find(token.str);
			if (opt == -1)
			{
				printf("Warning, unknown setting %s!\n", token.str);
				while ((token_readToken(file, &token) != TOKEN_EOF) && (token.type != TOKEN_NL));
			}
			else
			{
				stop_parse = config_doSettings(file, opt);
			}
			break;
		case TOKEN_NL:
			continue;
		case TOKEN_ERR:
			printf("File parsing aborted.\n");
			break;
		case TOKEN_EOF:
			stop_parse = 1;
			break;
		}
	}
}

void config_loadGlobal()
{
	FILE *config = NULL;
#ifdef WINDOWS
	config = fopen("global.cfg", "r");
#else
	char filename[256];

	//Create folder if it doesn't exist yet.
	snprintf(filename, 256, "%s/.crisp8", getenv("HOME"));
	mkdir(home_folder, S_IRWXU);

	snprintf(filename, 256, "%s/.crisp8/global.cfg", getenv("HOME"));
	config = fopen(filename, "r");
#endif

	if (!config)
	{
		printf("Warning, unable to open global.cfg!\n");
		return;
	}

	config_parseFile(config);
}
