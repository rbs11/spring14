// Test3
// Main exit. Program must terminate smoothly.

#include <stdio.h>
#include "../gtthread.h"

void* worker(void* arg)
{
	//for(;;);
	while(1);// printf("still ");
}

int main()
{
	gtthread_t th1;

	gtthread_init(1000);
	printf("init exit\n");
	gtthread_create(&th1, worker, NULL);
	printf("create exit\n");
	printf("main exit\n");

	gtthread_exit(NULL);
	printf("Wrong behavior\n");
	return 0;
}
