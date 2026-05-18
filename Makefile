
# file: Makefile

all:
	@cc -Wall -Werror -Wextra -g main.c lib/*.c -o maichat
	@./maichat

.PHONY: all
