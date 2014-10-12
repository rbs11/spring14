#include<stdio.h>
#include<omp.h>
#include<sys/time.h>
#include<stdbool.h>

bool sense = true;
bool local_sense =  true;
struct timeval t1,t2;

int main(int argc,char *argv[])
{
	int i,count,num_threads;
	double time_diff;
	num_threads = *argv[1] - '0';
	printf("num_threads = %d\n",num_threads);
	omp_set_num_threads(num_threads);
	count = num_threads;
	for(i=0;i<20;i++)
	{
	gettimeofday(&t1,NULL);
	#pragma omp parallel private(local_sense) shared(count,sense)
	{
		//printf("Entering thread %d\n",tid);		
		local_sense = true;		
		local_sense = !local_sense;
		
		#pragma omp critical
		{
			count--;
		}
		//printf("count = %d\n",count);		
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
	gettimeofday(&t2,NULL);
	if(i>=10)
		time_diff+= ((t2.tv_sec*1000.0 + t2.tv_usec/1000.0) - (t1.tv_sec*1000.0 + t1.tv_usec/1000.0));

	}
	printf("Average Time taken for %d threads = %lfms\n",num_threads,time_diff/10.0);
	return 0;
}
