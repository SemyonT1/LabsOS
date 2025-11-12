.PHONY: all clean
all: archiver
archiver: archiver.o
	gcc archiver.o -o archiver
archiver.o: archiver.c
	gcc -c archiver.c -Wall -Wextra
clean:
	rm -rf *.o archiver
