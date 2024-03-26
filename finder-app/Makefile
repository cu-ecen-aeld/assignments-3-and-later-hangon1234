
CFLAGS = -g -Wall -Og -std=c99 
LDFLAGS = 
CC = $(CROSS_COMPILE)gcc

all: writer
 
writer: writer.o
	$(CC) $< -o $@

writer.o: writer.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.elf *.bin 
