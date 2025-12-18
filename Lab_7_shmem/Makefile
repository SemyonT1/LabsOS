.PHONY: all clean

all: reciever sender

reciever: reciever.c
	gcc reciever.c -o reciever -lpthread -Wall -Wextra

sender: sender.c
	gcc sender.c -o sender -lpthread -Wall -Wextra


clean: 
	rm -rf *.o reciever sender
