# A Chip-8 Emulator in C++

## Requirements
- SDL 2

## ROMs Tested
- IBM Logo
- PONG 
- TETRIS

## To Run
- `cd chip8`
- ```g++ main.cc `pkg-config --cflags --libs sdl2` -o CHIP8```
- `./CHIP8 <ROM_PATH>`

## References
- https://tobiasvl.github.io/blog/write-a-chip-8-emulator/
- https://austinmorlan.com/posts/chip8_emulator/
- https://github.com/JamesGriffin/CHIP-8-Emulator/
- The Emulation Development discord helped me fix some nasty bugs :)