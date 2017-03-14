CC = clang
CFLAGS = -std=c11 -g -Wall -pedantic -Wextra
LDFLAGS =
SOURCES = error.c mount.c sector.c test-core.c test-inodes.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = test-core

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

clean:
	rm -rf *.o test-core
	