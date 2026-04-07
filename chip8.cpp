#include <fstream>
#include "chip8.hpp";
const unsigned int START_ADDRESS = 0x200;

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