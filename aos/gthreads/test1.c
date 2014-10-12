/* Andrew Wilder */

/* test1a: 5 pts
 *   Tests thread creation.
 *   Create a thread that prints "Hello World!" and exits by returning. Main
 *   loops forever.
 *
 * test1b: 3 pts
 *   Tests thread argument passing.
 *   Create a thread that prints the argument passed to it and exits by
 *   returning. Main loops forever.
 *
 * test1c: 3 pts
 *   Tests thread join and return value.
 *   Create a thread that returns a string. Main joins the thread and prints the
 *   returned value.
 */

#ifdef QM
#include "gtthread.h"
#else
#include <gtthread.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *str = "Hello World! (b)";

void *func1(void *arg) {
	printf("Hello World! (a)\n");
	return NULL;
}

void *func2(void *arg) {
	char *s = (char*) arg;
	printf("%s\n", s);
	return NULL;
}

void *func3(void *arg) {
	return(strdup("Hello World! (c)"));
}

int main() {
	gtthread_t t1, t2, t3;
	gtthread_init(1000);

	gtthread_create(&t1, func1, NULL);
	gtthread_create(&t2, func2, str);
	gtthread_create(&t3, func3, NULL);

	void *msg;
	gtthread_join(t3, &msg);
	printf("%s\n", (char*) msg);
	free(msg);

	while(1);
	return 0;
}
