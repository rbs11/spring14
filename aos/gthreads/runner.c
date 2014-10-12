#include <stdio.h>
#include <stdlib.h>
#include "gtthread.h"

gtthread_t t1,t2,t3,t4,t5;
gtthread_mutex_t Res1,Res2,Res3,Res4,Res5;

void *philosopher(void *arg){
	short i;
	gtthread_mutex_t* left = NULL;
	gtthread_mutex_t* right = NULL;
	unsigned int fork_left = 0;
	unsigned int fork_right = 0;
	switch((int)arg){
		case 1:
				left=&Res5;
				right=&Res1;
				fork_left = 5;
				fork_right = 1;
				break;
		case 2:
				left=&Res2;
				right=&Res1;
				fork_left = 2;
				fork_right = 1;
				break;
		case 3:
				left=&Res3;
				right=&Res2;
				fork_left = 3;
				fork_right = 2;
				break;
		case 4:
				left=&Res4;
				right=&Res3;
				fork_left = 4;
				fork_right = 3;
				break;
		case 5:
				left=&Res5;
				right=&Res4;
				fork_left = 5;
				fork_right = 4;
				break;
	}
	do{
		gtthread_mutex_lock((gtthread_mutex_t*)left);
		printf("Philosopher #%d acquired chopstick %d\n",(int)arg,fork_left);
		gtthread_mutex_lock((gtthread_mutex_t*)right);
		printf("Philosopher #%d acquired chopstick  %d\n",(int)arg,fork_right);

		/*Simulate random time to represent eating of food*/
		i=rand()%50000;
		while(--i>0);
		printf("Philosopher #%d eating with chopstick %lu and %lu\n",(int)arg,fork_left,fork_right);

		/*This yield is not mandatory. But by adding this,
		 * we can generate more non-uniformity in the behavior.
		 * Also, in case the time multiplexing is too spaced out, by yielding,
		 * we allow others who have available resources to continue*/
		gtthread_yield();

		gtthread_mutex_unlock(right);
		printf("Philosopher #%d released chopstick %d\n",(int)arg,fork_right);
		gtthread_mutex_unlock(left);
		printf("Philosopher #%d released chopstick %d\n",(int)arg,fork_left);

		printf("Philosopher #%d thinking\n",(int)arg);

		i=rand()%100000;
		while(--i>0);

		gtthread_yield();
	}while(1);
}

int main(){

	gtthread_init(100);

	gtthread_mutex_init(&Res1);
	gtthread_mutex_init(&Res2);
	gtthread_mutex_init(&Res3);
	gtthread_mutex_init(&Res4);
	gtthread_mutex_init(&Res5);
	gtthread_create(&t1, philosopher, (void*)1);
	gtthread_create(&t2, philosopher, (void*)2);
	gtthread_create(&t3, philosopher, (void*)3);
	gtthread_create(&t4, philosopher, (void*)4);
	gtthread_create(&t5, philosopher, (void*)5);

	while(1);

}
