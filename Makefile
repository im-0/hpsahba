.POSIX:
.SUFFIXES: .c .o

CC = gcc

BASE_CFLAGS = -std=gnu99 -Wall -Wextra -Wpedantic
CFLAGS = -O2 -g
BASE_LDFLAGS =
LDFLAGS =


all: hpsahba

.c.o:
	$(CC) $(BASE_CFLAGS) $(CFLAGS) -c -o $(@) $(<)

main.o: hpsa.h

hpsahba: main.o
	$(CC) $(BASE_CFLAGS) $(CFLAGS) $(BASE_LDFLAGS) $(LDFLAGS) -o $(@) $(<)

clean:
	rm -f *.o
	rm -f hpsahba

