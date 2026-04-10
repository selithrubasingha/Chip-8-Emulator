

# CHIP-8 Emulator

This project is a fully functional CHIP-8 emulator written in C++ using SDL2. 

This emulator is the first step on a larger "Golden Path" roadmap. The ultimate goal is to dive deep into low-level systems programming: starting with this emulator, eventually building a more complex system like a Game Boy emulator, writing a custom compiler, and finally developing a custom operating system from scratch. It might take years, but the journey starts here!

## ✨ Features
* **Complete Instruction Set:** Accurately decodes and executes all 34 standard CHIP-8 opcodes.
* **Hardware Accelerated Graphics:** Uses SDL2 for fast, efficient rendering.
* **Customizable Execution:** Command-line arguments allow you to tweak the display scale and the CPU cycle delay (speed) to match different ROMs.
* **Wayland Native:** Compatible with modern Linux compositors (like Hyprland/Mutter).

## 🛠️ Dependencies

To compile and run this emulator, you need the **SDL2** development libraries installed on your system.

**For Fedora Linux:**
```bash
sudo dnf install SDL2-devel
```

## 🚀 Compilation

Ensure you have `g++` and `pkg-config` installed. Open your terminal in the project directory and run:

```bash
g++ main.cpp chip8.cpp platform.cpp -o chip8 $(pkg-config --cflags --libs sdl2)
```

## 🎮 Usage

Run the compiled executable via the terminal, passing in your desired scale, cycle delay, and the path to a `.ch8` ROM file.

**Command Structure:**
```bash
./chip8 <Scale> <Delay> <Path_to_ROM>
```

**Example (Running Tetris with a 10x scale and 2ms delay):**
```bash
./chip8 10 2 roms/tetris.ch8
```

> **Note for Wayland Users:** If you are running a Wayland compositor and the window looks blurry or has scaling issues, force SDL to use the native Wayland driver:
> ```bash
> SDL_VIDEODRIVER=wayland ./chip8 10 2 roms/tetris.ch8
> ```

## ⌨️ Controls

The original CHIP-8 used a 16-key hexadecimal keypad (0-F). This emulator maps those keys to the left side of a standard QWERTY keyboard:

| CHIP-8 Key | Keyboard Key |   | CHIP-8 Key | Keyboard Key |
| :---: | :---: | :---: | :---: | :---: |
| 1 | 1 | | C | 4 |
| 2 | 2 | | D | R |
| 3 | 3 | | E | F |
| 4 | Q | | F | V |
| 5 | W | | 7 | A |
| 6 | E | | 8 | S |
| A | Z | | 9 | D |
| 0 | X | | B | C |

*Tip: If you are playing Fran Dachille's Tetris (1991), the default controls map to **W** (Left), **E** (Right), **Q** (Rotate), and **A** (Drop).*
