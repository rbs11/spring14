OMPFLAGS = -fopenmp
OMPLIBS = -lgomp

CC = gcc
MPICC = mpicc
CPPFLAGS = -g -Wall $(OMPFLAGS)
LDFLAGS = -g -Wall -lm
LDLIBS = $(OMPLIBS)

all: barrier_sense tree_barrier diss tourn
tree_final: tree_barrier.o
	$(CC) -o $@ $^ $(OMPFLAGS) -lm

barrier_sense: barrier_sense.o
	$(CC) $(OMPFLAGS) -lm -o $@ $^

clean:
	rm -f tree_barrier.o barrier_sense.o tree_barrier barrier_sense
