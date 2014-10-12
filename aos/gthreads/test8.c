/* Andrew Wilder */

/* test8a: 1 pts
 * Tests gtthread_cancel.
 *   Create a thread which prints "thr1 hi" forever.  main cancels the thread and
 *   calls gtthread_exit.  The program should stop.
 *
 * test8b: 1 pts
 *   Tests a different scheduling period.
 *   Same as test8a, except the thread is not canceled and the scheduling period
 *   is modified.
 */

#ifdef QM
#include "gtthread.h"
#else
#include <gtthread.h>
#endif

#include <stdio.h>

void *func1(void *arg) {
	volatile int x;
	while(1) {
		printf("thr1 hi\n");
		x = 10000000;
		while(--x);
	}
	return NULL;
}

int main() {
	gtthread_t t1;
#ifdef B
	gtthread_init(500);
#else
	gtthread_init(1000);
#endif

	gtthread_create(&t1, func1, NULL);
#ifndef B
	volatile int x = 100000000;
	while(--x);
	gtthread_cancel(t1);
#endif

	gtthread_exit(NULL);
	return 0;
}
