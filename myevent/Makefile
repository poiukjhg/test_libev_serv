PWD:=$(shell pwd)
LOCK_DIR := $(PWD)/../mylock
CC= gcc

OBJS = main
INC  = -I$(PWD)/../mylock
INCLIB   =  -lpthread -levent 
CFLAGS   += -Wall -g
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))
LOCK_OBJECTS = $(filter-out main.o, $(notdir $(patsubst %.c,%.o,$(wildcard $(LOCK_DIR)/*.c))))

all: $(OBJS) 

$(OBJS):$(OBJECTS) $(LOCK_OBJECTS) 
	$(CC) $(CFLAGS) -o $@  $(OBJECTS) $(LOCK_OBJECTS) $(INCLIB) $(INC)
$(LOCK_OBJECTS): $(filter-out $(LOCK_DIR)/main.c, $(wildcard $(LOCK_DIR)/*.c))
	$(CC) $(CFLAGS) -c $< -o $@	$(INCLIB)		
%.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@	$(INCLIB) $(INC)

clean:
	rm -rf *.o 
	rm -f $(OBJS)
	rm -f $(LOCK_OBJECTS) 

