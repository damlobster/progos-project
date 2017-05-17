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
EXECUTABLE = test-inodes test-file test-dirent test-ibm-fbm test-ino-cre

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
	rm -rf disks/*
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



##############################################
#TESTS
tests: test-adding-files build-ourcode-disk

test-adding-files: clean debug
	rm -f tests/test_files.cmd tests/test_sha.cmd tests/test_files.uv6; \
	echo "mkfs tests/test_files.uv6 196 8192" > tests/test_files.cmd; \
	echo "mount tests/test_files.uv6" >> tests/test_files.cmd; \
	echo "psb" >> tests/test_files.cmd; \
	echo "mkdir /tests" >> tests/test_files.cmd; \
	echo "mkdir /tests/files" >> tests/test_files.cmd
	-ls -1A tests/files/* | sed -e 's:\(.*\):add \1 /\1:' >> tests/test_files.cmd
	echo "lsall" >> tests/test_files.cmd
	./shell < tests/test_files.cmd 2>&1 | tee tests/test_files.log
	make clean all
	echo "mount tests/test_files.uv6" > tests/test_sha.cmd;
	-ls -1A tests/files/*.txt | sed -e 's:\(.*\):sha \1:' >> tests/test_sha.cmd
	./shell < tests/test_sha.cmd 2>&1 | sed 's/.* //' | tail -n13 > tests/sha_uv6.log
	sha256sum tests/files/*.txt | sed 's/ .*//' | tail -n13 > tests/sha_unix.log
	-diff -ry tests/sha_uv6.log tests/sha_unix.log | tee tests/test_files.log

build-ourcode-disk: clean debug
	rm -f tests/ourcode.cmd ourcode.uv6; \
	echo "mkfs ourcode.uv6 196 8192" > tests/ourcode.cmd; \
	echo "mount ourcode.uv6" >> tests/ourcode.cmd; \
	echo "psb" >> tests/ourcode.cmd; \
	echo "mkdir /src" >> tests/ourcode.cmd
	-ls -1A *.c *.h *.md | sed -e 's:\(.*\):add \1 /src/\1:' >> tests/ourcode.cmd
	echo "lsall" >> tests/ourcode.cmd
	./shell < tests/ourcode.cmd 2>&1 | tee tests/ourcode.log
