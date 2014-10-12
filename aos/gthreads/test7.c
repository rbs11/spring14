/* Andrew Wilder */

/* test7: 2 pts
 * Tests an explicitly created thread creating another thread.
 *   Create a thread which immediately creates another instance of itself (with a
 *   NULL parameter so it doesn't continue creating new copies). Each copy loops
 *   10 times, printing "Hi: X" and busy waiting. The statements should interleave
 *   in some rough fashion.
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

void *func1(void *arg) {
	long x = (long) arg;
	if(x - 1) {
		gtthread_t gt;
		gtthread_create(&gt, func1, (void*) (x - 1));
	}
	volatile int y;
	for(int i = 1; i <= 10; ++i) {
		y = 10000000;
		while(--y);
		gtthread_mutex_lock(&gttm); // smooth printing
		for(int j = x; j; --j) {
			printf("\t");
		}
		printf("thr%ld: %d\n", x, i);
		gtthread_mutex_unlock(&gttm);
	};
	return NULL;
}

int main() {
	gtthread_t t1;

	gtthread_init(1000);
	gtthread_mutex_init(&gttm);
	gtthread_create(&t1, func1, (void*) 4);

	gtthread_exit(NULL);
	return 0;
}
