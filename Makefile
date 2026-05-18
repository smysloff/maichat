
# file: Makefile

all:
	@cc -Wall -Werror -Wextra -g main.c lib/*.c -I./lib -o maichat
	@./maichat

.PHONY: all
