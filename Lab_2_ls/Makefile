.PHONY: all clean

all: myls

myls: myls.o
	gcc myls.o -o myls -Wall -Wextra -g

myls.o: myls.c
	gcc myls.c -c -Wall -Wextra -g

clean:  
	rm -rf *.o myls
