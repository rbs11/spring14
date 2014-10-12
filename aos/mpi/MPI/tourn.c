/*
 * tourn.c
 *
 *  Created on: Feb 25, 2014
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

void tournament_barrier(int *my_id, int *num_processes, int round)
{
	int id = *my_id;
	int nprocs = *num_processes;
	int winner = 0;
	int bufval = 0, src, dest;
	MPI_Request rec_handle;


	print_dbg("Round %d Id %d", round, id);
	if (round > ceil(log2(nprocs))){
		print_dbg("Exit\n");
		return;
	}
	print_dbg("\n");
	if ((id % (int)pow(2,round)) == 0)
	{
		winner = 1;
		src = id + (int)pow(2,round-1);
		if (src >= nprocs){
			print_dbg("Bye for id %d, ignore %d and move up\n", id, src);
			tournament_barrier(my_id, num_processes, round+1);
		}
		else{
			MPI_Irecv( &bufval, 1 ,MPI_INT, src, 1, MPI_COMM_WORLD, &rec_handle );
			MPI_Wait(&rec_handle,NULL);
			print_dbg("Winner %d woken up by <%d>\n", id, src);
			tournament_barrier(my_id, num_processes, round+1);
			/*Wake up corresponding loser*/
			MPI_Send( &bufval, 1, MPI_INT, src, 1 , MPI_COMM_WORLD );
			print_dbg("Winner %d shakes hand with loser <%d>\n", id, src);
		}
	}
	else
	{
		winner = 0;
		dest = id - (int)pow(2,round-1);
		print_dbg("Loser <%d> wake up Winner %d\n", id, dest);
		MPI_Send( &bufval, 1, MPI_INT, dest, 1 , MPI_COMM_WORLD );
		/*Wait for wake up by winner*/
		MPI_Irecv( &bufval, 1 ,MPI_INT, dest, 1, MPI_COMM_WORLD, &rec_handle );
		MPI_Wait(&rec_handle,NULL);
		print_dbg("Loser %d woken up by <%d>\n", id, dest);
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

	  tournament_barrier(&my_id, &num_processes, 1);
	  print_dbg("Process %d completed barrier %d\n", my_id, i);
  }
  if (my_id == 0){
	  tfinal = MPI_Wtime();
	  tend =  (double)((tfinal - tstart)/NUM_BARRIERS);
	  printf("[Tournament]Time for %d Processes [%f]\n", num_processes, tend);
  }
  MPI_Finalize();
  return 0;
}
