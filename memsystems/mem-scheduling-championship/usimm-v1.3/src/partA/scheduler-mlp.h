#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

// this vaiable counts the number of commands issued during write mode.
//int num_command_in_draining;
// priority array pointer: one entry per thread.
int *priorities;
// contains each core's MLP value.
//int *MLPs;
// the most recent request's row, bank, and bank.
long long int *current_row;
int *current_bank;
int *current_rank;
// the pointer to the array containing the number of read requests for 
// each thread in the read queue (used for MLP calculation).
int *num_read_request;
// array that keeps track of number of time each thread has low mlp.
int *low_mlp; 
// array that keeps track of number of time each thread has medium mlp.
int *medium_mlp;
// array that keeps track of number of time each thread has high mlp.
int *high_mlp;

// variables needed to consider aging in scheduler.
long long int aging_threshold;
long long int *last_service_time;




void calculate_MLP();// calculates Memory Level Parallism (MLP) for each thread in the read queue.
void update_priorities();//called every cycle to evaluate thread priorities in the read queue.

void init_scheduler_vars(); //called from main
void schedule(int); // scheduler function called every cycle
void scheduler_stats();
#endif //__SCHEDULER_H__

