CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -pedantic -lm -fcommon
.PHONY=all clean 

all: ipkcpd

ipkcpd: ipkcpd.c loc_errors.h
	$(CC) $(CFLAGS) -o ipkcpd ipkcpd.c 

clean:
	rm ipkcpd *.o