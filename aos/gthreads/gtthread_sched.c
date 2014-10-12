#include "gtthread.h"
/* Just an example. Nothing to see here. */
struct itimerval quantum;
struct sigaction preempt_action;
int sigtype = ITIMER_VIRTUAL;//ITIMER_REAL ITIMER_VIRTUAL ITIMER_PROF
int signum = SIGVTALRM;//SIGALRM SIGPROF SIGVTALRM

//#define DEBUG
void init_timer( long period ){

	quantum.it_value.tv_sec = 0;
	quantum.it_value.tv_usec = (long) period;
	quantum.it_interval = quantum.it_value;

}

void sighandler( int sig_nr, siginfo_t* info, void *old_context ) {
#ifdef DEBUG
	printf("Signal received %d\n", sig_nr);
#endif
	setitimer( sigtype , &quantum, NULL );
	if ( sig_nr == signum)//SIGALRM SIGPROF SIGVTALRM
	{
#ifdef DEBUG
		if (curr_thread == 0)
			printf("  Swapping to Sched from main  \n");
		else
			printf(" Saving current context of thread[%d]", curr_thread);
#endif
		swapcontext( &mythread[curr_thread].user_context, &ctx_sched );
	}
	else return;
}

void init_sigtimer(){
	if( setitimer( sigtype , &quantum, NULL ) == 0) {//ITIMER_REAL ITIMER_VIRTUAL ITIMER_PROF
#ifdef DEBUG
		printf("\n\tTimer initialized  \n");
#endif
	}

	preempt_action.sa_sigaction = sighandler;
	preempt_action.sa_flags = SA_SIGINFO;
	sigemptyset( &preempt_action.sa_mask );

	if ( sigaction( signum, &preempt_action, NULL ) == -1 ) {//SIGALRM SIGPROF SIGVTALRM
#ifdef DEBUG
		printf("\nTimer initialization Error\n");
#endif
	}
	return;
}

void scheduler() {

	while(1){
		if( nTotalThreads > 0 ){
			atomic_increment( &curr_thread );
			if (curr_thread >= nTotalThreads){
#ifdef ARCH_SUPPORT
				atomic_modulus( &curr_thread, &nTotalThreads );
#else
				curr_thread = curr_thread % nTotalThreads;
#endif
			}
		}

#ifdef DEBUG
		printf("\t****Scheduler*****\n");
		printf("\tactiveThreads %d\n", nActiveThreads);
		printf("\tTotalThreads %d\n", nTotalThreads);
		printf("\tcurrent thread %d\n", curr_thread);
#endif
		if((!mythread[curr_thread].exited)){
#ifdef DEBUG
				printf("----Swapping to Thread[%d]\n", curr_thread);
#endif
			swapcontext( &ctx_sched, &mythread[curr_thread].user_context );
		}
		else
		{
#ifdef DEBUG
			printf("Thread[%d] Exited\n", curr_thread);
#endif
		}
		if (nActiveThreads == 0)
			break;
	}
#ifdef DEBUG
	printf("All threads finished\n");
#endif
}
