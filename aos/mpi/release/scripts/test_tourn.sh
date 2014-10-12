#!/bin/tcsh
#PBS -q cs6210
#PBS -l nodes=6:sixcore
#PBS -l walltime=00:20:00
#PBS -N tourn_latest
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 2 /nethome/rshetty7/mpi/tourn
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 4 /nethome/rshetty7/mpi/tourn
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 6 /nethome/rshetty7/mpi/tourn
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 8 /nethome/rshetty7/mpi/tourn
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 10 /nethome/rshetty7/mpi/tourn
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 12 /nethome/rshetty7/mpi/tourn
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 16 /nethome/rshetty7/mpi/tourn


