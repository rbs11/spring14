#### OMP Makefile

CFLAGS  = -g -fopenmp  
C1FLAGS	= -DMAX_NUM_PROCS=16 -DCACHE_LINESIZE=`getconf LEVEL1_DCACHE_LINESIZE`
LFLAGS  =
CC      = gcc
RM      = /bin/rm -rf

SENSE_SRC = barrier_sense.c
SENSE = barrier_sense

TREE_SRC = tree_barrier.c
TREE = tree_barrier

all: $(SENSE) $(TREE) $(BUILTIN)

sense: $(SENSE)

tree: $(TREE)

builtin: $(BUILTIN)

$(SENSE): $(SENSE_SRC)
	$(CC) $(CFLAGS) $< -o $@

$(TREE): $(TREE_SRC)
	$(CC) $(CFLAGS) $(C1FLAGS) $< -o $@ -lm

clean:
	$(RM) $(SENSE) $(TREE) $(BUILTIN)
