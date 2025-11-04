C = gcc
C_FLAGS = -std=gnu11 -Wall -Werror -pedantic -ggdb -O0
PROGRAM = snake

$(PROGRAM): $(PROGRAM).c
	$(C) $(C_FLAGS) $< -o $@

.PHONY: clean

clean:
	rm $(PROGRAM)
