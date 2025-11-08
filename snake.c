/**
 * @file snake.c
 * @brief A snake game that can be played in the terminal.
 * @author Justin Thoreson
 */

#include "snake.h"
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
	coordinate_t* body;
	coordinate_t last_tail;
	uint16_t length;
	direction_t direction;
} snake_t;

typedef struct {
	snake_t snake;
	coordinate_t food;
	uint8_t width, height;
} grid_t;

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

static snake_result_t index_to_coord(coordinate_t* const coord, const uint16_t* const idx, const uint8_t* const width, const uint8_t* const height) {
	uint16_t size = *width * *height;
	if (*idx >= size)
		return SNAKE_FAIL;
	coord->x = *idx % *width;
	coord->y = *idx / *width;
	return SNAKE_OK;
}

static void wait() {
	struct timespec remaining, requested = { 0, SLEEP_TIME_MILLIS * 1000000 };
	nanosleep(&requested, &remaining);
}

static bool is_snake_tile(const coordinate_t* const coord, const snake_t* const snake) {
	for (uint16_t i = 0; i < snake->length; i++)
		if (snake->body[i].x == coord->x && snake->body[i].y == coord->y)
			return true;
	return false;
}

static snake_result_t compute_food(grid_t* const grid) {
	srand(time(NULL)); // TODO: only call this once at the initialization stage of the program
	uint16_t size = grid->width * grid->height;
	snake_t* snake = &grid->snake;
	coordinate_t* food = &grid->food;
	snake_result_t result = SNAKE_OK;
	do {
		uint16_t idx = rand() % size;
		result = index_to_coord(food, &idx, &grid->width, &grid->height);
	} while (result == SNAKE_OK && is_snake_tile(food, snake));
	return result;
}

static snake_result_t init_snake(grid_t* const grid) {
	uint16_t size = grid->width * grid->height;
	snake_t* snake = &grid->snake;
	snake->length = 1;
	snake->body = (coordinate_t*)malloc(size * sizeof(coordinate_t));
	if (!snake->body)
		return SNAKE_FAIL;
	snake->body[snake->length - 1] = (coordinate_t){ grid->width / 2, grid->height / 2 };
	snake->last_tail = snake->body[snake->length - 1];
	snake->direction = DIRECTION_RIGHT;
	return SNAKE_OK;
}

static snake_result_t init_grid(grid_t* const grid, const uint8_t* const width, const uint8_t* const height) {
	if (*width < GRID_DIMENSION_MIN || *height < GRID_DIMENSION_MIN)
		return SNAKE_FAIL;
	if (*width > GRID_DIMENSION_MAX || *height > GRID_DIMENSION_MAX)
		return SNAKE_FAIL;
	grid->width = *width;
	grid->height = *height;
	snake_result_t result = init_snake(grid);
	if (result != SNAKE_OK)
		return result;
	result = compute_food(grid);
	if (result != SNAKE_OK)
		return result;
	return SNAKE_OK;
}

static void draw_border(const uint8_t* const width, const uint8_t* const height) {
	// +1 to account for terminal coordinates starting at 1, not 0
	for (uint8_t i = 0; i < *height; i++)
		printf("\x1b[%d;%dH\x1b[0;47m ", i + 1, *width + 1);
	for (uint8_t i = 0; i <= *width; i++) // account for bottom-right corner
		printf("\x1b[%d;%dH\x1b[0;47m ", *height + 1, i + 1);
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

static void update_direction(snake_t* const snake) {
	char c;
	read(STDIN_FILENO, &c, 1); // no need to return error if nothing is read
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

static void shift_snake(snake_t* const snake) {
	snake->last_tail = snake->body[0];
	for (uint16_t i = 0; i < snake->length - 1; i++)
		snake->body[i] = snake->body[i + 1];
}

static snake_result_t move_snake(grid_t* const grid) {
	coordinate_t* head = &grid->snake.body[grid->snake.length - 1];
	coordinate_t new_head = *head;
	direction_t* direction = &grid->snake.direction;
	if (*direction == DIRECTION_UP) {
		if  (new_head.y <= 0)
			return SNAKE_LOSE;
		new_head.y--;
	} else if (*direction == DIRECTION_DOWN) {
		if (new_head.y >= grid->height - 1)
			return SNAKE_LOSE;
		new_head.y++;
	} else if (*direction == DIRECTION_RIGHT) {
		if (new_head.x >= grid->width - 1)
			return SNAKE_LOSE;
		new_head.x++;
	} else if (*direction == DIRECTION_LEFT) {
		if (new_head.x <= 0)
			return SNAKE_LOSE;
		new_head.x--;
	}
	if (is_snake_tile(&new_head, &grid->snake))
		return SNAKE_LOSE;
	*head = new_head;
	return SNAKE_OK;
}

static void grow_snake(snake_t* const snake) {
	coordinate_t* head = &snake->body[snake->length - 1];
	snake->length++;
	snake->body[snake->length - 1] = *head;
}

static snake_result_t update_grid(grid_t* const grid) {
	snake_t* snake = &grid->snake;
	coordinate_t* head = &snake->body[snake->length - 1];
	coordinate_t* food = &grid->food;
	snake_result_t result = SNAKE_OK;
	// grow snake and create food if snake eats existing one
	if (head->x == food->x && head->y == food->y) {
		result = compute_food(grid);
		if (compute_food(grid) != SNAKE_OK)
			return result;
		grow_snake(snake);
	} else {
		shift_snake(snake);
	}
	result = move_snake(grid);
	// flush output buffer so the result is displayed immediately
	if (result == SNAKE_OK)
		fflush(stdout);
	return result;
}

snake_result_t snake(const snake_args_t* const args) {
	grid_t grid;
	snake_result_t result = init_grid(&grid, &args->grid_width, &args->grid_height);
	if (result != SNAKE_OK)
		return result;
	struct termios old_terminal = init_terminal();
	clear_screen();
	draw_border(&grid.width, &grid.height);
	do {
		draw_food(&grid.food);
		draw_snake(&grid.snake);
		wait();
		update_direction(&grid.snake);
		result = update_grid(&grid);
	} while (result == SNAKE_OK);
	free(grid.snake.body);
	grid.snake.body = NULL;
	reset_terminal(&old_terminal);
	return result;
}
