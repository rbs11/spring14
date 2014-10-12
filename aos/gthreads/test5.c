/* Andrew Wilder */

/* test5a: 3 pts
 * Tests mutexes.
 *   Initialize a mutex, lock the mutex.  Create two threads which immediately
 *   attempt to lock and unlock the mutex, print something and exit by returning.
 *   main prints something, unlocks the mutex and joins the threads.  main should
 *   continue before the threads exit.
 *
 * test5b: 1 pts
 *   Tests thread interleaving with mutexes.
 *   Initialize a mutex, lock the mutex.  Create two threads which immediately
 *   attempt to lock and unlock the mutex, then repeatedly busy wait, lock the
 *   mutex, print something, unlock, and exit by returning.  main prints
 *   something, unlocks the mutex and joins the threads. 
 *
 * test5c: 1 pts
 *   Tests mutexes.
 *   Like test5a but with one thread.
 */

#ifdef QM
#include "gtthread.h"
#else
#include <gtthread.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	gtthread_mutex_t *gttm;
	int num;
} gtt_tuple;

char buffer[50];

void *func1(void *arg) {
	gtt_tuple *tuple = (gtt_tuple*) arg;
	gtthread_mutex_t *gttm = tuple->gttm;
	gtthread_mutex_lock(gttm);
	gtthread_mutex_unlock(gttm);
	printf("thr%d exiting...\n", tuple->num);
	return NULL;
}

void *func2(void *arg) {
	gtt_tuple *tuple = (gtt_tuple*) arg;
	gtthread_mutex_t *gttm = tuple->gttm;
	volatile int x, y;
	for(int i = 6; i; --i) {
		gtthread_mutex_lock(gttm);
		char *dst = buffer, *src = "\tPrinting locked message from thread ";
		while((*dst++ = *src++)) {
			y = 1000000; // slow copy to test mutex
			while(--y);
		}
		printf("%s%d\n", buffer, tuple->num);
		memset(buffer, ' ', 50);
		gtthread_mutex_unlock(gttm);
		x = 10000000;
		while(--x);
	}
	return NULL;
}

int main() {
	gtthread_t t1, t2, t3, t4, t5;
	gtthread_mutex_t gttm;
	gtthread_init(1000);
	gtthread_mutex_init(&gttm);

	gtt_tuple tup1 = (gtt_tuple) {
		.gttm = &gttm,
		.num = 1
	}, tup2 = (gtt_tuple) {
		.gttm = &gttm,
		.num = 2
	};
	gtthread_mutex_lock(&gttm);
	gtthread_create(&t1, func1, &tup1);
	gtthread_create(&t2, func1, &tup2);
	volatile int x = 10000000;
	while(--x);
	gtthread_yield();
	printf("This should print before threads 1 and 2 finish\n");
	gtthread_mutex_unlock(&gttm);
	gtthread_join(t1, NULL);
	gtthread_join(t2, NULL);

	tup1.num = 3;
	tup2.num = 4;
	gtthread_mutex_lock(&gttm);
	gtthread_create(&t3, func2, &tup1);
	gtthread_create(&t4, func2, &tup2);
	printf("This should print before threads 3 and 4 finish\n");
	gtthread_mutex_unlock(&gttm);
	gtthread_join(t3, NULL);
	gtthread_join(t4, NULL);

	tup1.num = 5;
	gtthread_mutex_lock(&gttm);
	gtthread_create(&t5, func2, &tup1);
	printf("This should print before thread 5 finishes\n");
	gtthread_mutex_unlock(&gttm);
	gtthread_join(t5, NULL);

	return 0;
}
