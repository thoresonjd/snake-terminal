/**
 * @file snake.h
 * @brief An implementation of snake playable within the terminal.
 * @author Justin Thoreson
 */

#include <stdint.h>

/**
 * @brief Arguments to be given to the snake program.
 */
typedef struct {
	uint8_t grid_width;
	uint8_t grid_height;
} snake_args_t;

/**
 * @brief Result codes returned by the snake program.
 */
typedef enum {
	SNAKE_OK,
	SNAKE_FAIL,
	SNAKE_WIN,
	SNAKE_LOSE
} snake_result_t;

/**
 * @brief Execute snake.
 * @param[in] args The snake arguments
 * @return Enum denoting the result of the snake program
 */
snake_result_t snake(const snake_args_t* const args);

