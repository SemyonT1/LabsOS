.PHONY: all clean
all: fork
fork: fork.o
	gcc fork.o -o fork
fork.o: fork.c
	gcc -c fork.c -Wall -Wextra -g
clean:
	rm -rf *.o fork
