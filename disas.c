#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#define HI0_4(a) (((a) >> 12) & 0x000F)
#define LO0_4(a) (((a) >> 8)  & 0x000F)
#define HI1_4(a) (((a) >> 4)  & 0x000F)
#define LO1_4(a)  ((a)        & 0x000F)
#define LO_12(a)  ((a)		  & 0x0FFF)
#define LO_8(a)   ((a)		  & 0x00FF)

int p_printf(uint32_t pc, const char *fmt, ...)
{
	char msg[81], msg2[81], msg3[81];
	va_list args;
	va_start(args, fmt);
	//TODO: CODE
	vsnprintf(msg, 81, fmt, args);
	snprintf(msg2, 81, ";0x%04X\n", pc);
	snprintf(msg3, 81, "%s%*s", msg, 80-strlen(msg), msg2);
	printf(msg3);
	va_end(args);
}

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
	uint16_t ins;
	while (fread(&ins, sizeof(ins), 1, f))
	{
		ins = (ins << 8) | ((ins >> 8) & 0xFF);
		
		uint16_t addr = LO_12(ins);
		uint8_t r0 = LO0_4(ins);
		uint8_t r1 = HI1_4(ins);
		uint8_t val = LO_8(ins);
		uint8_t op = LO1_4(ins);
	
		switch(HI0_4(ins))
		{	
		case 0x0:
			switch(addr)
			{
				case 0x0E0:
					p_printf(pc, "   CLS");
					break;
				case 0x0EE:
					p_printf(pc, "   RET");
					break;
				default:
					p_printf(pc, ";unknown opcode 0x%04X", ins);
					break;
			}
			break;
		case 0x1:
			p_printf(pc, "   JP 0x%03X", addr);
			break;
		case 0x2:
			p_printf(pc, "   CALL 0x%03X", addr);
			break;
		case 0x3:
			p_printf(pc, "   SE V%x, %i", r0, val);
			break;
		case 0x4:
			p_printf(pc, "   SNE V%x, %i", r0, val);
			break;
		case 0x5:
			p_printf(pc, "   SE V%x, V%x", r0, r1);
			break;
		case 0x6:
			p_printf(pc, "   LD V%x, %i", r0, val);
			break;
		case 0x7:
			p_printf(pc, "   ADD V%x, %i", r0, val);
			break;
		case 0x8:
			switch(op)
			{
			case 0x0:
				p_printf(pc, "   LD V%x, V%x", r0, r1);
				break;
			case 0x1:
				p_printf(pc, "   OR V%x, V%x", r0, r1);
				break;
			case 0x2:
				p_printf(pc, "   AND V%x, V%x", r0, r1);
				break;
			case 0x3:
				p_printf(pc, "   XOR V%x, V%x", r0, r1);
				break;
			case 0x4:
				p_printf(pc, "   ADD V%x, V%x", r0, r1);
				break;
			case 0x5:
				p_printf(pc, "   SUB V%x, V%x", r0, r1);
				break;
			case 0x6:
				p_printf(pc, "   SHR V%x, V%x", r0, r1);
				break;
			case 0x7:
				p_printf(pc, "   SUBN V%x, V%x", r0, r1);
				break;
			case 0xE:
				p_printf(pc, "   SHL V%x, V%x", r0, r1);
				break;		
			default:
				p_printf(pc, ";unknown opcode 0x%4X", ins);
				break;
			}
			break;
		case 0x9:
			p_printf(pc, "   SNE V%x, V%x", r0, r1);
			break;
		case 0xA:
			p_printf(pc, "   LD I, 0x%03X", addr);
			break;
		case 0xB:
			p_printf(pc, "   JP V%x, 0x%03X", r0, addr);
			break;
		case 0xC:
			p_printf(pc, "   RND V%x, %i", r0, val);
			break;
		case 0xD:
			p_printf(pc, "   SHL V%x, V%x, %i", r0, r1, op);
			break;
		case 0xE:            
			switch(val)
			{
				case 0x9E:
					p_printf(pc, "   SKP V%x", r0, val);
					break;
				case 0xA1:
					p_printf(pc, "   SKNP V%x", r0, val);
					break;
				default:
					p_printf(pc, ";unknown opcode 0x%04X", ins);
					break;
			}
			break;
		case 0xF:
			switch(val)
			{
				case 0x07:
					p_printf(pc, "   LD V%x, DT", r0);
					break;
				case 0x0A:
					p_printf(pc, "   LD V%x, K", r0);
					break;
				case 0x15:
					p_printf(pc, "   LD DT, V%x", r0);
					break;
				case 0x18:
					p_printf(pc, "   LD ST, V%x", r0);
					break;
				case 0x1E:
					p_printf(pc, "   ADD I, V%x", r0);
					break;
				case 0x29:
					p_printf(pc, "   FONT V%x", r0);
					break;
				case 0x33:
					p_printf(pc, "   BCD V%x", r0);
					break;
				case 0x55:
					p_printf(pc, "   LOAD V%x", r0);
					break;
				case 0x65:
					p_printf(pc, "   STORE V%x", r0);
					break;
				default: 
					p_printf(pc, ";unknown opcode 0x%04X", ins);
					break;
			}
			break;
		default:
			p_printf(pc, ";unknown opcode 0x%04X", ins);
			break;
		}
		
		pc+=2;
	}
	
	fclose(f);
	
	return 0;
}