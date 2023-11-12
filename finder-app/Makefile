
CFLAGS = -g -Wall -Og -std=c99 
LDFLAGS = 
CC = gcc

all: writer
 
writer: writer.o
	$(CROSS_COMPILE)$(CC) $< -o $@

writer.o: writer.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.elf *.bin 
