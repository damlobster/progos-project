CC = gcc
CLANG_INST = $(shell whereis $(CC))
ifeq "$(CLANG_INST)" "$(CC):"
  $(info CLANG is not installed !!!!!!!!!!!!!!)
  CC = gcc
endif

CFLAGS = -g -std=c11 -Wall -pedantic -Wextra -c -Wfloat-equal -Wshadow \
-Wpointer-arith -Wbad-function-cast -Wcast-qual -Wcast-align -Wwrite-strings \
-Wunreachable-code
LDFLAGS = -lcrypto

debug: CFLAGS += -g -Wconversion -Wformat -Werror=format-security -Werror=array-bounds
debug: LDFLAGS += -fsanitize=null,leak,bounds,address,undefined

ALL_SOURCES = $(wildcard *.c)
SOURCES = $(filter-out test-%.c shell.c fs.c, $(ALL_SOURCES))
SOURCES += test-core.c
OBJECTS = $(filter-out test-core.o shell.o, $(SOURCES:.c=.o))
EXECUTABLE = test-inodes test-file test-dirent

all: $(ALL_SOURCES) $(EXECUTABLE) shell fs

debug: all
	$(info debug build!!!)

.SECONDEXPANSION:

$(EXECUTABLE): $(OBJECTS) test-core.o $$@.o
	$(CC) $(OBJECTS) test-core.o $@.o -o $@ $(LDFLAGS) 

shell: $(OBJECTS) shell.o $$@.o
	$(CC) $(OBJECTS) $@.o -o $@ $(LDFLAGS) 

.c.o:
	$(CC) $(CFLAGS) $< -o $@ 

clean:
	rm -rf *.o $(EXECUTABLE) shell fs
	
fs.o: fs.c
	$(CC) -D_DEFAULT_SOURCE $$(pkg-config fuse --cflags) -g -o $@ -c $<

fs: fs.o
	$(CC) $(OBJECTS) $(LDFLAGS) $@.o -o $@ $$(pkg-config fuse --libs)