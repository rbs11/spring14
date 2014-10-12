#include<stdio.h>
#include <mpi.h>
#include<omp.h>
#include<sys/time.h>
#include<stdbool.h>
#include <math.h>
//#define DEBUG
#ifdef DEBUG
#define	print_dbg(fmt, arg...) 	printf(fmt, ## arg)
#else
#define	print_dbg(fmt, arg...)
#endif

#define NUM_BARRIERS 1000
bool sense = true;
bool local_sense =  true;
struct timeval t1,t2;


void dissemination_barrier(int *my_id, int *num_processes)
{
	int procs = *num_processes;
	int id = *my_id;
	int num_rounds = ceil(log2(procs));
	int j ,k, bufval;
	int dest, src;
	for ( k = 0; k < num_rounds; k++)
	{
		MPI_Request rec_handle, send_handle;
		//dest = ((id + (power(2,k)))%procs);
		dest = (id + (int)pow(2,k))%procs;
		src = (procs + id - (int)pow(2,k))%procs;

		print_dbg("---- [%d] --> [%d]-->[%d] __Round %d\n", src,id, dest, k);

		MPI_Irecv( &bufval, 1 ,MPI_INT, src, k, MPI_COMM_WORLD, &rec_handle );

		MPI_Send( &bufval, 1, MPI_INT, dest, k , MPI_COMM_WORLD );

		print_dbg("Sent from [%d] to proc <%d>__Round %d\n", id,dest,k);
		MPI_Wait(&rec_handle,NULL);

		print_dbg("<%d> Completed in Round [%d]\n", id, k);
	}
}


int main(int argc,char *argv[])
{
	int count,num_threads;
	int my_id, num_processes, i;
	int sense = 0, local_sense = 0;
	double tstart, tend, tfinal, time_diff;
	num_threads = *argv[1] - '0';
	char c[1000];
	  FILE *fptr;
	MPI_Init(&argc, &argv);

	MPI_Comm_size(MPI_COMM_WORLD, &num_processes);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
	omp_set_num_threads(num_threads);
	count = num_threads;
	if (my_id == 0)
		tstart = MPI_Wtime();
	for( i=0; i<NUM_BARRIERS; i++ ){
		#pragma omp parallel private(local_sense) shared(count,sense)
		{
			local_sense = true;
			local_sense = !local_sense;

			#pragma omp critical
			{
				count--;
			}
			if(count == 0)
			{
				count = num_threads;
				sense = local_sense;
			}
			else
			{
				while(sense != local_sense)
				{
					//printf("Spinning in %d\n",tid);
				}
			}
			//printf("Leaving thread %d\n",tid);
		}
		dissemination_barrier(&my_id, &num_processes);
	}
	if (my_id == 0){
		tend = MPI_Wtime();
		time_diff = (tend - tstart)/NUM_BARRIERS;
		printf("---HYBRID---Time taken for %d Procs %d threads = %lf\n",num_processes, num_threads,time_diff);
	}
	MPI_Finalize();
	return 0;
}
