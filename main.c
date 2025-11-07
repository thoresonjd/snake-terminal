/**
 * @file main.c
 * @brief A snake game that can be played in the terminal.
 * @author Justin Thoreson
 */

#include "snake.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

static bool parse_uint8(const char* const arg, uint8_t* const value);

static bool parse_args(snake_args_t* const args, const int* const argc, char** const argv);

int main(int argc, char** argv) {
	snake_args_t args = { 0 };
	if (!parse_args(&args, &argc, argv))
		return 1;
	snake_result_t result = snake(&args);
	(void)result;
	return 0;
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

static bool parse_args(snake_args_t* const args, const int* const argc, char** const argv) {
	if (*argc != 3)
		return false;
	if (!parse_uint8(argv[1], &args->grid_width) || !parse_uint8(argv[2], &args->grid_height))
		return false;
	return true;
}

