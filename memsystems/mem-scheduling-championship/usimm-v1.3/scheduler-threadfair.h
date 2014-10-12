#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

// struct containing requests to the same row [REMOVED!!!]
/*typedef struct row_entry {
	int rank;
	int bank;
	long long int row;
	int request_num;
	void * row_entry_head;
} same_row_entry_t;

#define MAX_RE_NUM 16

// the pointer to the array containing the number of read requests for
// each thread in the read queue.
int * committed_read_request;

// the pointer to the array containing the thread id
// the more close to the head, the higher priority

int * thread_priority_head;*/ //[REMOVED!!!]

void init_scheduler_vars(); //called from main
void scheduler_stats(); //called from main
void schedule(int); // scheduler function called every cycle

#endif //__SCHEDULER_H__
