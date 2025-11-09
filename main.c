/**
 * @file main.c
 * @brief An implementation of snake playable within the terminal.
 * @author Justin Thoreson
 */

#include "snake.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/**
 * @brief Parse an eight-bit unsigned integer.
 * @param[in] arg The string argument to parse
 * @param[out] value The parsed integer
 * @return true if the integer is parsed successfully, false otherwise
 */
static bool parse_uint8(const char* const arg, uint8_t* const value);

/**
 * @brief Parse command line arguments.
 * @param[out] args The parsed snake game arguments
 * @param[in] argc The number of command line arguments
 * @param[in] argv The command line arguments to parse
 * @return true if the arguments are parsed successfully, false otherwise
 */
static bool parse_args(
	snake_args_t* const args,
	const int* const argc,
	char** const argv
);

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

static bool parse_args(
	snake_args_t* const args,
	const int* const argc,
	char** const argv
) {
	if (*argc != 3)
		return false;
	if (!parse_uint8(argv[1], &args->grid_width) || !parse_uint8(argv[2], &args->grid_height))
		return false;
	return true;
}

