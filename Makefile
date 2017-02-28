PWD:=$(shell pwd)
LOCK_DIR := $(PWD)/mylock
EV_DIR := $(PWD)/myev

CC= gcc
OBJS = main
INCLIB   =  -lpthread -levent 
CFLAGS   += -Wall 
LOCK_OBJECTS = $(patsubst %.c,%.o,$(wildcard $(LOCK_DIR)/*.c)) 
EV_OBJECTS =  $(patsubst %.c,%.o,$(wildcard $(EV_DIR)/*.c))
OBJECTS = $(LOCK_OBJECTS) $(EV_OBJECTS)

all: $(OBJS) 
$(OBJS):$(OBJECTS)
	$(CC) $(CFLAGS) -o $@  $(OBJECTS) $(INCLIB)
$(OBJECTS): $(LOCK_DIR)/%.c $(EV_DIR)/%.c 
	$(CC) $(CFLAGS) -c $< -o $@	$(INCLIB)
clean:
	rm -rf $(LOCK_DIR)/*.o $(EV_DIR)/*.o *.o
	rm -f $(OBJS)

