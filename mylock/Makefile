PWD:=$(shell pwd)
CC= gcc

OBJS = main
INCLIB   = -levent  -lpthread
CFLAGS   += -Wall 
OBJECTS = mylock.o main.o

all: $(OBJS) 
$(OBJS):$(OBJECTS)
	$(CC) $(CFLAGS) -o $@  $(OBJECTS) -L$(INCLIB)
%.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@	-L$(INCLIB)
clean:
	rm -rf *.o 
	rm -f $(OBJS)

