CC = gcc
CLANG_INST = $(shell whereis $(CC))
ifeq "$(CLANG_INST)" "$(CC):"
  $(info CLANG is not installed !!!!!!!!!!!!!!)
  CC = gcc
endif

CFLAGS = -std=c11 -c -Wall -pedantic -Wextra -Wfloat-equal -Wshadow \
-Wpointer-arith -Wbad-function-cast -Wcast-qual -Wcast-align -Wwrite-strings \
-Wunreachable-code
LDFLAGS = -lcrypto

debug: CFLAGS += -g -Wconversion -Wformat -Werror=format-security -Werror=array-bounds \
-Wunused-but-set-variable -Wint-conversion -Wunused-parameter -DDEBUG
#debug: LDFLAGS += -fsanitize=null,leak,bounds,address,undefined

ALL_SOURCES = $(wildcard *.c)
SOURCES = $(filter-out test-%.c shell.c fs.c, $(ALL_SOURCES))
SOURCES += test-core.c
OBJECTS = $(filter-out test-core.o shell.o test-bmb.o, $(SOURCES:.c=.o))
EXECUTABLE = test-inodes test-file test-dirent test-ibm-fbm test-inodes-create

all: $(ALL_SOURCES) $(EXECUTABLE) shell fs test-bitmap

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
	rm -rf *.o $(EXECUTABLE) shell fs test-core test-bitmap
	
clean-disks: 
	cp -f ../provided/disks/* disks/

## non standard builds
fs.o: fs.c
	$(CC) -D_DEFAULT_SOURCE $$(pkg-config fuse --cflags) $(CFLAGS) -o $@ -c $<

fs: fs.o
	$(CC) $(OBJECTS) $(LDFLAGS) $@.o -o $@ $$(pkg-config fuse --libs)

test-bitmap.o: test-bitmap.c
	$(CC) -D_DEFAULT_SOURCE $(CFLAGS) -o $@ -c $<

test-bitmap: test-bitmap.o
	$(CC) bmblock.o $(LDFLAGS) $@.o -o $@
