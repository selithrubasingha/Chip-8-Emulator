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

        /*
        storing the 34 instruction is done using several tables ...
        it is like a tree structure where in the opcode code if the first hexNum is nto 
        0,8,E or F then we fo to the next subTables ... otherwise we can directly get the instructions . 

        */

        table[0x0] = &Chip8::Table0;
        table[0x1] = &Chip8::OP_1nnn;
        table[0x2] = &Chip8::OP_2nnn;
        table[0x3] = &Chip8::OP_3xkk;
        table[0x4] = &Chip8::OP_4xkk;
        table[0x5] = &Chip8::OP_5xy0;
        table[0x6] = &Chip8::OP_6xkk;
        table[0x7] = &Chip8::OP_7xkk;
        table[0x8] = &Chip8::Table8;
        table[0x9] = &Chip8::OP_9xy0;
        table[0xA] = &Chip8::OP_Annn;   
        table[0xB] = &Chip8::OP_Bnnn;
        table[0xC] = &Chip8::OP_Cxkk;
        table[0xD] = &Chip8::OP_Dxyn;
        table[0xE] = &Chip8::TableE;
        table[0xF] = &Chip8::TableF;    

        //initializing the sub-tables with the corresponding functions
        for (size_t i = 0; i < 0xE + 1 ; ++i){
            table0[i] = &Chip8::OP_NULL;
            table8[i] = &Chip8::OP_NULL;
            tableE[i] = &Chip8::OP_NULL;
        }

        table0[0x0] = &Chip8::OP_00E0;
        table0[0xE] = &Chip8::OP_00EE;

        table8[0x0] = &Chip8::OP_8xy0;
        table8[0x1] = &Chip8::OP_8xy1;
        table8[0x2] = &Chip8::OP_8xy2;
        table8[0x3] = &Chip8::OP_8xy3;
        table8[0x4] = &Chip8::OP_8xy4;
        table8[0x5] = &Chip8::OP_8xy5;
        table8[0x6] = &Chip8::OP_8xy6;
        table8[0x7] = &Chip8::OP_8xy7;
        table8[0xE] = &Chip8::OP_8xyE;

        tableE[0x1] = &Chip8::OP_Ex9E;
        tableE[0xA] = &Chip8::OP_ExA1;

        //the F table is the biggest one so we initialize it with a loop and then we fill the rest of the values with the corresponding functions
        // the F table has two variables the last two hex nums ... it could theoretically go upto FF which is 255
        // but in the instruction set the max is 65 which is 102 in decimal . 
        for (size_t i = 0; i < 0x65 + 1 ; ++i){
            tableF[i] = &Chip8::OP_NULL;
        }

        tableF[0x07] = &Chip8::OP_Fx07;
        tableF[0x0A] = &Chip8::OP_Fx0A;
        tableF[0x15] = &Chip8::OP_Fx15;
        tableF[0x18] = &Chip8::OP_Fx18;
        tableF[0x1E] = &Chip8::OP_Fx1E;
        tableF[0x29] = &Chip8::OP_Fx29;
        tableF[0x33] = &Chip8::OP_Fx33;
        tableF[0x55] = &Chip8::OP_Fx55;
        tableF[0x65] = &Chip8::OP_Fx65;

    }
}


void Chip8::Table0() {
    (this->*table0[opcode & 0x000Fu])();
}

void Chip8::Table8() {
    (this->*table8[opcode & 0x000Fu])();
}

void Chip8::TableE() {
    (this->*tableE[opcode & 0x000Fu])();
}

void Chip8::TableF() {
    (this->*tableF[opcode & 0x00FFu])();
}

void Chip8::OP_NULL() {
    // Do nothing for unknown opcodes
}

//cycle
void Chip8::Cycle(){
    //fetch the opcode

    opcode = (memory[pc] << 8u) | memory[pc+1];

    pc += 2; //increment the pc by 2 to point to the next instruction (cause each instruction is 2 bytes long)

    //decode and execute the opcode 

    //this induces a chain reaction of method calls . remember the () at teh end calls methods
    (this->*table[(opcode & 0xF000u) >> 12u])();

    //update timers
    if (delayTimer > 0){
        delayTimer--;
    }
    if (soundTimer > 0){
        soundTimer--;
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
    uint16_t Vx = (opcode && 0x0F00u) >> 8u; // we need to shift it to the right by 8 bits to get the value of x (the register number)
    uint16_t byte = (opcode & 0x00FFu);

    if ( registers[Vx] == byte){
        pc+=2;
    }
}

/// @brief skip next instruction if Vx != kk
void Chip8::OP_4xkk(){
    uint16_t Vx = (opcode && 0x0F00u) >> 8u;
    uint16_t byte = (opcode & 0x00FFu);

    if ( registers[Vx] != byte){
        pc+=2;
    }
}

/// @brief skip next instruction if Vx = Vy
///@note case where Vx and Vy are both variables and they are the same ...
void Chip8::OP_5xy0(){
    uint16_t Vx = (opcode && 0x0F00u) >> 8u;
    uint16_t Vy = (opcode && 0x00F0u) >> 4u;

    if ( registers[Vx] == registers[Vy]){
        pc+=2;
    }
}

/// @brief set Vx = kk
void Chip8::OP_6xkk(){
    uint16_t Vx = (opcode && 0x0F00u) >> 8u;
    uint16_t byte = (opcode & 0x00FFu);

    registers[Vx] = byte;

}

/// @brief set Vx = Vx + kk
void Chip8::OP_7xkk(){
    uint16_t Vx = (opcode && 0x0F00u) >> 8u;
    uint16_t byte = (opcode & 0x00FFu);

    registers[Vx] += byte;
}

/// @brief set Vx = Vy
void Chip8::OP_8xy0(){
    uint16_t Vx = (opcode && 0x0F00u) >> 8u;
    uint16_t Vy = (opcode && 0x00F0u) >> 4u;

    registers[Vx] = registers[Vy];
}

/// @brief set Vx = Vx OR Vy
void Chip8::OP_8xy1(){
    uint16_t Vx = (opcode && 0x0F00u) >> 8u;
    uint16_t Vy = (opcode && 0x00F0u) >> 4u;

    registers[Vx]  = registers[Vx] | registers[Vy];
}

/// @brief set Vx = Vx AND Vy
void Chip8::OP_8xy2(){
    uint16_t Vx = (opcode && 0x0F00u) >> 8u;
    uint16_t Vy = (opcode && 0x00F0u) >> 4u;

    registers[Vx]  = registers[Vx] & registers[Vy];
}

/// @brief set Vx = Vx XOR Vy
void Chip8::OP_8xy3(){
    uint16_t Vx = (opcode && 0x0F00u) >> 8u;
    uint16_t Vy = (opcode && 0x00F0u) >> 4u;

    registers[Vx]  = registers[Vx] ^ registers[Vy];
}

/// @brief set Vx = Vx + Vy, set VF = carry
/// @note note the 16th register is conventionally used for different flags in this case the carrry flag
void Chip8::OP_8xy4(){
    uint16_t Vx = (opcode && 0x0F00u) >> 8u;
    uint16_t Vy = (opcode && 0x00F0u) >> 4u;

    uint16_t sum = registers[Vx] + registers[Vy];
    if ( sum > 255U){
        registers[0xF] = 1; // carry
    }else{
        registers[0xF] = 0;
    }

    registers[Vx] = sum & 0xFFu; // we need to get the last 8 bits of the sum (cause the register can only hold 8 bits)
}

/// @brief set Vx = Vx - Vy, set VF = NOT borrow
/// @note if Vx > Vy then there is no borrow and we set VF to 1 else we set it to 0 , in this case the VF register acts as a NOT burrow flag 
void Chip8::OP_8xy5(){
    uint16_t Vx = (opcode && 0x0F00u) >> 8u;
    uint16_t Vy = (opcode && 0x00F0u) >> 4u;    

    //NOTE that no burrow makes the flag 1 , it's easier for the computer logic that way becasue of 2's complement and stuff
    if ( registers[Vx] > registers[Vy]){
        registers[0xF] = 1; // no borrow
    }else{
        registers[0xF] = 0; // borrow   
    }   

    registers[Vx] = registers[Vx] - registers[Vy];
}

/// @brief set Vx = Vx SHR 1
/// @note we need to save the least significant bit before shifting to the right because it will be lost after the shift and we need it for the flag register VF
void Chip8::OP_8xy6(){
    uint16_t Vx = (opcode && 0x0F00u) >> 8u;

    //save the LSB into the flag register VF
    registers[0xF] = registers[Vx] & 0x1u;

    //devide the number by 2 ( shift right by 1 !)(shifting to the right does the same thing as dividing by 2 !!!) 
    registers[Vx] = registers[Vx] >> 1;

}

/// @brief set Vx = Vy - Vx, set VF = NOT borrow
/// @note same as SUB Vx,Vy , but its Vy-Vx * (not the other way around)
void Chip8::OP_8xy7(){
    uint16_t Vx = (opcode && 0x0F00u) >> 8u;
    uint16_t Vy = (opcode && 0x00F0u) >> 4u;

    if ( registers[Vy] > registers[Vx]){
        registers[0xF] = 1; // no borrow
    }else{
        registers[0xF] = 0; // borrow
    }

    registers[Vx] = registers[Vy] - registers[Vx];
}

/// @brief set Vx = Vx SHL 1
/// @note If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0. Then Vx is multiplied by 2.
void Chip8::OP_8xyE(){
    uint16_t Vx = (opcode && 0x0F00u) >> 8u;

    //save the MSB into the flag register VF
    registers[0xF] = (registers[Vx] & 0x80u) >> 7u;

    //multiply the number by 2 ( shift left by 1 !)(shifting to the left does the same thing as multiplying by 2 !!!) 
    registers[Vx] = registers[Vx] << 1;

}

/// @brief skip next instruction if Vx != Vy
void Chip8::OP_9xy0(){
    uint16_t Vx = (opcode && 0x0F00u) >> 8u;
    uint16_t Vy = (opcode && 0x00F0u) >> 4u;
    if ( registers[Vx] != registers[Vy]){
        pc+=2;
    }
}

/// @brief set I = nnn
/// @note the index register is used to store memory addresses . (cause it's 16 bits) , 
void Chip8::OP_Annn(){

    uint16_t address = opcode & 0x0FFFu;

    //the index and PC are both adress storers .
    //BUT  index stores the adress for data needed currently 
    //while PC stores the the address for the next instruction to execute
    index = address;
}


/// @brief jump to location nnn + V0
void Chip8::OP_Bnnn(){
    uint16_t address = opcode & 0x0FFFu;

    pc = registers[0] + address;

}

/// @brief set Vx = random byte AND kk
void Chip8::OP_Cxkk(){
    uint16_t Vx = (opcode && 0x0F00u) >> 8u;
    uint16_t byte = (opcode & 0x00FFu);

    registers[Vx] = randByte(randGen) & byte;
}


/// @brief display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
/// @note LIterally just draw an image on the screen .
void Chip8::OP_Dxyn(){
    uint16_t Vx = (opcode && 0x0F00u) >> 8u;
    uint16_t Vy = (opcode && 0x00F0u) >> 4u;
    uint16_t height = opcode & 0x000Fu;

    registers[0xF] = 0;

    //x and y coords are the top left of the sprite
    uint16_t xPos = registers[Vx] % VIDEO_WIDTH;
    uint16_t yPos = registers[Vy] % VIDEO_HEIGHT;

    ////height can ba anywhere from 1 to 15 .
    for (unsigned int row = 0; row < height ; row++){
        //A spriteByte is literally one row of your sprite.
        uint8_t spriteByte = memory[index + row];

        for (unsigned int col = 0; col < 8 ; col++){
            //we use 1000 0000 shifting by col(1-8) to get the specific bit in the row!)
            uint8_t spritePixel = spriteByte & (0x80u >> col);
            uint32_t* screenPixel = &video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];


            if (spritePixel){
                
                //if the screen pixel is already ... we makr in the flag register .
                if (*screenPixel == 0xFFFFFFFF){
                    registers[0xF] = 1; // collision
                }

                //on pixels-->off && off pixels --> on (XOR operation)
                *screenPixel ^= 0xFFFFFFFF; // XOR the pixel with white color
            }
        }
    }

}

/// @brief skip next instruction if key with the value of Vx is pressed
void Chip8::OP_Ex9E(){
    uint8_t Vx = (opcode & 0x0F00u) >>8u;

    //notice that the key is first stored in the Vx register !
    uint8_t key = registers[Vx] ;
    
    if (keypad[key]){
        pc+=2;
    }

}


/// @brief skip next instruction if key with the value of Vx is not pressed
void Chip8::OP_ExA1(){
    uint8_t Vx = (opcode & 0x0F00u) >>8u;
    uint8_t key = registers[Vx] ;

    uint8_t key = registers[Vx] ;

    if (!keypad[key]){
        pc+=2;
    }

}

/// @brief set Vx = delay timer value
/// @note get the current Delaytime !
void Chip8::OP_Fx07(){
    /*
    here is the thing imagine we want to get the delayTImer value ... we first need to store that delay timer in some register
    , thereafter we can use that value in the register . whAT we give as F is the register we need to sotre the delay timer !
    */

    /// set the Vx as the F data
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    // store the data current delay TImer in a register Vx
    registers[Vx] = delayTimer;

}

/// @brief wait for a key press, store the value of the key in Vx
void Chip8::OP_Fx0A(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    if (keypad[0])
	{
		registers[Vx] = 0;
	}
	else if (keypad[1])
	{
		registers[Vx] = 1;
	}
	else if (keypad[2])
	{
		registers[Vx] = 2;
	}
	else if (keypad[3])
	{
		registers[Vx] = 3;
	}
	else if (keypad[4])
	{
		registers[Vx] = 4;
	}
	else if (keypad[5])
	{
		registers[Vx] = 5;
	}
	else if (keypad[6])
	{
		registers[Vx] = 6;
	}
	else if (keypad[7])
	{
		registers[Vx] = 7;
	}
	else if (keypad[8])
	{
		registers[Vx] = 8;
	}
	else if (keypad[9])
	{
		registers[Vx] = 9;
	}
	else if (keypad[10])
	{
		registers[Vx] = 10;
	}
	else if (keypad[11])
	{
		registers[Vx] = 11;
	}
	else if (keypad[12])
	{
		registers[Vx] = 12;
	}
	else if (keypad[13])
	{
		registers[Vx] = 13;
	}
	else if (keypad[14])
	{
		registers[Vx] = 14;
	}
	else if (keypad[15])
	{
		registers[Vx] = 15;
	}else{
        /*
        The clever trick : why do decrement pc by 2? 
        because in the fetch state pc+=2 happens ... and if no key is pressed .
        in the execute the pc is again decremented by 2 . 
        meaning pc is not chagned in the cycle!
        like frozen in time until the key is pressed !
        */
        pc-=2;
    }

}

    
/// @brief set delay timer = Vx
void Chip8::OP_Fx15(){

    uint8_t Vx = (opcode & 0x0F00u) >>8u;

    delayTimer = registers[Vx];
}

/// @brief set sound timer = Vx
void Chip8::OP_Fx18(){

    uint8_t Vx = (opcode & 0x0F00u) >>8u;

    soundTimer = registers[Vx];
}

/// @brief set I = I+ Vx
void Chip8::OP_Fx1E(){

    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    index += registers[Vx];
}

/// @brief set I = location of sprite for digit Vx
/// @note We know the font characters are located at 0x50, and we know they’re five bytes each, so we can get the address of the first byte of any character by taking an offset from the start address.
void Chip8::OP_Fx29(){

    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    //each sprite is 5 bytes long and they start at the address 0x50
    index = FONTSET_START_ADDRESS + (registers[Vx] * 5);
}

/// @brief store BCD representation of Vx in memory locations I, I+1, and I+2
void Chip8::OP_Fx33(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    //the value is stored in Vx address 0 < value < 255
    uint8_t value = registers[Vx];

    //hundreds digit
    memory[index] = value / 100;

    //tens digit
    memory[index + 1] = (value / 10) % 10;

    //ones digit
    memory[index + 2] = value % 10;
}

/// @brief store registers V0 through Vx in memory starting at location I
void Chip8::OP_Fx55(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    for (uint8_t i = 0; i <= Vx ; i++){
        //store the registor values into the memory
        memory[index + i] = registers[i];
    }
}

/// @brief read registers V0 through Vx from memory starting at location I
void Chip8::OP_Fx65(){
    uint8_t Vx = (opcode & 0x0F00u) >>8u;

    for (uint8_t i = 0; i <= Vx ; i++){
        // read the memory values to the registers
        registers[i] = memory[index + i];
    }

}

//END OF ALL 34 INSTRUCTIONS

