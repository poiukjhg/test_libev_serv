PWD:=$(shell pwd)
CC= gcc

OBJS = main
INCLIB   = -levent  -lpthread
CFLAGS   += -Wall 
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))

all: $(OBJS) 
$(OBJS):$(OBJECTS)
	$(CC) $(CFLAGS) -o $@  $(OBJECTS) $(INCLIB)
%.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@	$(INCLIB)
clean:
	rm -rf *.o 
	rm -f $(OBJS)

