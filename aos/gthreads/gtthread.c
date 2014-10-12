#include "gtthread.h"
/* Just an example. Nothing to see here. */


gtthread mythread[MAXTHREADS];

static int initialized = 0;
//#define DEBUG
volatile int curr_thread = 0;
volatile int nTotalThreads = 0;
volatile int tail_index = 1;
volatile int nActiveThreads = 0;
ucontext_t ctx_sched;
ucontext_t main_context;
int allthreadsfinished = 0;

void gtthread_init( long period )
{
	int i;

	for( i=0; i<MAXTHREADS; i++ )
	{
		mythread[i].active = 0;
		mythread[i].done = 0;
		mythread[i].exited = 0;
		mythread[i].threadid = -1;
	}

	if( !getcontext(&ctx_sched) )/*Create Scheduler context*/
	{
		ctx_sched.uc_stack.ss_sp = malloc( STACKSIZE );
		ctx_sched.uc_stack.ss_size = STACKSIZE;
		ctx_sched.uc_stack.ss_flags = 0;
		ctx_sched.uc_link = NULL;
		makecontext( &ctx_sched, (void(*)(void)) scheduler, 0 );
	}

	if (!getcontext( &mythread[0].user_context ))/*First element will be the main thread*/
	{
		mythread[0].threadid = 0;
	}


	init_timer( period );
	init_sigtimer();
	atomic_increment( &nTotalThreads );
	atomic_increment( &nActiveThreads );
	initialized = 1;
	return;
}

void generate_thread_id( gtthread_t *thread ){
	int i;
	static bool unique = 0;
	do{
		*thread = rand(); //Generate new thread ID and assign.
		for( i=1; i<MAXTHREADS; i++ ){
			if( gtthread_equal(mythread[i].threadid, *thread ) && (*thread == 0) ){
				break;
			}
			else unique = true;
		}
	}while(!unique);
	return;
}

sigset_t setz;
int gtthread_create( gtthread_t *thread, void *(*start_routine)(void *), void *arg ){

	if( !initialized ) {
		printf("Error: gtthread_init() not called!  Exiting.");
		exit(-1);
	}
	if( nTotalThreads >= MAXTHREADS )
	{
		printf("\n!!!!Number of threads have exceeded the maximum supported threads!!!!\n");
		return -1;
	}

	generate_thread_id( thread );
	getcontext( &mythread[tail_index].user_context );
	mythread[tail_index].threadid = *thread;
	mythread[tail_index].user_context.uc_link = &ctx_sched;
	mythread[tail_index].user_context.uc_stack.ss_sp = malloc(STACKSIZE);
	mythread[tail_index].user_context.uc_stack.ss_size = STACKSIZE;
	mythread[tail_index].user_context.uc_stack.ss_flags = 0;

	if( mythread[tail_index].user_context.uc_stack.ss_sp == 0 ) {
		printf( "Error: Could not allocate stack.");
		return -1;
	}

#ifdef DEBUG
	printf("\nMAKE THREAD user_context:\n\t Allocating stack for thread %ld with user_context %ld\n", tail_index, mythread[tail_index].user_context.uc_stack.ss_sp );
#endif

	makecontext( &mythread[tail_index].user_context, (void(*)(void)) threadcore, 2, start_routine, arg );

	atomic_increment( &tail_index );
	atomic_increment( &nTotalThreads );
	atomic_increment( &nActiveThreads );
	return 0;
}

int gtthread_join( gtthread_t thread, void **status )
{
	int i;
	bool flag = 0;

	/*Find the thread which is requested*/
	for( i=1; i < nTotalThreads; i++ ){
		if(mythread[i].threadid == thread) {
			flag = 1;
#ifdef DEBUG
			printf("Joined Thread[%d] ID[%d] ",i, thread);
#endif
			break;
		}
	}
	if (flag == 0)	{
		printf("Thread to join not found. Already exited or not created\n");
		return -1;
	}

	/*Wait till thread completes execution*/
	while(!mythread[i].exited){
#ifdef DEBUG
		printf("---->Waiting for thread[%d] Curr[%d]\n",i, curr_thread);
#endif
		gtthread_yield();
	}

	if (status)
		*status = (void *)mythread[i].ret;

	/*We free resources only when a join is called to ensure that the thread information
	 * is saved in case of future call to join. */
	if ((mythread[i].user_context.uc_stack.ss_sp))
		free(mythread[i].user_context.uc_stack.ss_sp);

#ifdef DEBUG
	printf("****JOIN*****:\nThread %d exited %d finished %d\n", i, mythread[i].exited, mythread[i].done);
#endif


return 0;
}

void gtthread_exit( void *retval ){
#ifdef DEBUG
	printf("****EXIT*****:\nThread %d finished %d\n", curr_thread, mythread[curr_thread].done);
	if (curr_thread == 0)
			printf("Main Thread going to exit!!\n");
#endif
	mythread[curr_thread].ret = retval;
	mythread[curr_thread].exited = 1;
	mythread[curr_thread].active = 0;
	if (!mythread[curr_thread].done){
#ifdef DEBUG
		printf("****Exiting Explicit[%d]\n", curr_thread );
#endif
		mythread[curr_thread].done = 1;
	}
	atomic_decrement( &nActiveThreads );


	swapcontext( &mythread[curr_thread].user_context, &ctx_sched );

	return;
}

int gtthread_cancel( gtthread_t thread ){
	ucontext_t dummyctx;
	int i, found=0;
	for( i=1; i < nTotalThreads; i++ ) {
		if( gtthread_equal( mythread[i].threadid, thread ) && !mythread[i].exited ){
			mythread[i].exited = 1;
			mythread[i].active = 0;
			atomic_decrement( &nActiveThreads );
			found = 1;
#ifdef DEBUG
			printf("****CANCEL*****:\nActive thread count decremented to %d\n", nActiveThreads);
			printf("\tThread %d cancelled\n", i);
#endif
			swapcontext( &dummyctx, &ctx_sched );
		}
	}
	if (!found)
		printf("----Thread [%ld] to Cancel not found---- \n", thread);
	return 0;
}

void gtthread_yield( void ){
#ifdef DEBUG
	if (curr_thread == 0)
		printf("+++++Swapping yield to sched from main\n");
#endif

	swapcontext( &mythread[curr_thread].user_context, &ctx_sched);
	return;
}

int gtthread_equal( gtthread_t t1, gtthread_t t2 ){
	if( t1 == t2 ) return 1;
	else return 0;
}


void threadcore( void *func(), void *arg ) {
#ifdef DEBUG
	printf("*****threadcore*****\nActiveThreads %d\n", nActiveThreads);
	printf("Thread[%d] Function Complete\n", curr_thread);
#endif
	mythread[curr_thread].active = 1;
	mythread[curr_thread].ret = func(arg);
	mythread[curr_thread].done = 1;
	if(!mythread[curr_thread].exited){
		mythread[curr_thread].active = 0;
		gtthread_exit(mythread[curr_thread].ret);//Required for cleanup
	}
	else{
		ucontext_t dummyctx;
#ifdef DEBUG
		printf("\tthreadcore swapping to scheduler\n");
#endif
		swapcontext( &dummyctx, &ctx_sched );
	}
	printf("---This should never be happen because context is lost at exit\n");
	setcontext( &ctx_sched );
	return;
}

gtthread_t gtthread_self( void ){
	return mythread[curr_thread].threadid; //threadID
}

void atomic_decrement(volatile int *operand ){
#ifdef ARCH_SUPPORT
	__asm__ __volatile__(
							"mov %1, %%eax\n\t"
							"sub $0x01, %%eax\n\t"
							"xchg %%eax, %0\n\t"
							:"=r" (*operand)
							:"r" (*operand)
							:"%eax"
						);
#else
	(*operand)--;
#endif
	return;
}

void atomic_increment(volatile int *operand ){
#ifdef ARCH_SUPPORT
	__asm__ __volatile__(
							"mov %1, %%eax\n\t"
							"add $0x01, %%eax\n\t"
							"xchg %%eax, %0\n\t"
							:"=r" (*operand)
							:"r" (*operand)
							:"%eax"
						);
#else
	(*operand)++;
#endif
	return;

}
void atomic_modulus( int *operand, int *divisor ){

	__asm__ __volatile__(
							"mov %1, %%eax\n\t"
							"xor %%edx, %%edx\n\t"
							"mov %2, %%ecx\n\t"
							"idiv %%ecx\n\t"
							"mov %%edx, %0\n\t"
							:"=r"(*operand)
							:"r"(*operand), "r"(*divisor)
							:"%eax", "%ecx", "%edx"
						);
}
