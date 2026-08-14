#include <iostream>

using namespace std;

typedef unsigned char BYTE; //char is 1 byte, unsigned cuz 0-255
typedef unsigned short int WORD; // short int is 2 bytes, 0-65535

BYTE m_Game_Memory[0xFFF];
BYTE m_Registers[16];
WORD m_AddressI;
WORD m_ProgramCounter;
vector m_stack;

void CPU_Reset(){
	
	m_AddressI = 0;
	m_ProgramCounter = 0x200;
	memset(m_Registers,0,sizeof(m_Registers));// to set all registers to 0

	FILE *in;
	in = fopen("d:/Own_Projects/CHIP-8_Emulator/INVADERS", "rb");
	fread(&m_Game_Memory[0x200], 0xfff, 1, in);
	fclose(in);
	}

