#ifndef __GTTHREAD_H
#define __GTTHREAD_H

#include <ucontext.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#define gtthread_t unsigned long int
#define STACKSIZE 1024*8
#define MAXTHREADS 200
typedef int bool;
enum { false, true };
/* Must be called before any of the below functions. Failure to do so may
 * result in undefined behavior. 'period' is the scheduling quantum (interval)
 * in microseconds (i.e., 1/1000000 sec.). */
void gtthread_init(long period);

/* see man pthread_create(3); the attr parameter is omitted, and this should
 * behave as if attr was NULL (i.e., default attributes) */
int  gtthread_create(gtthread_t *thread,
                     void *(*start_routine)(void *),
                     void *arg);

/* see man pthread_join(3)*/
int  gtthread_join(gtthread_t thread, void **status);

 /*gtthread_detach() does not need to be implemented; all threads should be
 * joinable

 see man pthread_exit(3)*/
void gtthread_exit(void *retval);

 /*see man sched_yield(2)*/
void gtthread_yield(void);

/* see man pthread_equal(3)*/
int  gtthread_equal(gtthread_t t1, gtthread_t t2);

/* see man pthread_cancel(3); but deferred cancelation does not need to be
 * implemented; all threads are canceled immediately*/
int  gtthread_cancel(gtthread_t thread);

/* see man pthread_self(3)*/
gtthread_t gtthread_self(void);

void atomic_increment( volatile int *operand );
void atomic_decrement( volatile int *operand );
void atomic_modulus( int *operand, int *divisor );
/* see man pthread_mutex(3); except init does not have the mutexattr parameter,
 * and should behave as if mutexattr is NULL (i.e., default attributes); also,
 * static initializers do not need to be implemented*/


typedef struct gtthread{
	volatile bool active;
	volatile bool done;
	volatile bool exited;
	ucontext_t user_context;
	gtthread_t threadid;
	void *ret;
}gtthread;

extern struct sigaction preempt_action;
extern gtthread mythread[MAXTHREADS];
extern volatile int curr_thread;
extern volatile int nTotalThreads;
extern volatile int nActiveThreads;
extern int sigtype;
extern int signum;
extern ucontext_t ctx_sched;
extern ucontext_t main_context;

extern void scheduler();
extern void threadcore( void *func(), void *arg );
extern void generate_thread_id( gtthread_t *thread );

extern void init_timer( long period );
extern void init_sigtimer();
extern void init_sched_preempt_handler();
extern void sighandler( int sig_nr, siginfo_t* info, void *old_context );

typedef struct gtthread_mutex_t{
	int8_t lock;
}gtthread_mutex_t;

extern int  gtthread_mutex_init(gtthread_mutex_t *mutex);
extern int  gtthread_mutex_lock(gtthread_mutex_t *mutex);
extern int  gtthread_mutex_unlock(gtthread_mutex_t *mutex);


#endif // __GTTHREAD_H
