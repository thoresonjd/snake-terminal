/**
 * @file snake.h
 * @brief A snake game that can be played in the terminal.
 * @author Justin Thoreson
 */

#include <stdint.h>

typedef struct {
	uint8_t grid_width;
	uint8_t grid_height;
} snake_args_t;

typedef enum {
	SNAKE_OK,
	SNAKE_FAIL,
	SNAKE_WIN,
	SNAKE_LOSE
} snake_result_t;

snake_result_t snake(const snake_args_t* const args);

