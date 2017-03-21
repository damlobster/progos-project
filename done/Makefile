CC = clang
CFLAGS = -std=c11 -g -Wall -pedantic -Wextra
LDFLAGS = 
ALL_SOURCES = $(wildcard *.c)
SOURCES = $(filter-out test-*.c, $(ALL_SOURCES)) test-core.c 
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = test-inodes test-file

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $@.c -o $@

clean:
	rm -rf *.o test-inodes
	