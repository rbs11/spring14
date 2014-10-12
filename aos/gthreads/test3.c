/* Andrew Wilder */

/* test3a: 1 pts
 * Tests preemption and fair scheduling.  
 *   The thread busy waits, prints, and returns. Main does the same, but with a
 *   much longer busy wait. The thread should finish before main.
 *
 * test3b: 1 pts
 *   Tests preemption and fair scheduling.
 *   Like test3a except the busy wait lengths are swapped. Main should finish
 *   before the thread.
 *
 * test3c: 2 pts
 *   Tests interleaving between two spawned threads.  
 *   Main creates thr1 and thr2 which each loop five times: in the loop, each
 *   thread busy waits and then prints "thrX hello". Main loops forever. The two
 *   threads should alternate messages printed.
 *
 * test3d: 2 pts
 *   Tests an explicitly created thread joining another. 
 *   Create two threads.  Thread 1 busy waits for a long time and then exits,
 *   printing "thr1 exiting...". Thread 2 joins Thread 1 and prints "thr2
 *   exiting...". Main loops forever. Thread 1 should print its message before
 *   thread 2.
 */

#ifdef QM
#include "gtthread.h"
#else
#include <gtthread.h>
#endif

#include <stdio.h>

void *func1(void *arg) {
	volatile int x = 10000000;
	while(--x);
	printf("thr1 prints first...\n");
	return NULL;
}

void *func2(void *arg) {
	volatile int x = 100000000;
	while(--x);
	printf("...then thr2 prints\n");
	return NULL;
}

void *func3(void *arg) {
	char *s = (char*) arg;
	volatile int x;
	for(int i = 5; i; --i) {
		x = 10000000;
		while(--x);
		printf("%s\n", s);
	}
	return NULL;
}

void *func4(void *arg) {
	volatile int x = 100000000;
	while(--x);
	printf("thr5 exiting...\n");
	return NULL;
}

void *func5(void *arg) {
	gtthread_t *gtt = (gtthread_t*) arg;
	gtthread_join(*gtt, NULL);
	printf("thr6 exiting...\n");
	return NULL;
}

int main() {
	gtthread_t t1, t2, t3, t4, t5, t6;
	gtthread_init(1000);

	gtthread_create(&t1, func1, NULL);
	volatile int x = 100000000;
	while(--x);
	printf("...then main prints\n");
	gtthread_join(t1, NULL);

	gtthread_create(&t2, func2, NULL);
	x = 10000000;
	while(--x);
	printf("main prints first...\n");
	gtthread_join(t2, NULL);

	printf("Interleaved threads:\n");
	gtthread_create(&t3, func3, "\tthr3 printing");
	gtthread_create(&t4, func3, "\t\tthr4 printing");
	gtthread_join(t3, NULL);
	gtthread_join(t4, NULL);

	gtthread_create(&t5, func4, NULL);
	gtthread_create(&t6, func5, &t5);

	while(1);
	return 0;
}
