PWD:=$(shell pwd)
LOCK_DIR := $(PWD)/mylock
EV_DIR := $(PWD)/myevent

CC= gcc
OBJS = main
INCLIB   =  -lpthread -levent 
INCFILE = -I$(PWD)/mylock -I$(PWD)/myevent
CFLAGS   += -Wall  -g
LOCK_OBJECTS = $(filter-out main.o, $(notdir $(patsubst %.c,%.o,$(wildcard $(LOCK_DIR)/*.c))))
LOCK_SRC = $(filter-out $(LOCK_DIR)/main.c, $(wildcard $(LOCK_DIR)/*.c))
EV_OBJECTS = $(filter-out main.o, $(notdir $(patsubst %.c,%.o,$(wildcard $(EV_DIR)/*.c))))
EV_SRC = $(filter-out $(EV_DIR)/main.c, $(wildcard $(EV_DIR)/*.c))
OBJECTS = $(LOCK_OBJECTS) $(EV_OBJECTS) main.o


all: $(OBJS) 
$(OBJS):$(OBJECTS)
	$(CC) $(CFLAGS) -o $@  $(OBJECTS) $(INCLIB) $(INCFILE)
$(LOCK_OBJECTS):$(LOCK_SRC)
	$(CC) $(CFLAGS) -c $< -o $@	$(INCLIB) $(INCFILE)
$(EV_OBJECTS):$(EV_SRC)
	$(CC) $(CFLAGS) -c $< -o $@	$(INCLIB) $(INCFILE)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@	$(INCLIB) $(INCFILE)			
clean:
	rm -rf $(LOCK_DIR)/*.o $(EV_DIR)/*.o *.o
	rm -f $(OBJS)
	rm -rf *.o 

