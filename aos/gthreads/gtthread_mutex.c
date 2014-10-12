#include "gtthread.h"


int  gtthread_mutex_init(gtthread_mutex_t *mutex){
	mutex->lock = 0;
	return 0;
}

int  gtthread_mutex_lock(gtthread_mutex_t *mutex){
	while (__sync_lock_test_and_set(&mutex->lock, 1)){
		/*Yield to other thread*/
		gtthread_yield();
#ifdef DEBUG
		printf("Mutex %x ", mutex);
		printf("Spinning\n");
#endif
	}
#ifdef DEBUG
	printf("Mutex Acquired \n");
#endif
	return 0;
}

int  gtthread_mutex_unlock(gtthread_mutex_t *mutex){
	__sync_lock_release(&mutex->lock);
	return 0;
}
