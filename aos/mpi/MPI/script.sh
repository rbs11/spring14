for (( p=2; p<=8; p=p*2 ))
do
        for (( t=2; t<=8; t=t*2 ))
        do
                mpirun combo $t -n $p
        done
done


/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 2 diss
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 4 diss
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 6 diss
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 8 diss

/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 2 diss 2
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 2 combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 2 combo 8
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 4 combo 2
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 4 combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 4 combo 8
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 8 combo 2
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 8 combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 8 combo 8
echo "------------------------------"
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 2 combo 2
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 2 combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 2 combo 8
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 4 combo 2
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 4 combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 4 combo 8
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 8 combo 2
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 8 combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 8 combo 8
echo "------------------------------"
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 2 combo 2
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 2 combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 2 combo 8
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 4 combo 2
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 4 combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 4 combo 8
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 8 combo 2
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 8 combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 8 combo 8
echo "------------------------------"
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 2 combo 2
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 2 combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 2 combo 8
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 4 combo 2
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 4 combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 4 combo 8
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 8 combo 2
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 8 combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 8 combo 8
echo "------------------------------"
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 2 combo 2
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 2 combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 2 combo 8
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 4 combo 2
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 4 combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 4 combo 8
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 8 combo 2
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 8 combo 4
/opt/openmpi-1.4.3-gcc44/bin/mpirun -n 8 combo 8
echo "------------END------------------"
