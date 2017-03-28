CC = clang
CLANG_INST = $(shell whereis $(CC))
ifeq "$(CLANG_INST)" "$(CC):"
  $(info CLANG is not installed !!!!!!!!!!!!!!)
  CC = gcc
endif


CFLAGS = -std=c11 -g -Wall -pedantic -Wextra -c
LDFLAGS = -lcrypto
ALL_SOURCES = $(wildcard *.c)
SOURCES = $(filter-out test-%.c, $(ALL_SOURCES))
SOURCES += test-core.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = test-inodes test-file

all: $(ALL_SOURCES) $(EXECUTABLE)

.SECONDEXPANSION:

$(EXECUTABLE): $(OBJECTS) $$@.o
	$(CC) $(LDFLAGS) $(OBJECTS) $@.o -o $@ 


.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o test-inodes