#include "gtthread.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
gtthread_t threads[10];
gtthread_mutex_t test;
void *thread_func( void *id ){
	printf("Hello from %ld thread\n", (int)id);
	//gtthread_exit( NULL );
	//gtthread_mutex_lock(&test);
	//printf("%ld Has acquired mutex\n", (int)id);
	//gtthread_mutex_unlock(&test);
}

void main(){
	gtthread_init( 1000 );
	int i;


	gtthread_mutex_init(&test);
	for (i = 0; i < 10; i++)
    	        gtthread_create (&threads[i], thread_func, (void *)i);
  	    for (i = 0; i < 10; i++)
  	    {
    	        //gtthread_join (threads[i], NULL);
    	        //printf("Thread[%d] has exited\n", i);
  	    }
  	    usleep(10000000);
  	    printf("Main exit\n");
}

