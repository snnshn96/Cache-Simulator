all: first

first: first.c first.h
	gcc -g -Wall -Werror -fsanitize=address first.c -o first

clean:
	rm -rf first
