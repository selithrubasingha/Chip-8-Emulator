#include "chip8.hpp"
#include <iostream>
#include "platform.hpp"


int main(int argc, char* argv[])
{
    if (argc != 4){
        std::cerr << "Usage: " << argv[0] << " <Scale> <Delay> <ROM>" << std::endl;
        std::exit(EXIT_FAILURE);
    
    }

    //these arguments should be given in the program 
    int videoScale = std::stoi(argv[1]);
    int cycleDelay = std::stoi(argv[2]);
    char const* romFilename = argv[3];

    //platform is the connector of teh cpp code and the display (GPU)
    Platform platform("CHIP-8 Emulator",VIDEO_WIDTH * videoScale ,VIDEO_HEIGHT* videoScale , VIDEO_WIDTH,VIDEO_HEIGHT);

    Chip8 chip8;
    chip8.LoadROM(romFilename);

    int videoPitch = sizeof(chip8.video[0]) * VIDEO_WIDTH;

    auto lastCycleTime = std::chrono::high_resolution_clock::now();

    bool quit = false;

    while (!quit){
        //quit will become true if escape key or X(in window) is pressed
        quit = platform.ProcessInput(chip8.keypad);


        //if timedelay is 30ms , when dt hits 30ms , the cycle will execute . then the freq is 1/30ms !!
        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastCycleTime).count();

        if (dt > cycleDelay){
            chip8.Cycle();
            lastCycleTime = currentTime;

            //update the screen
            platform.Update(chip8.video, videoPitch);
        }

        
    }   

    return 0;
}

