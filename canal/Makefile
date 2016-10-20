CC = gcc

BUILD_DIR = build/
SRC_DIR = src/
H_DIR = headers/

CFLAGS = -Wall -g -I$(H_DIR)
LDFLAGS = 

SRC = $(shell find . -name '*.c') 
OBJS = $(SRC:%.c=$(BUILD_DIR)%.o)
HEADERS = $(shell find . -name '*.h') 

PROG = myCanal

all: $(PROG)

exeMyCanal: $(PROG)
	./$(PROG)

myCanal: $(OBJS)
	@$(CC) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)main.o: $(SRC_DIR)main.c $(HEADERS)
	@$(CC) -o $@ -c $< $(CFLAGS) 

$(BUILD_DIR)%.o: %.c $(HEADERS)
	@$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean, all

clean: 
	rm -f build/*/*.o 
	rm $(PROG)


