#include <cstdint>

class Chip8
{
    public :
        uint8_t registers[16];
        uint8_t memory[4096];
        uint16_t index{};
        uint16_t pc{};
        uint16_t stack[16];
        uint8_t sp{};
        uint8_t SoundTimer{};
        uint8_t delayTimer{};
        uint32_t video[64*32]{};
        uint8_t keypad[16]{};
        uint16_t opcode;


};


class Chip8 {
public:
    // ... your registers and memory go here ...

    // ADD THIS LINE:
    void LoadROM(char const* filename); 

    // ... timers, video, etc. ...
}; // <--- The class ends here