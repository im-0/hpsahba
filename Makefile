.POSIX:
.SUFFIXES: .c .o

CC = gcc

CFLAGS = -std=gnu99 -Wall -Wextra -Wpedantic -O2 -g
LDFLAGS =


all: hpsahba

.c.o:
	$(CC) $(CFLAGS) -c -o $(@) $(<)

main.o: hpsa.h

hpsahba: main.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(@) $(<)

clean:
	rm -f *.o
	rm -f hpsahba

