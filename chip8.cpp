#include <fstream>
#include "chip8.hpp";
const unsigned int START_ADDRESS = 0x200;

void Chip8::LoadROM(char const* filename){

    //open the file and move the cursor to the end of file
    // ate = at the end
    std::ifstream file(filename , std::ios::binary | std::ios::ate);
}