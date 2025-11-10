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
 * @brief Information on how to run the snake program.
 */
static const char* USAGE = "Usage: ./snake <grid_width> <grid_height>";

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

/**
 * @brief Print the result of the snake program.
 * @param[in] result The result of the snake program
 */
static void print_snake_result(const snake_result_t* const result);

int main(int argc, char** argv) {
	snake_args_t args = { 0 };
	if (!parse_args(&args, &argc, argv)) {
		printf("%s\n", USAGE);
		return 1;
	}
	snake_result_t result = snake(&args);
	print_snake_result(&result);
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

static void print_snake_result(const snake_result_t* const result) {
	switch (*result) {
		case SNAKE_OK:
			printf("SNAKE_OK (%d)\n", SNAKE_OK);
			break;
		case SNAKE_FAIL:
			printf("SNAKE_FAIL (%d)\n", SNAKE_FAIL);
			break;
		case SNAKE_WIN:
			printf("SNAKE_WIN (%d)\n", SNAKE_WIN);
			break;
		case SNAKE_LOSE:
			printf("SNAKE_LOSE (%d)\n", SNAKE_LOSE);
			break;
		case SNAKE_UNKNOWN:
		default:
			printf("SNAKE_UNKNOWN (%d)\n", SNAKE_UNKNOWN);
	}
}

