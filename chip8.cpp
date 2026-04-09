#include <fstream>
#include <chrono>
#include <random>
#include "chip8.hpp"
#include <cstring>

const unsigned int START_ADDRESS = 0x200;

/// @brief font array : literally has the on and off pixels for the letters and numbers
uint8_t fontset[FONTSET_SIZE] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

Chip8::Chip8() 
		: randGen(std::chrono::system_clock::now().time_since_epoch().count())
{
    // initializing the PC to 200 (cause that is where the instructions start )
    pc = START_ADDRESS;   

    //load fonts into memory
    for (unsigned int i = 0 ; i<FONTSET_SIZE ; ++i){
        memory[FONTSET_START_ADDRESS + i ] = fontset[i];
    }

    //initialize the random number generator 
    randByte = std::uniform_int_distribution<uint8_t>(0,255U);
}

/// @brief loads the instruction set into the memory
/// @param filename 
void Chip8::LoadROM(char const* filename){

    //open the file and move the cursor to the end of file
    // ate = at the end
    std::ifstream file(filename , std::ios::binary | std::ios::ate);

    if (file.is_open()){
        //get the size of the file and allocate a buffer to it 
        std::streampos size = file.tellg();
        char* buffer = new char[size];

        //go back to the beginning to the file and start reading baby !
        file.seekg(0,std::ios::beg);
        file.read(buffer , size);
        file.close();

        //  YOu need to load these data into the chip 8 's memory
        for (long i = 0; i< size ; ++i){
            memory[START_ADDRESS + i] = buffer[i];
        }

        //you need to free the pointers always 
        delete[] buffer;
    }
}

//ALL THE 34 INSTRUCTION IN THE BASE CHIP 8

/// @brief clear the display
void Chip8::OP_00E0(){
        memset(video, 0, sizeof(video));
}

/// @brief return from a subroutine
void Chip8::OP_00EE(){
    // literally just popping from the stack 
    sp--;
    pc = stack[sp];
}

/// @brief jump to location nnn
void Chip8::OP_1nnn(){
    uint16_t address = opcode & 0x0FFFu; // hear from opcode 1250 we extract 250 cause that's the adress
    pc = address;

    // When you "AND" them together, the 0 at the front acts like a wall
    // that blocks the first digit ( the 1), and the Fs act like open
    // windows that let the address (nnn) pass through perfectly.
}

/// @brief call subroutine at nnn
/// @note store the current pc adress in the stack and then increment the stack pointer ...
void Chip8::OP_2nnn(){
    uint16_t address = opcode & 0x0FFFu;

    stack[sp]  = pc;
    sp++;
    pc = address ; 
}

/// @brief skip next instruction if Vx = kk
///@note The Vx is the register name and kk is the byte value (the 1 byte value in memory)
void Chip8::OP_3xkk(){
    uint16_t Vx = (opcode && 0x0F00u);
    uint16_t byte = (opcode & 0x00FFu);

    if ( registers[Vx] == byte){
        pc+=2;
    }
}

/// @brief skip next instruction if Vx != kk
void Chip8::OP_4xkk(){
    uint16_t Vx = (opcode && 0x0F00u);
    uint16_t byte = (opcode & 0x00FFu);

    if ( registers[Vx] != byte){
        pc+=2;
    }
}

/// @brief skip next instruction if Vx = Vy
///@note case where Vx and Vy are both variables and they are the same ...
void Chip8::OP_5xy0(){
    uint16_t Vx = (opcode && 0x0F00u);
    uint16_t Vy = (opcode && 0x00F0u);

    if ( registers[Vx] == registers[Vy]){
        pc+=2;
    }
}

/// @brief set Vx = kk
void Chip8::OP_6xkk(){
    uint16_t Vx = (opcode && 0x0F00u);
    uint16_t byte = (opcode & 0x00FFu);

    registers[Vx] = byte;

}

/// @brief set Vx = Vx + kk
void Chip8::OP_7xkk(){
    uint16_t Vx = (opcode && 0x0F00u);
    uint16_t byte = (opcode & 0x00FFu);

    registers[Vx] += byte;
}

/// @brief set Vx = Vy
void Chip8::OP_8xy0(){
    uint16_t Vx = (opcode && 0x0F00u);
    uint16_t Vy = (opcode && 0x00F0u);

    registers[Vx] = registers[Vy];
}

/// @brief set Vx = Vx OR Vy
void Chip8::OP_8xy1(){
    uint16_t Vx = (opcode && 0x0F00u);
    uint16_t Vy = (opcode && 0x00F0u);

    registers[Vx]  = registers[Vx] | registers[Vy];
}

/// @brief set Vx = Vx AND Vy
void Chip8::OP_8xy2(){
    uint16_t Vx = (opcode && 0x0F00u);
    uint16_t Vy = (opcode && 0x00F0u);  

    registers[Vx]  = registers[Vx] & registers[Vy];
}







    
