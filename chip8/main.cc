#include "SDL2/SDL.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>

#include "chip8.cc"

using namespace std;

#define ESC_KEY 27
#define SCALE 15

map<int, int> key_map = {
	{SDLK_1, 0x1}, {SDLK_2, 0x2}, {SDLK_3, 0x3}, {SDLK_4, 0xC},
	{SDLK_q, 0x4}, {SDLK_w, 0x5}, {SDLK_e, 0x6}, {SDLK_r, 0xD},
	{SDLK_a, 0x7}, {SDLK_s, 0x8}, {SDLK_d, 0x9}, {SDLK_f, 0xE},
	{SDLK_z, 0xA}, {SDLK_x, 0x0}, {SDLK_c, 0xB}, {SDLK_v, 0xF},
};

int main(int argc, char *argv[])
{

	if (argc != 3) {
		cerr << "Error: 3 args must be provided:- "
			 << "./CHIP8 <ROM_PATH> <FPS>" << endl;
		return 1;
	}

	Chip8 chip8;
	SDL_Window *window = NULL;
	SDL_Renderer *renderer;
	SDL_Texture *texture;

	window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_UNDEFINED,
							  SDL_WINDOWPOS_UNDEFINED,
							  WIDTH * SCALE,  // width
							  HEIGHT * SCALE, // height
							  SDL_WINDOW_RESIZABLE);
	if (window == NULL) {
		cerr << SDL_GetError() << endl;
		return 0;
	}

	renderer = SDL_CreateRenderer(window, -1, 0);
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
								SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

	chip8.load(argv[1]);
	bool running = true;
	const int FPS = stoi(argv[2]);
	Uint32 starting_tick;
	SDL_Event event;

	while (running) {
		starting_tick = SDL_GetTicks();

		while (SDL_PollEvent(&event)) {
			Uint32 e = event.type;
			int key = event.key.keysym.sym; // returns code for key pressed
			switch (e) {
				case SDL_KEYDOWN: {
					if (key == SDLK_ESCAPE) {
						running = false;
						break;
					}
					else if (key_map.count(key)) {
						chip8.key = 1;
						chip8.key_value = key_map[key];
					}
					else {
						chip8.key = 0;
					}
					break;
				}

				case SDL_QUIT:
					running = false;
					break;

				default:
					chip8.key = 0;
					break;
			}
		}

		chip8.cycle();

		// draw to screen
		if (chip8.draw) {
			chip8.draw = 0;
			SDL_UpdateTexture(texture, NULL, chip8.display,
							  WIDTH * sizeof(uint32_t));
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texture, NULL, NULL);
			SDL_RenderPresent(renderer);
		}

		// delay
		if ((1000 / FPS) > SDL_GetTicks() - starting_tick)
			SDL_Delay(1000 / FPS - (SDL_GetTicks() - starting_tick));
	}

	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}