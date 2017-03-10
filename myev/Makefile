PWD:=$(shell pwd)
LOCK_DIR := $(PWD)/../mylock
CC= gcc

OBJS = main
INC  = -I$(PWD)/../mylock
INCLIB   =  -lpthread -lev 
CFLAGS   += -Wall -g
LOCK_OBJECTS = $(filter-out main.o, $(notdir $(patsubst %.c,%.o,$(wildcard $(LOCK_DIR)/*.c))))
LOCK_SRC = $(filter-out $(LOCK_DIR)/main.c, $(wildcard $(LOCK_DIR)/*.c))
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c)) $(LOCK_OBJECTS)

all: $(OBJS) 

$(OBJS):$(OBJECTS)
	$(CC) $(CFLAGS) -o $@  $(OBJECTS) $(INCLIB) $(INC)
$(LOCK_OBJECTS): $(LOCK_SRC)
	$(CC) $(CFLAGS) -c $< -o $@	$(INCLIB)		
%.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@	$(INCLIB) $(INC)

clean:
	rm -rf *.o 
	rm -f $(OBJS)
	rm -f $(LOCK_OBJECTS) 

