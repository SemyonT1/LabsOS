.PHONY: all clean

all: mycat mygrep

mycat: mycat.o
	gcc mycat.o -o mycat

mycat.o: mycat.c
	gcc mycat.c -c -Wall -Wextra -g

mygrep: mygrep.o
	gcc mygrep.o -o mygrep

mygrep.o: mygrep.c
	gcc mygrep.c -c -Wall -Wextra -g

clean:
	rm -rf *.o mycat mygrep

