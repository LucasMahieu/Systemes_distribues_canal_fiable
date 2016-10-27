CC = gcc

BUILD_DIR = build/
SRC_DIR = src/
H_DIR = headers/

CFLAGS = -Wall -g -I$(H_DIR)
LDFLAGS = 


PROG0_DIR = procTestA/
PROG1_DIR = procTestB/
PROG2_DIR = canal/

PROG0 = $(PROG0_DIR)myProcTestA
PROG1 = $(PROG1_DIR)myProcTestB
PROG2 = $(PROG2_DIR)myCanal

all: $(PROG0) $(PROG1) $(PROG2)
	@(cp $(PROG0) ./)
	@(cp $(PROG1) ./)
	@(cp $(PROG2) ./)

exeA: $(PROG0)
	./myProcTestA
	
exeB: $(PROG1)
	./myProcTestB

exeMyCanal: $(PROG2)
	./myCanal

### Compilation of PROG 0
$(PROG0): $(PROG0_DIR)$(SRC_DIR)*.c 
	@(cd $(PROG0_DIR) && $(MAKE))
$(PROG1): $(PROG1_DIR)$(SRC_DIR)*.c
	@(cd $(PROG1_DIR) && $(MAKE))
$(PROG2): $(PROG2_DIR)$(SRC_DIR)*.c
	@(cd $(PROG2_DIR) && $(MAKE))

.PHONY: clean 

clean: 
	@(cd $(PROG0_DIR) && $(MAKE) $@)
	@rm myProcTestA
	@(cd $(PROG1_DIR) && $(MAKE) $@)
	@rm myProcTestB
	@(cd $(PROG2_DIR) && $(MAKE) $@)
	@rm myCanal


