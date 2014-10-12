#### GTThread Library Makefile

CFLAGS  = -g -Wall -pedantic
LFLAGS  =
CC      = gcc
RM      = /bin/rm -rf
AR      = ar rc
RANLIB  = ranlib

LIBRARY = gtthread.a

LIB_SRC = gtthread.c gtthread_sched.c gtthread_mutex.c

LIB_OBJ = gtthread.o gtthread_sched.o gtthread_mutex.o

# pattern rule for object files
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@


all: $(LIB_OBJ)

$(LIB_OBJ) :$(LIB_SRC)
	$(CC) -c $(CFLAGS) gtthread.c -o gtthread.o
	$(CC) -c $(CFLAGS) gtthread_sched.c -o gtthread_sched.o 
	$(CC) -c $(CFLAGS) gtthread_mutex.c -o gtthread_mutex.o

$(LIB_SRC):

clean:
	        $(RM) $(LIB_OBJ) 


#gcc main.c *.o

.PHONY: depend
depend:
	$(CFLAGS) -- $(LIB_SRC)  2>/dev/null
