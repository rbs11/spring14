/* Andrew Wilder */

/* test6a: 2 pts
 * Tests thread interleaving with yield.
 *   Create two threads, each of which yield then print a message.  main joins
 *   both threads and exits.  The messages should interleave.
 *
 * test6b: 1 pts
 *   Tests thread interleaving with yield and mutex locks.
 *   Like test5b except no busy waiting and yield after each lock, print, unlock
 *   sequence.
 *
 * test6c: 1 pts
 *   Tests yield with one schedulable thread.
 *   Like test6a but with one thread.
 *
 * test6d: 3 pts
 *   Tests mutexes.
 *   Initialize a mutex, lock the mutex.  Create a thread which immediately
 *   attempts to lock the mutex and print something.  main joins the thread
 *   without unlocking the mutex.  The program should either hang or exit after
 *   detecting deadlock (but neither thread should proceed).
 */

#ifdef QM
#include "gtthread.h"
#else
#include <gtthread.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

gtthread_mutex_t gttm;
char buffer[50];

void *func1(void *arg) {
	long x = (long) arg;
	for(int i = 5; i; --i) {
		gtthread_yield();
		printf("Message from thr%ld\n", x);
	}
	return NULL;
}

void *func2(void *arg) {
	long x = (long) arg;
	volatile int y;
	for(int i = 5; i; --i) {
		gtthread_mutex_lock(&gttm);
		gtthread_yield();
		char *dst = buffer, *src = "\tThis is a locked message from thread ";
		while((*dst++ = *src++)) {
			y = 5000000;
			while(--y);
		}
		printf("%s%ld\n", buffer, x);
		memset(buffer, ' ', 50);
		gtthread_mutex_unlock(&gttm);
	}
	return NULL;
}

void *func3(void *arg) {
	gtthread_mutex_lock(&gttm);
	printf("Printing from thread 5... which shouldn\'t happen\n");
	gtthread_mutex_unlock(&gttm);
	return NULL;
}

int main() {
	gtthread_t t1, t2, t3, t4, t5;

	gtthread_init(1000);
	gtthread_mutex_init(&gttm);

	gtthread_create(&t1, func1, (void*) 1);
	gtthread_create(&t2, func1, (void*) 2);
	gtthread_join(t1, NULL);
	gtthread_join(t2, NULL);

	gtthread_mutex_lock(&gttm);
	gtthread_create(&t3, func2, (void*) 3);
	gtthread_create(&t4, func2, (void*) 4);
	printf("This should print before threads 3 and 4 finish\n");
	gtthread_mutex_unlock(&gttm);
	gtthread_join(t3, NULL);
	gtthread_join(t4, NULL);

	printf("Testing yield with one schedulable thread... ");
	gtthread_yield();
	gtthread_yield();
	gtthread_yield();
	printf("done!\n");

	printf("Deadlocking with thread 5...\n");
	gtthread_mutex_lock(&gttm);
	gtthread_create(&t5, func3, NULL);
	gtthread_join(t5, NULL);
	gtthread_mutex_unlock(&gttm);
	printf("!!! ERROR !!! Failed to deadlock with thread 5\n");

	return 0;
}
