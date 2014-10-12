#!/bin/tcsh
#PBS -q cs6210
#PBS -l nodes=6:sixcore
#PBS -l walltime=00:20:00
#PBS -N mpi_diss_tourn

for i in {1..100}
do
	/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 2 /nethome/rshetty7/mpi/tourn
done
for i in {1..100}
do
	/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 4 /nethome/rshetty7/mpi/tourn
done
for i in {1..100}
do
	/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 6 /nethome/rshetty7/mpi/tourn
done
for i in {1..100}
do
	/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 8 /nethome/rshetty7/mpi/tourn
done
for i in {1..100}
do
	/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 12 /nethome/rshetty7/mpi/tourn
done

for i in {1..100}
do
	/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 2 /nethome/rshetty7/mpi/diss
done
for i in {1..100}
do
	/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 4 /nethome/rshetty7/mpi/diss
done
for i in {1..100}
do
	/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 6 /nethome/rshetty7/mpi/diss
done
for i in {1..100}
do
	/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 8 /nethome/rshetty7/mpi/diss
done
for i in {1..100}
do
	/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 12 /nethome/rshetty7/mpi/diss
done

