CC ?= cc
CFLAGS ?= -std=c17 -O2 -Wall -Wextra -Wpedantic -Werror

all: build/budget

build/budget: src/main.c
	mkdir -p build
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf build

.PHONY: all clean
