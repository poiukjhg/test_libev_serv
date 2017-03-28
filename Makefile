PWD:=$(shell pwd)
LOCK_DIR := $(PWD)/mylock
EVENT_DIR := $(PWD)/myevent
EV_DIR := $(PWD)/myev

CC= gcc
OBJS = main
INCLIB   =  -lpthread -levent -lev
INCFILE = -I$(PWD)/mylock -I$(PWD)/myevent -I$(PWD)/myev
CFLAGS   += -Wall  -pg
LOCK_OBJECTS = $(filter-out main.o, $(notdir $(patsubst %.c,%.o,$(wildcard $(LOCK_DIR)/*.c))))
LOCK_SRC = $(filter-out $(LOCK_DIR)/main.c, $(wildcard $(LOCK_DIR)/*.c))
EVENT_OBJECTS = $(filter-out main.o, $(notdir $(patsubst %.c,%.o,$(wildcard $(EVENT_DIR)/*.c))))
EVENT_SRC = $(filter-out $(EVENT_DIR)/main.c, $(wildcard $(EVENT_DIR)/*.c))
EV_OBJECTS = $(filter-out main.o, $(notdir $(patsubst %.c,%.o,$(wildcard $(EV_DIR)/*.c))))
EV_SRC = $(filter-out $(EV_DIR)/main.c, $(wildcard $(EV_DIR)/*.c))
OBJECTS = $(LOCK_OBJECTS)  $(EV_OBJECTS) main.o
#OBJECTS = $(LOCK_OBJECTS)  $(EVENT_OBJECTS) main.o

all: $(OBJS) 
$(OBJS):$(OBJECTS)
	$(CC) $(CFLAGS) -o $@  $(OBJECTS) $(INCLIB) $(INCFILE)
$(LOCK_OBJECTS):$(LOCK_SRC)
	$(CC) $(CFLAGS) -c $< -o $@	$(INCLIB) $(INCFILE)
$(EVENT_OBJECTS):$(EVENT_SRC)
	$(CC) $(CFLAGS) -c $< -o $@	$(INCLIB) $(INCFILE)
$(EV_OBJECTS):$(EV_SRC)
	$(CC) $(CFLAGS) -c $< -o $@	$(INCLIB) $(INCFILE)	
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@	$(INCLIB) $(INCFILE)			
clean:
	rm -rf $(LOCK_DIR)/*.o $(EVENT_DIR)/*.o $(EV_DIR)/*.o *.o
	rm -f $(OBJS) $(LOCK_DIR)/$(OBJS) $(EVENT_DIR)/$(OBJS) $(EV_DIR)/$(OBJS)
	rm -rf *.o 

