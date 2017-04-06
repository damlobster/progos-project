CC = gcc
CLANG_INST = $(shell whereis $(CC))
ifeq "$(CLANG_INST)" "$(CC):"
  $(info CLANG is not installed !!!!!!!!!!!!!!)
  CC = gcc
endif

CFLAGS = -std=c11 -g -Wall -pedantic -Wextra -c -Wfloat-equal -Wshadow \
-Wpointer-arith -Wbad-function-cast -Wcast-qual -Wcast-align -Wwrite-strings \
-Wunreachable-code
LDFLAGS = -lcrypto

debug: CFLAGS += -Wconversion -Wformat -Werror=format-security -Werror=array-bounds
debug: LDFLAGS += -fsanitize=null,leak,bounds,address,undefined

ALL_SOURCES = $(wildcard *.c)
SOURCES = $(filter-out test-%.c shell.c, $(ALL_SOURCES))
SOURCES += test-core.c
OBJECTS = $(SOURCES:.c=.o)
OBJECTS_TESTS = $(filter-out shell.o, $(OBJECTS))
OBJECTS_SHELL = $(filter-out test-core.o, $(OBJECTS))
EXECUTABLE = test-inodes test-file test-dirent

all: $(ALL_SOURCES) $(EXECUTABLE) shell

debug: all

.SECONDEXPANSION:

$(EXECUTABLE): $(OBJECTS_TESTS) $$@.o
	$(CC) $(OBJECTS_TESTS) $@.o -o $@ $(LDFLAGS) 

shell: $(OBJECTS_SHELL) $$@.o
	$(CC) $(OBJECTS_SHELL) $@.o -o $@ $(LDFLAGS) 

.c.o:
	$(CC) $(CFLAGS) $< -o $@ 

clean:
	rm -rf *.o $(EXECUTABLE) shell