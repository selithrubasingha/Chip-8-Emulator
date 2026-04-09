#include <fstream>
#include <chrono>
#include <random>
#include "chip8.hpp"
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
void Chip8::OP_00E0(){
    
}