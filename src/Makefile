CC = gcc
CFLAGS = -Werror -Wall -g -ggdb
DISABLE_FLAGS = -Wno-missing-braces

INCLUDE = -isystem C:/c_libs/include/ -LC:/c_libs/lib/


C_SOURCES = $(wildcard *.c */*.c */*/*.c)
HEADERS = $(wildcard *.h */*.h */*/*.h)

BINARY_DIR = ../bin/

all: modl2obj obj2modl

modl2obj:
	$(CC) -o $(BINARY_DIR)$@.exe $@.c $(CFLAGS) $(DISABLE_FLAGS) $(INCLUDE)

obj2modl:
	$(CC) -o $(BINARY_DIR)$@.exe $@.c $(CFLAGS) $(DISABLE_FLAGS) $(INCLUDE)

clean:
	rm -rf *.o
	rm -rf */*.o
	rm log.txt
	rm ../*.exe