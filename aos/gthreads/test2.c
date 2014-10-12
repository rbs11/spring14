/* Andrew Wilder */

/* test2a: 5 pts
 * Tests thread termination with gthread_exit.
 *   Create a thread that prints "thr1 exiting..." and exits by calling
 *   gthread_exit(NULL). Main loops forever.
 *
 * test2b: 3 pts
 *   Tests thread join and retrun value with gthread_exit.
 *   Create a thread that exits by calling gthread_exit with a string return
 *   value. Main joins the thread and prints the returned value.
 *
 * test2c: 2 pts
 *   Tests main calling gthread_exit.
 *   Create a thread that loops forever. Main prints "main exiting..." and exits
 *   by calling gthread_exit(NULL).
 */

#ifdef QM
#include "gtthread.h"
#else
#include <gtthread.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *func1(void *arg) {
	printf("thr1 exiting...\n");
	return NULL;
}

void *func2(void *arg) {
	return strdup("thr2\'s join message");
}

void *func3(void *arg) {
	while(1) ;
	return NULL;
}

int main() {
	gtthread_t t1, t2, t3;
	gtthread_init(1000);

	gtthread_create(&t1, func1, NULL);
	gtthread_create(&t2, func2, NULL);
	gtthread_create(&t3, func3, NULL);

	void *msg;
	gtthread_join(t2, &msg);
	printf("%s\n", (char*) msg);
	free(msg);

	printf("main thread exiting...\n");
	gtthread_exit(NULL);
	return 0;
}
