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

Config config =
{
	.fgColor = RGB_TO_U32(52, 172, 32), .bgColor = RGB_TO_U32(24, 32, 12),
	.stretch = VID_STRETCH | VID_STRETCH_ASPECT | VID_STRETCH_INTEGER,
	.phosphor = 1, .phosphor_add = 160, .phosphor_sub = 24,
	.key_binds = {
		SDLK_x, SDLK_1, SDLK_2, SDLK_3, SDLK_q, SDLK_w, SDLK_e, SDLK_a,
		SDLK_s, SDLK_d, SDLK_z, SDLK_c, SDLK_4, SDLK_r, SDLK_f, SDLK_v
	}
};

enum {
	SETTINGS_INT,
	SETTINGS_INT_HEX,
	SETTINGS_STRING,
};

typedef struct Settings
{
	char *name;
	int  type;
	void *data;
} Settings;

//Offset of 'a' on struct Config
#define offc(a) ((void*)&(((Config*)0x0)->a))

Settings settings[] = {
	{
		"fg_color",
		SETTINGS_INT_HEX,
		offc(fgColor),
	},
	{
		"bg_color",
		SETTINGS_INT_HEX,
		offc(bgColor),
	},
	{
		"stretch",
		SETTINGS_INT,
		offc(stretch),
	},
	{
		"phosphor",
		SETTINGS_INT,
		offc(phosphor),
	},
	{
		"phosphor_add",
		SETTINGS_INT,
		offc(phosphor_add),
	},
	{
		"phosphor_sub",
		SETTINGS_INT,
		offc(phosphor_sub),
	},
	{
		"key_binds.0",
		SETTINGS_INT,
		offc(key_binds[0]),
	},
	{
		"key_binds.1",
		SETTINGS_INT,
		offc(key_binds[1]),
	},
	{
		"key_binds.2",
		SETTINGS_INT,
		offc(key_binds[2]),
	},
	{
		"key_binds.3",
		SETTINGS_INT,
		offc(key_binds[3]),
	},
	{
		"key_binds.4",
		SETTINGS_INT,
		offc(key_binds[4]),
	},
	{
		"key_binds.5",
		SETTINGS_INT,
		offc(key_binds[5]),
	},
	{
		"key_binds.6",
		SETTINGS_INT,
		offc(key_binds[6]),
	},
	{
		"key_binds.7",
		SETTINGS_INT,
		offc(key_binds[7]),
	},
	{
		"key_binds.8",
		SETTINGS_INT,
		offc(key_binds[8]),
	},
	{
		"key_binds.9",
		SETTINGS_INT,
		offc(key_binds[9]),
	},
	{
		"key_binds.A",
		SETTINGS_INT,
		offc(key_binds[10]),
	},
	{
		"key_binds.B",
		SETTINGS_INT,
		offc(key_binds[11]),
	},
	{
		"key_binds.C",
		SETTINGS_INT,
		offc(key_binds[12]),
	},
	{
		"key_binds.D",
		SETTINGS_INT,
		offc(key_binds[13]),
	},
	{
		"key_binds.E",
		SETTINGS_INT,
		offc(key_binds[14]),
	},
	{
		"key_binds.F",
		SETTINGS_INT,
		offc(key_binds[15]),
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

#define fromoffset(a, b) ((void*)((int)((void*)(a)) + (int)((void*)(b))))

static int config_doSettings(Config *cfg, FILE *file, int opt)
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
			case SETTINGS_INT_HEX:
			case SETTINGS_INT:
				if (!sscanf(value.str, "0x%x", (int*)fromoffset(settings[opt].data, cfg)))
					if (!sscanf(value.str, "%i", (int*)fromoffset(settings[opt].data, cfg)))
						printf("Warning, unable to parse value for %s!\n", settings[opt].name);

				break;
			case SETTINGS_STRING:
				strncpy(fromoffset(settings[opt].data, cfg), value.str, 80);
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

void config_parseFile(Config *cfg, FILE *file)
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
				stop_parse = config_doSettings(cfg, file, opt);
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

int config_loadGlobal(Config *cfg)
{
	FILE *f = NULL;
#ifdef WINDOWS
	f = fopen("global.cfg", "r");
#else
	char filename[256];

	//Create folder if it doesn't exist yet.
	snprintf(filename, 256, "%s/.crisp8", getenv("HOME"));
	mkdir(filename, S_IRWXU);

	snprintf(filename, 256, "%s/.crisp8/global.cfg", getenv("HOME"));
	f = fopen(filename, "r");
#endif

	if (!f)
	{
		printf("Warning, unable to open global.cfg!\n");
		return -1;
	}

	config_parseFile(cfg, f);
	fclose(f);
	return 1;
}

void config_loadGame(Chip8 *chip, Config *cfg)
{
	FILE *f = NULL;
	char filename[256];
#ifdef WINDOWS
	snprintf(filename, 256, "%08X.cfg", chip->crc_hash);
#else
	snprintf(filename, 256, "%s/.crisp8/%08X.cfg", getenv("HOME"), chip->crc_hash);
#endif

	if (!(f = fopen(filename, "r")))
		return;

	config_parseFile(cfg, f);
	fclose(f);
}

void config_saveGlobal(Config *cfg)
{
	FILE *f = NULL;
	char filename[256];
#ifdef WINDOWS
	snprintf(filename, 256, "global.cfg");
#else
	snprintf(filename, 256, "%s/.crisp8/global.cfg", getenv("HOME"));
#endif

	if (!(f = fopen(filename, "w+")))
	{
		printf("Error, Unable to open file %s!", filename);
		return;
	}

	char *c;
	int i;
	for (i=0; settings[i].name != NULL; i++)
	{
		switch(settings[i].type)
		{
		case SETTINGS_INT_HEX:
			fprintf(f, "%s = 0x%08X\n", settings[i].name, *((int*)fromoffset(settings[i].data, cfg)));
			break;
		case SETTINGS_INT:
			fprintf(f, "%s = %i\n", settings[i].name, *((int*)fromoffset(settings[i].data, cfg)));
			break;
		case SETTINGS_STRING:
			for (c = (char*)fromoffset(settings[i].data, cfg); *c!='\0'; c++)
			{
				if (*c=='"')
					fputc('\\', f);
				fputc(*c, f);
			}
			break;
		}
	}

	fclose(f);
}

void config_saveGame(Chip8* chip, Config *cfg)
{
	Config global;
	FILE *f = NULL;
	char filename[256];
#ifdef WINDOWS
	snprintf(filename, 256, "%08X.cfg", chip->crc_hash);
#else
	snprintf(filename, 256, "%s/.crisp8/%08X.cfg", getenv("HOME"), chip->crc_hash);
#endif
	if (!(f = fopen(filename, "w+")))
	{
		printf("Error, Unable to open file %s!", filename);
		return;
	}

	if (!config_loadGlobal(&global))
		return;

	char *c;
	int i, current_int, global_int;
	for (i=0; settings[i].name != NULL; i++)
	{
		switch(settings[i].type)
		{
		case SETTINGS_INT_HEX:
		case SETTINGS_INT:
			current_int = *((int*)fromoffset(settings[i].data, cfg));
			global_int  = *((int*)fromoffset(settings[i].data, &global));

			if (current_int != global_int)
			{
				if (settings[i].type == SETTINGS_INT)
					fprintf(f, "%s = %i\n", settings[i].name, current_int);
				else
					fprintf(f, "%s = 0x%08X\n", settings[i].name, *((int*)fromoffset(settings[i].data, cfg)));
			}

			break;
		case SETTINGS_STRING:
			if (strcmp((char*)fromoffset(settings[i].data, cfg),
					(char*)fromoffset(settings[i].data, &global)))
			{
				for (c = (char*)fromoffset(settings[i].data, cfg); *c!='\0'; c++)
				{
					if (*c=='"')
						fputc('\\', f);

					fputc(*c, f);
				}
			}
			break;
		}
	}

	fclose(f);
}
