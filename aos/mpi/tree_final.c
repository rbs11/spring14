#include<stdio.h>
#include<sys/time.h>
#include<omp.h>
#include<stdbool.h>
#include<math.h>
#include<stdlib.h>
//#define DEBUG 1

int *count,levels,N;
bool *lock_sense;
struct timeval t1,t2;
void recurse(int temp, bool *local_sense)
{
#if DEBUG
	printf("TEMP = %d\n",temp);
#endif
	if(temp >= N-1)
	{
		return;
	}	
	else
	{
		#pragma omp critical
		{
			count[temp]--;
		}
		if(count[temp] == 0)
		{
			recurse(temp/2 + N/2,local_sense);
			lock_sense[temp] = *local_sense;
		}
		else
		{
#if DEBUG
			printf("Spinning in thread %d\n",2*(temp-N/2));	
#endif
			while(lock_sense[temp] != *local_sense);
			
#if DEBUG
			printf("lock_Sense[%d] unlocked!!\n",temp);
#endif
		}
		//printf("Leaving thread %d!\n",temp);		
		
	}
//	lock_sense[temp]
}

int main(int argc, char *argv[])
{
	double time_diff;
	int i;
	bool local_sense;
	N = atoi(argv[1]);
	printf("num_threads = %d\n",N);
	count  = (int *)malloc(sizeof(int)*N);
	lock_sense  = (bool *)malloc(sizeof(bool)*N);
#if DEBUG
	printf("N = %d\n",N);	
#endif
	for(i=0;i<N;i++)
	{
		lock_sense[i] = true;
		count[i] = 2;
	}
	levels = ceil(log2(N));
	for(i=0;i<20;i++)
	{
	gettimeofday(&t1,NULL);
	#pragma omp parallel private(local_sense) shared(count,lock_sense) num_threads(N)
	{
		int tid = omp_get_thread_num();
#if DEBUG
		printf("Entering thread %d\n",tid);		
#endif
		local_sense = true;
		local_sense = !local_sense;
		#pragma omp critical 
		{
#if DEBUG
			printf("Decremented by %d\n",tid);			
#endif
			count[tid/2]--;
		}
		if(count[tid/2] == 0)
		{
#if DEBUG
			printf("TID = %d\n",tid);
#endif
			recurse((int)tid/4 + (N/2),&local_sense);
			lock_sense[tid/2] = local_sense;
		}
		else
		{
#if DEBUG
			printf("Spinning in thread %d\n",tid);
#endif
			while(lock_sense[tid/2] != local_sense);
#if DEBUG
			printf("lock_Sense[%d] unlocked\n",tid);
#endif			
	
		}
#if DEBUG
		printf("Leaving thread %d\n",tid);		
#endif
	}
	gettimeofday(&t2,NULL);
	//printf("Time taken for %d threads = %lfms\n",N,((t2.tv_sec*1000.0 + t2.tv_usec/1000.0) - (t1.tv_sec*1000.0 + t1.tv_usec/1000.0)));
	if(i>=10)
		time_diff+= ((t2.tv_sec*1000.0 + t2.tv_usec/1000.0) - (t1.tv_sec*1000.0 + t1.tv_usec/1000.0));

	}
	printf("Average Time taken for %d threads = %lfms\n",N,time_diff/10.0);
	
	return 0;
}
