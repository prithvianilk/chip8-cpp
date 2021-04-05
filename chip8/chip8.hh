#define MEMORY_SIZE 4096
#define MAX_STACK_SIZE 16
#define REGISTER_SIZE 16
#define FONT_SET_SIZE 80
#define HEIGHT 32
#define WIDTH 64
#define START_ADDRESS 0x200

class Chip8 {
  private:
	// basic state
	uint8_t memory[MEMORY_SIZE];
	uint8_t variable_register[REGISTER_SIZE];
	uint16_t stack[MAX_STACK_SIZE];
	uint8_t stack_size = 0;
	uint16_t program_counter;
	uint16_t index_register;

	// stack operations
	void stack_push(uint16_t val);
	uint16_t stack_pop();

  public:
	Chip8();

	// main operations

	void load(char *fpath);
	void cycle();
	uint16_t fetch();
	void execute(uint16_t instruction);

	// timers
	uint8_t delay_timer, sound_timer;

	// draw flag
	bool draw;
	// display value [32px x 64px]
	uint8_t display[HEIGHT * WIDTH * 3]; 

	// key flag
	bool key;
	// pressed key value
	uint8_t key_value;
};
