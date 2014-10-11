#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

int main(int argc, char *argv[])
{
	if (argc < 2) 
	{
		char *filename = strrchr(argv[0], '\\'); 
		if (!filename)
			filename = strrchr(argv[0], '/'); 
			
		if (*filename != '\0')
			filename++;
		
		printf("Awful Disassembler, Syntax:\n%s FILE\n", filename);
		return -1;
	}

	FILE *f;
	if (!(f = fopen(argv[1], "rb")))
	{
		printf("Error: Unable to open file %s\n", argv[1]);
		return -1;
	}
	
	uint16_t pc = 0x200;
	uint16_t buf;
	while (fread(&buf, sizeof(buf), 1, f))
	{
		//buf = (buf << 8) | ((buf >> 8) & 0xFF);
		printf("0x%04X - 0x%04X\n", pc, buf);
		pc += 2;
	}
	
	fclose(f);
	
	return 0;
}