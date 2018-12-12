.POSIX:
.SUFFIXES: .c .o

CC = gcc
PANDOC = pandoc

BASE_CFLAGS = -std=gnu99 -Wall -Wextra
CFLAGS = -O2 -g
BASE_LDFLAGS =
LDFLAGS =


all: hpsahba hpsahba.8

.c.o:
	$(CC) $(BASE_CFLAGS) $(CFLAGS) -c -o $(@) $(<)

main.o: hpsa.h

hpsahba: main.o
	$(CC) $(BASE_CFLAGS) $(CFLAGS) $(BASE_LDFLAGS) $(LDFLAGS) -o $(@) $(<)

hpsahba.8: README.md
	$(PANDOC) --from markdown --to man --standalone --metadata "title=HPSAHBA(8)" --output $(@) $(<)

clean:
	rm -f *.o
	rm -f hpsahba
	rm -f hpsahba.8

