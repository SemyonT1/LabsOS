
.PHONY: all clean

all: mutex

mutex: mutex.c
	gcc mutex.c -o mutex -lpthread -Wall -Wextra

clean: 
	rm -rf *.o mutex
