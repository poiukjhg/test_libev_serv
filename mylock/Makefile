PWD:=$(shell pwd)
CC= gcc

OBJS = main.o
INCLIB   = -levent
CFLAGS   += -Wall 

all: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) 
%.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@	-L$(INCLIB)
clean:
	rm -rf *.o 
endif
