CC = gcc
CLANG_INST = $(shell whereis $(CC))
ifeq "$(CLANG_INST)" "$(CC):"
  $(info CLANG is not installed !!!!!!!!!!!!!!)
  CC = gcc
endif

CFLAGS = -std=c11 -g -Wall -pedantic -Wextra -c -Wfloat-equal -Wshadow \
-Wpointer-arith -Wbad-function-cast -Wcast-qual -Wcast-align -Wwrite-strings \
-Wunreachable-code

all-with-Wconversion: CFLAGS += -Wconversion

LDFLAGS = -lcrypto
ALL_SOURCES = $(wildcard *.c)
SOURCES = $(filter-out test-%.c, $(ALL_SOURCES))
SOURCES += test-core.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = test-inodes test-file test-dirent

all: $(ALL_SOURCES) $(EXECUTABLE)

all-with-Wconversion: all

.SECONDEXPANSION:

$(EXECUTABLE): $(OBJECTS) $$@.o
	$(CC) $(OBJECTS) $@.o -o $@ $(LDFLAGS) 

.c.o:
	$(CC) $(CFLAGS) $< -o $@ 

clean:
	rm -rf *.o $(EXECUTABLE)