/**
 * @file snake.c
 * @brief A snake game that can be played in the terminal.
 * @author Justin Thoreson
 */

#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

static const uint8_t GRID_DIMENSION_MIN = 5;
static const uint8_t GRID_DIMENSION_MAX = 50;
static const char* SNAKE = "\x1b[0;41m ";
static const char* FOOD = "\x1b[0;46m ";
static const uint8_t SLEEP_TIME_MILLIS = 100;

// TODO: use result enum for game state tracking
#if 0 // currently unused
typedef enum {
	SNAKE_OK,
	SNAKE_FAIL,
	SNAKE_WIN,
	SNAKE_LOSE
} snake_result_t;
#endif

typedef enum {
	DIRECTION_UP    = 'A',
	DIRECTION_DOWN  = 'B',
	DIRECTION_RIGHT = 'C',
	DIRECTION_LEFT  = 'D'
} direction_t;

typedef struct {
	uint8_t x, y;
} coordinate_t;

typedef struct {
	uint8_t width;
	uint8_t height;
} grid_t;

typedef struct {
	coordinate_t* body;
	coordinate_t last_tail;
	direction_t direction;
	uint16_t length;
} snake_t;

static struct termios get_terminal() {
	struct termios terminal;
	tcgetattr(STDIN_FILENO, &terminal);
	return terminal;
}

static void set_terminal(const struct termios* const terminal) {
	tcsetattr(STDIN_FILENO, TCSANOW, terminal);
	fflush(stdout);
}

static struct termios terminal_noncanon(struct termios terminal) {
	terminal.c_lflag &= ~(ICANON | ECHO);
	return terminal;
}

static void stdin_set_flag(int flag) {
	int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, flags | flag);
}

static struct termios init_terminal() {
	struct termios old_terminal = get_terminal();
	struct termios new_terminal = terminal_noncanon(old_terminal);
	set_terminal(&new_terminal);
	stdin_set_flag(O_NONBLOCK);
	return old_terminal;
}

static void reset_terminal(const struct termios* const terminal) {
	set_terminal(terminal);
	stdin_set_flag(~O_NONBLOCK);
}

static void clear_screen() {
	printf("\x1b[2J\x1b[H");
}

#if 0 // currently unused 
static uint16_t coord_to_index(coordinate_t coord) {
	return coord.x + coord.y * WIDTH;
}
#endif

// check for invalid index
static coordinate_t index_to_coord(uint16_t idx, const grid_t* const grid) {
	coordinate_t coord;
	coord.x = idx % grid->width;
	coord.y = idx / grid->width;
	return coord;
}

static coordinate_t compute_food(const snake_t* const snake, const grid_t* const grid) {
	srand(time(NULL)); // TODO: only call this once at the initialization stage of the program
	coordinate_t food;
	uint16_t size = grid->width * grid->height;
	bool is_cell_available;
	do {
		is_cell_available = true;
		food = index_to_coord(rand() % size, grid);
		for (uint16_t i = 0; i < snake->length; i++)
			if (snake->body[i].x == food.x && snake->body[i].y == food.y)
				is_cell_available = false;
	} while (!is_cell_available);
	return food;
}

static void wait() {
	// TODO: error check
	struct timespec req, rem;
	req.tv_sec = 0;
	req.tv_nsec = SLEEP_TIME_MILLIS * 1000000;
	nanosleep(&req, &rem);	
}

static snake_t init_snake(const grid_t* const grid) {
	uint16_t size = grid->width * grid->height;
	snake_t snake;
	snake.length = 1;
	snake.body = (coordinate_t*)malloc(size * sizeof(coordinate_t));
	snake.body[snake.length - 1] = (coordinate_t){ grid->width / 2, grid->height / 2 };
	snake.last_tail = snake.body[snake.length - 1];
	snake.direction = DIRECTION_RIGHT;
	return snake;	
}

static void draw_border(const grid_t* const grid) {
	for (uint8_t i = 0; i < grid->height; i++)
		printf("\x1b[%d;%dH\x1b[0;47m ", i + 1, grid->width + 1);
	for (uint8_t i = 0; i <= grid->width; i++) // account for bottom-right corner
		printf("\x1b[%d;%dH\x1b[0;47m ", grid->height + 1, i + 1);
	printf("\x1b[0m");
}

static void draw_food(const coordinate_t* const food) {
	printf("\x1b[%d;%dH%s\x1b[0m", food->y + 1, food->x + 1, FOOD);
}

static void draw_snake(const snake_t* const snake) {
	const coordinate_t* segment = &snake->last_tail;
	printf("\x1b[%d;%dH \x1b[0m", segment->y + 1, segment->x + 1);
	for (uint16_t i = 0; i < snake->length; i++) {
		segment = &snake->body[i];
		printf("\x1b[%d;%dH%s\x1b[0m", segment->y + 1, segment->x + 1, SNAKE);
	}
}

static void process_input(snake_t* const snake) {
	char c;
	read(STDIN_FILENO, &c, 1);	
	if (c == '\x1b') { // ANSI escape code
		getchar(); // ignore [
		char value = getchar();
		direction_t* const direction = &snake->direction;
		if (value == DIRECTION_UP && *direction != DIRECTION_DOWN)
			*direction = DIRECTION_UP;
		else if (value == DIRECTION_DOWN && *direction != DIRECTION_UP)
			*direction = DIRECTION_DOWN;
		else if (value == DIRECTION_RIGHT && *direction != DIRECTION_LEFT)
			*direction = DIRECTION_RIGHT;
		else if (value == DIRECTION_LEFT && *direction != DIRECTION_RIGHT)
			*direction = DIRECTION_LEFT;
	}
}

static void update(snake_t* const snake, coordinate_t* const food, const grid_t* const grid) {
	// shift snake segments
	snake->last_tail = snake->body[0];
	for (uint16_t i = 0; i < snake->length - 1; i++)
		snake->body[i] = snake->body[i + 1];
	// move snake forward
	coordinate_t* head = &snake->body[snake->length - 1];
	direction_t* direction = &snake->direction;
	if (*direction == DIRECTION_UP && head->y > 0)
		head->y--;
	else if (*direction == DIRECTION_DOWN && head->y < grid->height - 1)
		head->y++;
	else if (*direction == DIRECTION_RIGHT && head->x < grid->width - 1)
		head->x++;
	else if (*direction == DIRECTION_LEFT && head->x > 0)
		head->x--;
	// grow snake and create food if snake eats existing one
	if (head->x == food->x && head->y == food->y) {
		*food = compute_food(snake, grid);
		snake->length++;
		snake->body[snake->length - 1] = *head;
	}
	// flush output buffer so the result is displayed immediately
	fflush(stdout);
}

static bool parse_uint8(const char* const arg, uint8_t* const value) {
	if (!arg || !value)
		return false;
	int64_t temp;
	if (!sscanf(arg, "%ld", &temp))
		return false;
	if (temp < 0 || temp > UINT8_MAX)
		return false;
	*value = (uint8_t)temp;
	return true;
}

int main(int argc, char** argv) {
	if (argc != 3)
		return 1;
	grid_t grid;
	if (!parse_uint8(argv[1], &grid.width) || !parse_uint8(argv[2], &grid.height))
		return 1;
	if (grid.width < GRID_DIMENSION_MIN || grid.height < GRID_DIMENSION_MIN)
		return 1;
	if (grid.width > GRID_DIMENSION_MAX || grid.height > GRID_DIMENSION_MAX)
		return 1;
	snake_t snake = init_snake(&grid);
	coordinate_t food = compute_food(&snake, &grid);
	struct termios old_terminal = init_terminal();
	clear_screen();
	draw_border(&grid);
	do {
		draw_snake(&snake);
		draw_food(&food);
		wait();
		process_input(&snake);
		update(&snake, &food, &grid);
	} while (true); // TODO: terminate loop on game over state
	free(snake.body);
	snake.body = NULL;
	reset_terminal(&old_terminal);
	return 0;
}
