#include "chip8.hh"
using namespace std;

// helpers

// ref :- https://www.geeksforgeeks.org/program-decimal-hexadecimal-conversion/
string opcode2hex(int opcode)
{
	char hexa[100];
	int i = 0;
	int instruction = opcode;
	while (opcode != 0) {
		int temp = 0;
		temp = opcode % 16;
		if (temp < 10) {
			hexa[i] = temp + 48;
			i++;
		}
		else {
			hexa[i] = temp + 55;
			i++;
		}
		opcode = opcode / 16;
	}
	string s = "";
	for (int j = i - 1; j >= 0; j--)
		s += hexa[j];
	return s;
}

void unknown_opcode(int opcode)
{
	string hex_opcode = opcode2hex(opcode);
	cerr << "Invalid Opcode :- " << opcode << " [0x" << hex_opcode << "]" << endl;
	exit(0);
}

// font set for chip8
int font_set[FONT_SET_SIZE] = {
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

// inits chip8
Chip8::Chip8()
{
	program_counter = START_ADDRESS;
	index_register = 0x0000;

	for (int i = 0; i < MEMORY_SIZE; i++)
		memory[i] = 0x0000;

	draw = 1;
	for (int i = 0; i < HEIGHT * WIDTH; i++)
		display[i] = PIXEL_OFF;

	// load font set
	for (int i = 0; i < FONT_SET_SIZE; i++)
		memory[i] = font_set[i];

	delay_timer = 0;
	sound_timer = 0;
}

// load ROM into chip8 memory
void Chip8::load(char *fpath)
{
	cout << "Loading ROM in path :- \"" << fpath << "\"" << endl;
	ifstream file(fpath, ios_base::in);
	if (file.is_open()) {
		file.seekg(0, ios::end);
		int fsize = file.tellg();
		file.seekg(0, ios::beg);
		char *data = (char *)malloc(sizeof(char) * fsize);
		file.read(data, fsize);
		for (int i = 0; i < fsize; i++)
			memory[i + START_ADDRESS] = (uint8_t)data[i];
		free(data);
	}
	else {
		cerr << "ROM in path \"" << fpath << "\" does not exist" << endl;
		exit(0);
	}
	file.close();
	cout << "ROM successfuly loaded" << endl;
}

// runs every cycle
void Chip8::cycle()
{
	uint16_t instruction = fetch();
	execute(instruction);
	delay_timer = max(0, delay_timer - 1);
	sound_timer = max(0, sound_timer - 1);
}

// fetches instruction from PC
uint16_t Chip8::fetch()
{
	uint16_t instruction =
		(memory[program_counter] << 8) |
		memory[program_counter + 1]; // converting to 16 bit instruction.
	program_counter += 2;			 // incrementing PC
	return instruction;
}

// executes instruction
void Chip8::execute(uint16_t instruction)
{
	uint8_t N1 = (instruction & 0xF000) >> 12; // 1st nibble
	uint8_t X = (instruction & 0x0F00) >> 8;   // 2nd nibble
	uint8_t Y = (instruction & 0x00F0) >> 4;   // 3rd nibble
	uint8_t N = (instruction & 0x000F);		   // 4th nibble
	uint8_t NN = (instruction & 0x00FF);	   // 2nd byte
	uint16_t NNN = (instruction & 0x0FFF);	   // 2nd, 3rd and 4th nibble

	switch (N1) {
		case 0x0:
			switch (NN) {
				// clear screen
				case 0xE0: {
					draw = 1;
					for (int i = 0; i < HEIGHT * WIDTH; i++)
						display[i] = PIXEL_OFF;
				} break;
				case 0xEE:
					program_counter = stack_pop();
					break;
				default:
					unknown_opcode(instruction);
			}
			break;

		case 0x1:
			program_counter = NNN;
			break;

		case 0x6:
			variable_register[X] = NN;
			break;

		case 0x7:
			variable_register[X] += NN;
			break;

		case 0xA:
			index_register = NNN;
			break;

		// draw on screen
		case 0xD: {
			// set draw flag to true
			draw = 1;
			variable_register[0xF] = 0;
			// for N rows
			for (int dy = 0; dy < N; dy++) {
				// get 1 byte of data sprite data in memory address
				uint8_t sprite_data = memory[index_register + dy];
				// for each of the 8 pixels / bits in this sprite row
				for (int dx = 0; dx < 8; dx++) {
					// if the current pixel in the sprite row is on
					if (sprite_data & (0x80 >> dx)) {
						// set draw flag to true
						uint8_t x = (variable_register[X] + dx) % WIDTH,
								y = (variable_register[Y] + dy) % HEIGHT;
						// if the pixel at coordiates x, y on the screen is on
						if (display[y * WIDTH + x] == PIXEL_ON) {
							// turn off the pixel and set VF to 1
							display[y * WIDTH + x] = PIXEL_OFF;
							variable_register[0xF] = 1;
						}
						// or if the pixel is not on
						else if (display[y * WIDTH + x] != PIXEL_ON)
							display[y * WIDTH + x] = PIXEL_ON;
					}
				}
			}
			break;
		}

		case 0x2: {
			stack_push(program_counter);
			program_counter = NNN;
			break;
		}

		case 0x3:
			if (variable_register[X] == NN)
				program_counter += 2;
			break;

		case 0x4:
			if (variable_register[X] != NN)
				program_counter += 2;
			break;

		case 0x5:
			if (variable_register[X] == variable_register[Y])
				program_counter += 2;
			break;

		case 0x9:
			if (variable_register[X] != variable_register[Y])
				program_counter += 2;
			break;

		case 0x8: {
			switch (N) {
				case 0x0:
					variable_register[X] = variable_register[Y];
					break;

				case 0x1:
					variable_register[X] |= variable_register[Y];
					break;

				case 0x2:
					variable_register[X] &= variable_register[Y];
					break;

				case 0x3:
					variable_register[X] ^= variable_register[Y];
					break;

				case 0x4: {
					int sum = variable_register[X] + variable_register[Y];
					if ((variable_register[X] < 0 && variable_register[Y] < 0 &&
						 sum > 0) ||
						(variable_register[X] > 0 && variable_register[Y] > 0 &&
						 sum < 0))
						variable_register[0xF] = 1;
					else
						variable_register[0xF] = 0;
					variable_register[X] = sum;
					break;
				}

				case 0x5: {
					variable_register[0xF] =
						(variable_register[X] > variable_register[Y]);
					variable_register[X] -= variable_register[Y];
					break;
				}

				case 0x7: {
					variable_register[0xF] =
						(variable_register[Y] > variable_register[X]);
					variable_register[X] =
						variable_register[Y] - variable_register[X];
					break;
				}

				case 0x6: {
					variable_register[X] = variable_register[Y];
					variable_register[0xF] = (variable_register[X] & 0x1);
					variable_register[X] = variable_register[X] >> 1;
					break;
				}

				case 0xE: {
					variable_register[X] = variable_register[Y];
					variable_register[0xF] =
						((variable_register[X] & 0x80) >> 7);
					variable_register[X] = variable_register[X] << 1;
					break;
				}

				default:
					unknown_opcode(instruction);
					break;
			}
			break;
		}

		case 0xB:
			program_counter = NNN + variable_register[0];
			break;

		case 0xC:
			variable_register[X] = (rand() % NN) & NN;
			break;

		case 0xE: {
			switch (NN) {
				case 0x9E:
					if (variable_register[X] == key_value)
						program_counter += 2;
					break;

				case 0xA1:
					if (variable_register[X] != key_value)
						program_counter += 2;
					break;

				default:
					unknown_opcode(instruction);
			}
			break;
		}

		case 0xF: {
			switch (NN) {
				case 0x07:
					variable_register[X] = delay_timer;
					break;

				case 0x15:
					delay_timer = variable_register[X];
					break;

				case 0x18:
					sound_timer = variable_register[X];
					break;

				case 0x1E:
					index_register += variable_register[X];
					break;

				case 0x0A: {
					if (!key)
						program_counter -= 2;
					else
						variable_register[X] = key_value;
					break;
				}

				case 0x29:
					index_register = 5 * variable_register[X];
					break;

				case 0x33: {
					uint8_t VX = variable_register[X];
					memory[index_register + 2] = VX % 10;
					VX /= 10;
					memory[index_register + 1] = VX % 10;
					VX /= 10;
					memory[index_register] = VX % 10;
					break;
				}

				case 0x55:
					for (int i = 0; i <= X; i++)
						memory[index_register + i] = variable_register[i];
					break;

				case 0x65:
					for (int i = 0; i <= X; i++)
						variable_register[i] = memory[index_register + i];
					break;

				default:
					unknown_opcode(instruction);
			}
		} break;

		default:
			unknown_opcode(instruction);
	}
}

// pushes value to stack
void Chip8::stack_push(uint16_t val)
{
	if (stack_size == MAX_STACK_SIZE)
		return;
	stack[stack_size++] = val;
}

// pops value from stack
uint16_t Chip8::stack_pop()
{

	if (stack_size == 0) {
		cerr << "Error: Stack is empty" << endl;
		exit(0);
	}
	return stack[--stack_size];
}