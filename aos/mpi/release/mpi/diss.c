/*
 * diss.c
 *
 *  Created on: Feb 24, 2014
 *      Author: root
 */

#include <stdio.h>
#include <mpi.h>
#include <math.h>

#define NUM_BARRIERS 1000
//#define DEBUG
#ifdef DEBUG
#define	print_dbg(fmt, arg...) 	printf(fmt, ## arg)
#else
#define	print_dbg(fmt, arg...)
#endif

void dissemination_barrier(int *my_id, int *num_processes)
{
	int procs = *num_processes;
	int id = *my_id;
	int num_rounds = ceil(log2(procs));
	int k, bufval;
	int dest, src;
	for ( k = 0; k < num_rounds; k++)
	{
		MPI_Request rec_handle;
		dest = ((id + (int)pow(2,k))%procs);
		src = (((procs + id - ((int)pow(2,k))))%procs);

		print_dbg("---- [%d] --> [%d]-->[%d] __Round %d\n", src,id, dest, k);

		MPI_Irecv( &bufval, 1 ,MPI_INT, src, k, MPI_COMM_WORLD, &rec_handle );

		MPI_Send( &bufval, 1, MPI_INT, dest, k , MPI_COMM_WORLD );

		print_dbg("Sent from [%d] to proc <%d>__Round %d\n", id,dest,k);
		MPI_Wait(&rec_handle,NULL);

		print_dbg("<%d> Completed in Round [%d]\n", id, k);
	}
}

int main(int argc, char **argv)
{
  int my_id, num_processes, i;
  double tstart, tend, tfinal;

  MPI_Init(&argc, &argv);

  MPI_Comm_size(MPI_COMM_WORLD, &num_processes);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
  if (my_id == 0)
	  tstart = MPI_Wtime();
  for( i=0; i < NUM_BARRIERS; i++ ){

	  dissemination_barrier(&my_id, &num_processes);
  }
  if (my_id == 0){
	  tfinal = MPI_Wtime();
	  tend =  (double)((tfinal - tstart)/NUM_BARRIERS);
	  printf("<Dissemination> Time for %d Processes [%f]\n", num_processes, tend);
  }
  MPI_Finalize();
  return 0;
}
