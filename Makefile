CC = gcc

BUILD_DIR = build/
SRC_DIR = src/
H_DIR = headers/

CFLAGS = -g -I$(H_DIR)

TOTO = $(shell echo "toto est content")
SRC = $(shell find . -name '*.c') 
OBJS = $(SRC:%.c=$(BUILD_DIR)%.o)
HEADERS = $(shell find . -name '*.h') 

PROG = myCanal

all: $(PROG)
	
$(PROG): $(BUILD_DIR)main.o

$(BUILD_DIR)main.o: $(SRC_DIR)main.c $(HEADERS)
	@$(CC) -o $@ -c $< $(CFLAGS) 

$(BUILD_DIR)%.o: %.c $(HEADERS)
	@$(CC) -o $@ -c $< $(CFLAGS)

clean: 
	rm $(PROG)
	rm -f build/*.o 


