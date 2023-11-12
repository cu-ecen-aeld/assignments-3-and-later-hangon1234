
CFLAGS = -g -Wall -Og -std=c99 -ffreestanding
LDFLAGS = -nostdlib
CC = gcc

all: writer
 
writer: %.elf
	$(CC) $< -O binary $@

%.elf: %.o
	$(CC) $(LDFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.elf *.bin *.list
