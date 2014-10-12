OMPFLAGS = -fopenmp
OMPLIBS = -lgomp

CC = gcc
CPPFLAGS = -g -Wall $(OMPFLAGS)
LDFLAGS = -g -Wall -lm
LDLIBS = $(OMPLIBS)

all: barrier_sense tree_final
tree_final: tree_final.o
	$(CC) -o $@ $^ $(OMPFLAGS) -lm

barrier_sense: barrier_sense.o
	$(CC) $(OMPFLAGS) -lm -o $@ $^

clean:
	rm -f tree_final.o barrier_sense.o
