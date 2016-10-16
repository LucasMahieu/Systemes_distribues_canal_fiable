CC = gcc

BUILD_DIR = build/
SRC_DIR = src/
H_DIR = headers/

CFLAGS = -Wall -g -I$(H_DIR)
LDFLAGS = 

SRC = $(shell find . -name '*.c') 
OBJS = $(SRC:%.c=$(BUILD_DIR)%.o)
HEADERS = $(shell find . -name '*.h') 

PROG0_DIR = procTestA/
PROG1_DIR = procTestB/
PROG2_DIR = canal/

PROG0 = myProcTestA
PROG1 = myProcTestB
PROG2 = myCanal

all: $(PROG0) $(PROG1) $(PROG2)

exeA: $(PROG0)
	./$(PROG0)
	
exeB: $(PROG1)
	./$(PROG1)

exeMyCanal: $(PROG2)
	./$(PROG2) $(ARG)

### Compilation of PROG 0
$(PROG0): $(PROG0_DIR)$(BUILD_DIR)main.o
	@$(CC) -o $@ $^ $(LDFLAGS)

$(PROG0_DIR)$(BUILD_DIR)main.o: $(PROG0_DIR)$(SRC_DIR)main.c $(PROG0_DIR)$(HEADERS)
	@$(CC) -o $@ -c $< $(CFLAGS) 

$(PROG0_DIR)$(BUILD_DIR)%.o: $(PROG0_DIR)%.c $(PROG0_DIR)$(HEADERS)
	@$(CC) -o $@ -c $< $(CFLAGS)

### Compilation of PROG 1
$(PROG1): $(PROG1_DIR)$(BUILD_DIR)main.o
	@$(CC) -o $@ $^ $(LDFLAGS)

$(PROG1_DIR)$(BUILD_DIR)main.o: $(PROG1_DIR)$(SRC_DIR)main.c $(PROG1_DIR)$(HEADERS)
	@$(CC) -o $@ -c $< $(CFLAGS) 

$(PROG1_DIR)$(BUILD_DIR)%.o: $(PROG1_DIR)%.c $(PROG1_DIR)$(HEADERS)
	@$(CC) -o $@ -c $< $(CFLAGS)

### Compilation of PROG 2
$(PROG2): $(PROG2_DIR)$(BUILD_DIR)main.o
	@$(CC) -o $@ $^ $(LDFLAGS)

$(PROG2_DIR)$(BUILD_DIR)main.o: $(PROG2_DIR)$(SRC_DIR)main.c $(PROG2_DIR)$(HEADERS)
	@$(CC) -o $@ -c $< $(CFLAGS) 

$(PROG2_DIR)$(BUILD_DIR)%.o: $(PROG2_DIR)%.c $(PROG2_DIR)$(HEADERS)
	@$(CC) -o $@ -c $< $(CFLAGS)


.PHONY: clean 
clean: 
	rm -f $(PROG0_DIR)build/*.o 
	rm -f $(PROG1_DIR)build/*.o 
	rm -f $(PROG2_DIR)build/*.o 
	rm $(PROG0) $(PROG1) $(PROG2)


