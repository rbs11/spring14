/*
 * File: problem1.cpp
 * Description: Program to determine cache stats and DRAM access time
 * Author: Rajath Shetty
 * Comments: This program will take around 30 seconds to run
 */

#include <cstdlib>
#include <iostream>
#include <sys/mman.h>
#include <sys/time.h>
#include <vector>
#include<stdio.h>

#define ITER   10
#define MAX_N 64*1024*1024 
#define MB    (1024*1024)
#define KB		1024
// LLC Parameters assumed
#define START_SIZE 1*MB
#define STOP_SIZE  16*MB


using namespace std;

char array[MAX_N];



/////////////////////////////////////////////////////////
// Provides elapsed Time between t1 and t2 in milli sec
/////////////////////////////////////////////////////////

double elapsedTime(timeval t1, timeval t2){
  double delta;
  delta = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
  delta += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
  return delta; 
}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

double DummyTest(void)
{    
  timeval t1, t2;
  int ii, iterid;

  // start timer
  gettimeofday(&t1, NULL);

  for(iterid=0;iterid<ITER;iterid++){
    for(ii=0; ii< MAX_N; ii++){
      //array[ii] += rand();
    	array[ii] += ii;
    }
  }

  // stop timer
  gettimeofday(&t2, NULL);
 
  return elapsedTime(t1,t2);
}



/////////////////////////////////////////////////////////
// Change this, including input parameters
/////////////////////////////////////////////////////////

double LineSizeTest(void)
{    
  double retval, prev = 0;
  bool bflag = 0;
  timeval t1, t2;
    int tmp, tmp2;
    /*
     * We vary the step sizes at every iteration. This uses the idea that multiple accesses
     * within the same cache line would be much faster than each access on different lines
     * Therefore, by varying the step sizes we try to reach the point where there is spike in
     * access time owing to the change of cache line which will correspond to the line size
     */
    for (int i = 4; i <= 512/sizeof(int); i*=2)
    {
    	/*Repeat to get consistency.*/
  	  for (int j = 0; j < 4; j++) {
  		  	  gettimeofday(&t1, NULL);
  			  for (unsigned int k = 0; k < MAX_N; k++) {
  					  /*Read data in step sizes*/
  					  tmp += array[(k * i) & (MAX_N-1)];
  			  }
  			  gettimeofday(&t2, NULL);

  			  retval = elapsedTime(t1,t2);
  			  /*If there is spike in access time, it would mean that we have reached the ideal line size*/
  			  if ((prev != 0) && (((retval - prev)*100/prev)>30) && !bflag)
			  {
				  printf("\n***Line size is [%d bytes]***\n\n", i);
				  tmp2 = i;
				  bflag = 1;
				  break;
			  }
  			  prev = retval;
  			  //printf("Stride [%d] Delta[%f]\n", i, retval);
  	  }
  	  if (bflag)
  		  break;

    }
  return tmp2;
}


/////////////////////////////////////////////////////////
// Change this, including input parameters
/////////////////////////////////////////////////////////

double CacheSizeTest(void)
{    
  double retval, prev;
  timeval t1, t2;
   int sizes[] = {
                  1 * KB, 4 * KB, 8 * KB, 16 * KB, 32 * KB, 64 * KB, 128 * KB, 256 * KB,
          512 * KB, 1 * MB, 2 * MB, 3 * MB, 4 * MB, 6 * MB, 6432 * KB, 7 * MB, 8 * MB, 10 * MB, 12 * MB
          };
   bool bflag = 0;
   int tmp;

   /*We use different sizes according to the array defined above. For each size, we access the cache 64 million times.
    *These accesses are random accesses within a define range. Once we use a size which is greater than the available
    *cache size, ie, working set greater than cache capacity, we will see more memory accesses which increase the average
    *access time.*/
      for (int i = 0; i <= sizeof(sizes) / sizeof(int) - 1; i++)
      {
    		  	  gettimeofday(&t1, NULL);
    			  for (unsigned int k = 0; k < MAX_N; k++) {
    				  ++array[(k * rand()) % sizes[i]];
    			  }
    			  gettimeofday(&t2, NULL);

    			  retval = elapsedTime(t1,t2);
    			  /*If the difference of timing is greater than 25 percent, ie, there is a significant spike in
    			   * access time, we can conclude that we have reached the max capacity of the cache.*/
    			  if ((prev != 0) && (((retval - prev)*100/prev)>25) && !bflag)
    			  {
    				  printf("***LLC capacity is [%d]KB***\n\n", sizes[i]/KB);
    				  bflag = 1;
    				  break;
    			  }
    			  prev = retval;
    			  //printf("Size [%d] Delta[%f]\n", sizes[i]/KB, retval);
      }
  return retval; 
}




/////////////////////////////////////////////////////////
// Change this, including input parameters
/////////////////////////////////////////////////////////
double MemoryTimingTest(void)
{
		timeval t1, t2, t3, t4;
		double retval;
		double accesstime = 0;
		int rand_arr[16*KB];
		int tmp;

		/*Read entire 64MB of data from start to finish so that the first (64-LLC size)MB of data will
		 * NOT be in the cache for sure and will trigger capacity misses resulting in memory access*/
		for (unsigned int k = 0; k < MAX_N; k++) {
			  ++array[k];
		}


		/*Create an array of random numbers. This array will be in the cache since it is latest accessed data
		 * These random numbers will be used to randomly traverse the first (64-LLC size)MB of data..*/
		for (int j = 0; j < 16*KB; j++)
		{
			rand_arr[j] = rand()%(48*MB);
		}

	  	gettimeofday(&t1, NULL);
	  	/*We can ignore the access time of rand_arr because this will have close to 16KB of data which will easily
	  	 * fit into the l1 Cache and therefore will always be accessed from the cache*/
	  	for (int j = 0; j < 16*KB; j++){
	  		++array[rand_arr[j]];
	  	}
	  	gettimeofday(&t2, NULL);

	  	retval = elapsedTime(t1,t2);
	  	accesstime = retval/(16*KB);
	  	printf("***DRAM Accesstime [%f ns]***\n", (accesstime*1000000));
	  return accesstime;
}


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

int main(){
  
  cout << "\nStarting DummyTest:" << "\n";
  cout << "Test took " << DummyTest()/1000.0 << " seconds\n";
  // comments are in individual functions
  LineSizeTest ();
  CacheSizeTest();
  MemoryTimingTest();
  cout << "\n";
}

/////////////////////////////////////////////////////////
/*
 * 	gettimeofday(&t3, NULL);
	++array[1103];
	++array[15];
	++array[552];
	++array[84];
	++array[5989];
	++array[150];
	++array[501];
	++array[10001];
	++array[989];
	++array[8999];
	++array[89994];
	++array[15000];
	++array[25030];
	++array[2503];
	++array[32908];
	++array[21503];
	++array[52503];
	++array[25803];
	++array[7543];
	++array[9400];

	gettimeofday(&t4, NULL);
	retval = elapsedTime(t3,t4);
	accesstime = retval/20;
	printf("Truly Random Accesstime [%f ns]\n", (accesstime*1000000));
	printf("retval [%f] t3.tv_usec[%ld] t4.tv_usec [%ld]\n", retval, t3.tv_usec, t4.tv_usec);
 * */
/////////////////////////////////////////////////////////
