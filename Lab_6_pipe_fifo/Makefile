.PHONY: all clean

all: pipe fifo_reader fifo_writer

pipe: pipe.c
	gcc pipe.c -o pipe -Wall -Wextra

fifo_reader: fifo_reader.c
	gcc fifo_reader.c -o fifo_reader -Wall -Wextra
	
fifo_writer: fifo_writer.c
	gcc fifo_writer.c -o fifo_writer -Wall -Wextra

clean:
	rm -rf *.o pipe fifo_reader fifo_writer