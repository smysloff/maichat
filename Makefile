
# file: Makefile
# author: ishimai

all: clean cli srv

cli:
	gcc -o cli src/cli.c src/utils.c

srv:
	gcc -o srv src/srv.c src/utils.c

clean:
	rm -r cli srv

.PHONY: all cli srv clean
