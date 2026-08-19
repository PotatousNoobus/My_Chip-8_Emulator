#include <iostream>
#include <bits/stdc++.h>
#include <thread>
#include <chrono>
#include <windows.h>

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
	in = fopen("d:/Own_Projects/CHIP-8_Emulator/ibm-logo.ch8", "rb");
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

void DrawTerminalASCII() {
	std::cout << "\033[H"; // Reset console cursor to top left
	for (int y = 0; y < 32; ++y) {
		for (int x = 0; x < 64; ++x) {
			std::cout << (m_ScreenData[x][y] ? "█" : " ");
		}
		std::cout << "\n";
	}
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
	if (!m_stack.empty()){
		m_ProgramCounter = m_stack.back();
		m_stack.pop_back();
	}

}
void Opcode5XY0(WORD opcode){
int reg_x = opcode & 0x0F00;
int reg_y = opcode & 0x00F0;
reg_x = reg_x >> 8;
reg_y = reg_y >> 4;
if(m_Registers[reg_x]==m_Registers[reg_y]) m_ProgramCounter+=2;
}

void Opcode8XYN(WORD opcode){

//m_Registers[0xF]=1;
int reg_x = opcode & 0x0F00;
int reg_y = opcode & 0x00F0;
reg_x = reg_x >> 8;
reg_y = reg_y >> 4;
int vx = m_Registers[reg_x];
int vy = m_Registers[reg_y];
	int n = opcode & 0x000F;

	switch (n) {
		case 0x0:
			m_Registers[reg_x] = vy;
			break;

		case 0x1:
			m_Registers[reg_x] = vx | vy;
			break;

		case 0x2:
			m_Registers[reg_x] = vx & vy;
			break;

		case 0x3:
			m_Registers[reg_x] = vx ^ vy;
			break;

		case 0x4: {
			int sum = vx + vy;
			m_Registers[reg_x] = sum & 0xFF;
			m_Registers[0xF] = (sum > 255) ? 1 : 0;
			break;
		}

		case 0x5: {
			m_Registers[reg_x] = (vx - vy) & 0xFF;
			m_Registers[0xF] = (vx >= vy) ? 1 : 0;
			break;
		}

		case 0x6: {
			int lsb = vx & 0x1;
			m_Registers[reg_x] = vx >> 1;
			m_Registers[0xF] = lsb;
			break;
		}

		case 0x7: {
			m_Registers[reg_x] = (vy - vx) & 0xFF;
			m_Registers[0xF] = (vy >= vx) ? 1 : 0;
			break;
		}

		case 0xE: {
			int msb = (vx & 0x80) >> 7;
			m_Registers[reg_x] = (vx << 1) & 0xFF;
			m_Registers[0xF] = msb;
			break;
		}

		default:
			break;
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
	DrawTerminalASCII();
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

void Opcode6XNN(WORD opcode) {
	// 6XNN: Set VX to NN
	int reg_x = (opcode & 0x0F00) >> 8;
	m_Registers[reg_x] = opcode & 0x00FF;
}

void Opcode7XNN(WORD opcode) {
	// 7XNN: Add NN to VX (Carry flag is not changed)
	int reg_x = (opcode & 0x0F00) >> 8;
	m_Registers[reg_x] += (opcode & 0x00FF);
}

void OpcodeANNN(WORD opcode) {
	// ANNN: Set Index Register I to NNN
	m_AddressI = opcode & 0x0FFF;
}

void EmulateCycle(){
WORD opcode = next_opcode();

switch(opcode & 0xF000){
	case 0x1000: Opcode1NNN(opcode); break;

	case 0x0000:{
		    switch(opcode & 0x00FF){
		    	case 0x00E0: Opcode00E0(opcode); break;
			    case 0x00EE: Opcode00EE(opcode); break;
		    
		    }
		    
		    }break;

	case 0x2000: Opcode2NNN(opcode); break;
	case 0x5000: Opcode5XY0(opcode); break;
	case 0x6000: Opcode6XNN(opcode); break;
	case 0x7000: Opcode7XNN(opcode); break;
	case 0x8000: Opcode8XYN(opcode); break;
	case 0xA000: OpcodeANNN(opcode); break;
	case 0xD000: OpcodeDXYN(opcode); break;
	case 0xF000: {
		switch(opcode & 0x00FF) {
			case 0x0033: OpcodeFX33(opcode); break;
			case 0x0055: OpcodeFX55(opcode); break;
			default: break;
		}
	} break;

	default: break;

}
}


int main() {

	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	GetConsoleMode(hOut, &dwMode);
	dwMode |= 0x0004;
	SetConsoleMode(hOut, dwMode);

	SetConsoleOutputCP(CP_UTF8);


	CPU_Reset();
	std::cout << "\033[2J";
	while (true) {
		EmulateCycle();
		//DrawTerminalASCII();
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}


	return 0;
}
