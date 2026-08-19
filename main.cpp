#include <iostream>
#include <bits/stdc++.h>
#include <thread>
#include <chrono>

using namespace std;

typedef unsigned char BYTE; //char is 1 byte, unsigned cuz 0-255
typedef unsigned short int WORD; // short int is 2 bytes, 0-65535

BYTE m_GameMemory[0x1000];
BYTE m_Registers[16];
WORD m_AddressI;
WORD m_ProgramCounter;
vector <WORD> m_stack;
BYTE m_ScreenData[64][32];

void CPU_Reset(){
	
	m_AddressI = 0;
	m_ProgramCounter = 0x200;
	memset(m_Registers,0,sizeof(m_Registers));// to set all registers to 0

	FILE *in;
	in = fopen("d:/Own_Projects/CHIP-8_Emulator/INVADERS", "rb");
	fread(&m_GameMemory[0x200], 0x1000, 1, in);
	fclose(in);
	}

WORD next_opcode(){

WORD res = m_GameMemory[m_ProgramCounter];
res = res<<8;
res=res|m_GameMemory[m_ProgramCounter+1];
m_ProgramCounter+=2;
return res;

}
void Opcode1NNN(WORD opcode){
m_ProgramCounter = opcode & 0x0FFF;

}
void Opcode2NNN(WORD opcode){
m_stack.push_back(m_ProgramCounter);
m_ProgramCounter = opcode & 0x0FFF;

}
void Opcode00E0(WORD opcode){
	memset(m_ScreenData, 0, sizeof(m_ScreenData));
}
void Opcode00EE(WORD opcode){
m_ProgramCounter = m_stack.back();
m_stack.pop_back();

}
void Opcode5XY0(WORD opcode){
int reg_x = opcode & 0x0F00;
int reg_y = opcode & 0x00F0;
reg_x = reg_x >> 8;
reg_y = reg_y >> 4;
if(m_Registers[reg_x]==m_Registers[reg_y]) m_ProgramCounter+=2;
}

void Opcode8XYN(WORD opcode){

m_Registers[0xF]=1; //carry flag =1
int reg_x = opcode & 0x0F00;
int reg_y = opcode & 0x00F0;
reg_x = reg_x >> 8;
reg_y = reg_y >> 4;
int x = m_Registers[reg_x];
int y = m_Registers[reg_y];

if(x<y) m_Registers[0xF] = 0;
else{
m_Registers[reg_x] = x-y;
}

}

void OpcodeDXYN( WORD opcode )
{
     int regx = opcode & 0x0F00 ;
     regx = regx >> 8 ;
     int regy = opcode & 0x00F0 ;
     regy = regy >> 4 ;

     int height = opcode & 0x000F;
     int coordx = m_Registers[regx] ;
     int coordy = m_Registers[regy] ;

     m_Registers[0xf] = 0 ;

     // loop for the amount of vertical lines needed to draw
     for (int yline = 0; yline < height; yline++)
     {
          BYTE data = m_GameMemory[m_AddressI+yline];
          int xpixelinv = 7 ;
          int xpixel = 0 ;
          for(xpixel = 0;xpixel < 8; xpixel++,xpixelinv--)
          {
               int mask = 1 << xpixelinv ;
               if (data & mask)
               {
                    int x = (coordx + xpixel)%64;
                    int y = (coordy + yline)%32 ;
                    if ( m_ScreenData[x][y] == 1 )
                         m_Registers[0xF]=1; //collision
                    m_ScreenData[x][y]^=1 ;
               }
          }
     }
}

void OpcodeFX33(WORD opcode){
int reg_x = opcode & 0x0F00;
reg_x = reg_x>>8;
int val = m_Registers[reg_x];
int hundred = val / 100;
int tens = (val/10)%10;
int units = val%10;
m_GameMemory[m_AddressI] =hundred; 
m_GameMemory[m_AddressI+1] =tens;
m_GameMemory[m_AddressI+2] =units;

}

void OpcodeFX55(WORD opcode){
int reg_x = opcode & 0x0F00;
reg_x>>=8;
for(int i=0; i<=reg_x; i++){
	m_GameMemory[m_AddressI+i] = m_Registers[i]; 

}
m_AddressI = m_AddressI+reg_x+1;

}

void EmulateCycle(){
WORD opcode = next_opcode();

switch(opcode & 0xF000){
	case 0x1000: Opcode1NNN(opcode); break;

	case 0x0000:{
		    switch(opcode & 0x000F){
		    	case 0x0000: Opcode00E0(opcode); break;
			    case 0x000E: Opcode00EE(opcode); break;
		    
		    }
		    
		    }break;

	case 0x2000: Opcode2NNN(opcode); break;
	case 0x5000: Opcode5XY0(opcode); break;
	case 0x8000: Opcode8XYN(opcode); break;
	case 0xD000: OpcodeDXYN(opcode); break;
	case 0xF000: {
		// There are many 'F' opcodes. We isolate the last two digits to know which one.
		switch(opcode & 0x00FF) {
			case 0x0033: OpcodeFX33(opcode); break;
			case 0x0055: OpcodeFX55(opcode); break;
			default: break; 
		}
	} break;


	default: break;

}
}

void DrawTerminalASCII() {
    std::cout << "\033[H"; // Reset console cursor to top left
    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 64; ++x) {
            std::cout << (m_ScreenData[x][y] ? "█" : " ");
        }
        std::cout << "\n";
    }
}

int main() {
	CPU_Reset();
	std::cout << "\033[2J";
	while (true) {
		EmulateCycle();
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}


	return 0;
}
