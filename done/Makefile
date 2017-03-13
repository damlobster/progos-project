CFLAGS = -c99
CFLAGS += -g
CFLAGS += -W
CFLAGS += -Wall
CFLAGS += -ansi
CFLAGS += -pedantic


all: test-inodes

test-inodes: test-core.o error.o test-inodes.o
	gcc test-core.o error.o test-inodes.o

error.o: error.c
	gcc -c -o error.o error.c

test-core.o: test-core.c
	gcc -c -o test-core.o test-core.c

test-inodes.o: test-inodes.c
	gcc -c -o test-inodes.o test-inodes.c

clean:
	rm -rf *.o