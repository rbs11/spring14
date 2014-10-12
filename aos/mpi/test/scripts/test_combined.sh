#!/bin/tcsh
#PBS -q cs6210
#PBS -l nodes=6:sixcore
#PBS -l walltime=00:20:00
#PBS -N recomb
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 2 /nethome/rshetty7/mpi/combo 2
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 2 /nethome/rshetty7/mpi/combo 2
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 2 /nethome/rshetty7/mpi/combo 2
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 2 /nethome/rshetty7/mpi/combo 2
echo "------------------------------"
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 4 /nethome/rshetty7/mpi/combo 2
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 4 /nethome/rshetty7/mpi/combo 2
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 4 /nethome/rshetty7/mpi/combo 2
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 4 /nethome/rshetty7/mpi/combo 2

echo "------------------------------"
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 8 /nethome/rshetty7/mpi/combo 2
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 8 /nethome/rshetty7/mpi/combo 2
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 8 /nethome/rshetty7/mpi/combo 2
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 8 /nethome/rshetty7/mpi/combo 2

echo "------------------------------"
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 2 /nethome/rshetty7/mpi/combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 2 /nethome/rshetty7/mpi/combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 2 /nethome/rshetty7/mpi/combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 2 /nethome/rshetty7/mpi/combo 4
echo "------------------------------"
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 4 /nethome/rshetty7/mpi/combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 4 /nethome/rshetty7/mpi/combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 4 /nethome/rshetty7/mpi/combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 4 /nethome/rshetty7/mpi/combo 4
echo "------------------------------"
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 8 /nethome/rshetty7/mpi/combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 8 /nethome/rshetty7/mpi/combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 8 /nethome/rshetty7/mpi/combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 8 /nethome/rshetty7/mpi/combo 4

echo "------------------------------"
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 2 /nethome/rshetty7/mpi/combo 8
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 2 /nethome/rshetty7/mpi/combo 8
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 2 /nethome/rshetty7/mpi/combo 8
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 2 /nethome/rshetty7/mpi/combo 8
echo "------------------------------"
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 4 /nethome/rshetty7/mpi/combo 8
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 4 /nethome/rshetty7/mpi/combo 8
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 4 /nethome/rshetty7/mpi/combo 8
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 4 /nethome/rshetty7/mpi/combo 8

echo "------------------------------"
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 8 /nethome/rshetty7/mpi/combo 8
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 8 /nethome/rshetty7/mpi/combo 8
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 8 /nethome/rshetty7/mpi/combo 8
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 8 /nethome/rshetty7/mpi/combo 8

echo "------------------------------"
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 2 /nethome/rshetty7/mpi/combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 2 /nethome/rshetty7/mpi/combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 2 /nethome/rshetty7/mpi/combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 2 /nethome/rshetty7/mpi/combo 8
echo "------------------------------"
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 4 /nethome/rshetty7/mpi/combo 2
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 4 /nethome/rshetty7/mpi/combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 4 /nethome/rshetty7/mpi/combo 8
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 4 /nethome/rshetty7/mpi/combo 2
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 4 /nethome/rshetty7/mpi/combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 4 /nethome/rshetty7/mpi/combo 8
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 4 /nethome/rshetty7/mpi/combo 2
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 4 /nethome/rshetty7/mpi/combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 4 /nethome/rshetty7/mpi/combo 8
echo "------------------------------"
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 8 /nethome/rshetty7/mpi/combo 2
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 8 /nethome/rshetty7/mpi/combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 8 /nethome/rshetty7/mpi/combo 8
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 8 /nethome/rshetty7/mpi/combo 2
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 8 /nethome/rshetty7/mpi/combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 8 /nethome/rshetty7/mpi/combo 8
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 8 /nethome/rshetty7/mpi/combo 2
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 8 /nethome/rshetty7/mpi/combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -n 8 /nethome/rshetty7/mpi/combo 8