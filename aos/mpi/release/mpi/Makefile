#### MPI Makefile

CFLAGS  = -g
C1FLAGS	= -lm
LFLAGS  =
CC      = mpicc
RM      = /bin/rm -rf

DISSEMINATION_SRC = diss.c
DISSEMINATION = diss

TM_SRC = tourn.c
TM = tourn

all: $(DISSEMINATION) $(TM)

dissemination: $(DISSEMINATION)

mcs: $(TM)

$(DISSEMINATION): $(DISSEMINATION_SRC)
	$(CC) $(CFLAGS) $< -o $@ $(C1FLAGS)

$(TM): $(TM_SRC)
	$(CC) $(CFLAGS) $< -o $@ $(C1FLAGS)
clean:
	$(RM) $(DISSEMINATION) $(TM)
