all: first

first:
	gcc -g -Wall -Werror -fsanitize=address cache_simulator.c -o first

clean:
	rm -rf first