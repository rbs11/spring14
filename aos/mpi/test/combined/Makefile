#### OpenMP-MPI Makefile

CFLAGS  = -g -fopenmp
LFLAGS  =
CC      = mpicc
RM      = /bin/rm -rf

CUSTOM_SRC = mp_mpi_sendiss.c
CUSTOM = mp_mpi_sendiss

all: $(CUSTOM)

custom: $(CUSTOM)

$(CUSTOM): $(CUSTOM_SRC)
	$(CC) $(CFLAGS) $< -o $@ -lm

clean:
	$(RM) $(CUSTOM)
