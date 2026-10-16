# Not part of the ROM build (make_tools.mk only builds the tools it lists).
# Needs libmgba-dev and libpng-dev.

CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra -Wno-unused-parameter

replay-runner: runner.c
	$(CC) $(CFLAGS) -o $@ $< -lmgba -lpng

clean:
	rm -f replay-runner

.PHONY: clean
