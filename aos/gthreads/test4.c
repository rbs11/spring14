/* Andrew Wilder */

/* test4a: 1 pts
 * Tests gtthread_equal.
 *   Create a thread and store the descriptor in a global. Pass the thread's
 *   descriptor to a function. The gtthread_equal() function should say the
 *   global descriptor and the function parameter descriptor are equal.
 *
 * test4b: 1 pts
 *   Tests gtthread_equal.
 *   Create two threads. The gtthread_equal() function should say the second
 *   thread's descriptor is not equal to the first thread's descriptor.
 *
 * test4c: 1 pts
 *   Tests gtthread_self and gtthread_equal.
 *   Create a thread and store the descriptor in a global. The gtthread_equal()
 *   function should say the global descriptor and the descriptor returned by
 *   gtthread_self() are equal.
 *
 * test4d: 1 pts
 *   Tests gtthread_self and gtthread_equal.
 *   Create two threads. The gtthread_equal() function should say the second
 *   thread's descriptor is not equal to the descriptor returned by
 *   gtthread_self() in the first thread.
 *
 * test4e: 3 pts
 *   Tests a join long after a thread has terminated.
 *   Create a thread which immediately returns a static string. Busy wait in
 *   main, so that the thread has time to finish, and then join the thread and
 *   check the return value.
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
	gtthread_t gtt1;
	gtthread_t gtt2;
} gtt_tuple;

gtthread_t g_gtt;

static const char *str = "thr5 joined properly";

void *func1(void *arg) {
	gtthread_t *gtt = (gtthread_t*) arg;
	gtthread_yield(); // ensure that the global is set first
	gtthread_yield();
	gtthread_yield();
	int res = gtthread_equal(g_gtt, *gtt);
	printf("First test is equal:      %s\n", res ? "correct" : "incorrect");
	return NULL;
}

void *func2(void *arg) {
	gtt_tuple *tuple = (gtt_tuple*) arg;
	gtthread_yield(); // ensure that the tuple is set first
	gtthread_yield();
	gtthread_yield();
	int res = gtthread_equal(tuple->gtt1, tuple->gtt2);
	printf("Second test is not equal: %s\n", !res ? "correct" : "incorrect");
	return NULL;
}

void *func3(void *arg) {
	gtthread_t gtt = gtthread_self();
	gtthread_yield(); // ensure that the global is set first
	gtthread_yield();
	gtthread_yield();
	int res = gtthread_equal(g_gtt, gtt);
	printf("Third test is equal:      %s\n", res ? "correct" : "incorrect");
	gtthread_t *ret = malloc(sizeof(gtt));
	memcpy(ret, &gtt, sizeof(gtt));
	return ret;
}

void *func4(void *arg) {
	gtthread_t *t3_self = (gtthread_t*) arg;
	gtthread_t gtt = gtthread_self();
	int res = gtthread_equal(gtt, *t3_self);
	printf("Fourth test is not equal: %s\n", !res ? "correct" : "incorrect");
	return NULL;
}

void *func5(void *arg) {
	return (void*) str;
}

int main() {
	gtthread_t t1, t2, t3, t4, t5;
	gtthread_init(1000);

	gtthread_create(&t1, func1, &t1);
	g_gtt = t1;
	gtthread_join(t1, NULL);

	gtt_tuple tuple;
	gtthread_create(&t2, func2, &tuple);
	tuple = (gtt_tuple) {
		.gtt1 = t1,
		.gtt2 = t2
	};
	gtthread_join(t2, NULL);

	gtthread_create(&t3, func3, NULL);
	g_gtt = t3;
	void *t3_self;
	gtthread_join(t3, &t3_self);

	gtthread_create(&t4, func4, t3_self);
	gtthread_join(t4, NULL);
	free(t3_self);

	gtthread_create(&t5, func5, NULL);
	volatile int x = 100000000;
	while(--x);
	void *msg;
	gtthread_join(t5, &msg);
	printf("%s\n", (char*) msg);

	return 0;
}
