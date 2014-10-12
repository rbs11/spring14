
/*
This file contains the implementation of a simple Memory Level Pralelism (MLP) aware
scheduling. The algorith treats read and write request in different ways.
Here, we review main points in this scheduling.
1) Read to Write & Write to Read switching: In two cases scheduler switches to write mode:
(i) the number of requests in write queue exceeded high watermark, (ii) there is no request on the read queue.
In case(i), the mode will changed to read mode again when the number of request in write queue shrinks to low
watermark. In case (ii), the mode will change to read mode, when scheduler finished issue at least up to commands (founded by experience).
or there is no more write request.

2) Read scheduling policy: The main idea beind this scheduler is to pay more attention to threads showing
less MLP, which is a good sign of being in "computing mode". In addition, scheduler support fairness by
considering the age of request in the read queue. This is realized by increasing every thread's priority
that wait more that some specific preiod (aging_threshold).

3) Write scheduling policy: simple fcfs.
 
*/

#include <stdio.h>
#include "utlist.h"
#include "utils.h"
#include <stdlib.h>
#include <assert.h>

#include "params.h"
#include "scheduler.h"
#include "memory_controller.h"
#define HI_WM 40
#define LO_WM 20

extern long long int CYCLE_VAL;


//find the first available request that leads to acheive at least row_hit number of row hit.
request_t* find_wr_request(int channel, int row_hit)
{
	int counter=0;
	request_t *wr_ptr1=NULL;
	request_t *wr_ptr2=NULL;
	LL_FOREACH(write_queue_head[channel], wr_ptr1)
	{
		LL_FOREACH(write_queue_head[channel], wr_ptr2)
		{
			if( (wr_ptr1->dram_addr.rank==wr_ptr2->dram_addr.rank)
			  & (wr_ptr1->dram_addr.bank==wr_ptr2->dram_addr.bank)	
              & (wr_ptr1->dram_addr.row ==wr_ptr2->dram_addr.row) )	
				counter++;	
		}
		if(counter>=row_hit)
			return wr_ptr1;
	}
	return NULL;
}

// for every thread calculate the current MLP by counting 
// the number of non-completed requests in the read queue.
void calculate_MLP()
{
	int i;
	request_t * rd_ptr = NULL;
	for(int i=0;i<NUMCORES;i++)
	{
		num_read_request[i]=0;
	}
	//printf("%lld --- ",CYCLE_VAL);
	for(i=0;i<NUM_CHANNELS;i++)
	{
		LL_FOREACH(read_queue_head[i],rd_ptr)
		{
			if(rd_ptr->completion_time < CYCLE_VAL)
			{
				num_read_request[rd_ptr->thread_id]++;	
			}
		}
	}
}


// first: assigns priority of every thread based on its current MLP.
// second: increase priority of threads that wait for more than aging_threshold cycle.
// The increase in threads' priority is leanier to its waiting time.
void update_priorities()
{
	int i;
	calculate_MLP();
	for(i=0;i<NUMCORES;i++)	
	{
		if(num_read_request[i]< 1)
        {
			priorities[i]=3;
            low_mlp[i]++;
        }
		else if(num_read_request[i]< 3)
        {
			priorities[i]=2;
            medium_mlp[i]++;
        }
		else
        {
			priorities[i]=1;
            high_mlp[i]++;
        }
	}
	for(i=0;i<NUMCORES;i++)	
	{
		priorities[i]+= (CYCLE_VAL-last_service_time[i])/aging_threshold;
	}
}
// initializing auxilary variables defined in scheduler-mlp.h.
void init_scheduler_vars()
{
	int i;
	// initialize all scheduler variables here
	priorities=(int*) malloc(sizeof(int)*NUMCORES);
    low_mlp=(int*) malloc(sizeof(int)*NUMCORES);
    medium_mlp=(int*) malloc(sizeof(int)*NUMCORES);
    high_mlp=(int*) malloc(sizeof(int)*NUMCORES);
	for(i=0;i<NUMCORES;++i)
    {
        low_mlp[i]=0;
        medium_mlp[i]=0;
        high_mlp[i]=0;
    }

	current_row = (long long int *) malloc(sizeof(long long int *)*NUM_CHANNELS);
	current_bank = (int *) malloc(sizeof(int *)*NUM_CHANNELS);
        current_rank = (int *) malloc(sizeof(int *)*NUM_CHANNELS);
	for(i=0;i<NUM_CHANNELS;i++)
	{
		current_row[i]=0;
		current_bank[i]=0;
		current_rank[i]=0;
	}

	aging_threshold=40;
	num_read_request=(int*) malloc(sizeof(int)*NUMCORES);
	last_service_time=(long long int*) malloc(sizeof(long long int)*NUMCORES);
	return;
}


int drain_writes[MAX_NUM_CHANNELS];

/* Each cycle it is possible to issue a valid command from the read or write queues
   OR
   a valid precharge command to any bank (issue_precharge_command())
   OR
   a valid precharge_all bank command to a rank (issue_all_bank_precharge_command())
   OR
   a power_down command (issue_powerdown_command()), programmed either for fast or slow exit mode
   OR
   a refresh command (issue_refresh_command())
   OR
   a power_up command (issue_powerup_command())
   OR
   an activate to a specific row (issue_activate_command()).

   If a COL-RD or COL-WR is picked for issue, the scheduler also has the
   option to issue an auto-precharge in this cycle (issue_autoprecharge()).

   Before issuing a command it is important to check if it is issuable. For the RD/WR queue resident commands, checking the "command_issuable" flag is necessary. To check if the other commands (mentioned above) can be issued, it is important to check one of the following functions: is_precharge_allowed, is_all_bank_precharge_allowed, is_powerdown_fast_allowed, is_powerdown_slow_allowed, is_powerup_allowed, is_refresh_allowed, is_autoprecharge_allowed, is_activate_allowed.
   */



void schedule(int channel)
{
	request_t * rd_ptr = NULL;
	request_t * wr_ptr = NULL;
	

	int i;
	int max=-1;
	request_t* rd_ptr_best=NULL;
	dram_address_t temp;

    update_priorities();
    
    // if in write drain mode, keep draining writes until the
	// write queue occupancy drops to LO_WM
	if (drain_writes[channel] && (write_queue_length[channel] > LO_WM)) {
        drain_writes[channel] = 1; // Keep draining.
	}
	else {
        drain_writes[channel] = 0; // No need to drain.
	}
    
	// initiate write drain if either the write queue occupancy
	// has reached the HI_WM , OR, if there are no pending read
	// requests
	if(write_queue_length[channel] > HI_WM)
	{
		drain_writes[channel] = 1;
	}
	else {
        if (!read_queue_length[channel])
            drain_writes[channel] = 1;
	}
    // in writing mode apply fcfs policy
	if(drain_writes[channel])
	{
		
           
        LL_FOREACH(write_queue_head[channel], wr_ptr)
        {
            if(wr_ptr->command_issuable)
            {
                issue_request_command(wr_ptr);
                break;
            }
        }
        return;    
	}

	if(!drain_writes[channel])
	{
		//update_priorities();
		
		// if there is still chance for more row hits, go ahead with the same row.
		LL_FOREACH(read_queue_head[channel],rd_ptr)
		{
			if(rd_ptr->command_issuable)
			{
				temp= rd_ptr->dram_addr;
				if( temp.rank ==  current_rank[channel] 
				 && temp.bank == current_bank[channel] 
				 && temp.row == current_row[channel] )
			 	{			
					issue_request_command(rd_ptr);
					return;
				}
			}
		}
		// if there is no more row hit, select based on the proirity;
		LL_FOREACH(read_queue_head[channel],rd_ptr)
		{
			if(rd_ptr->command_issuable)
			{
				if( priorities[rd_ptr->thread_id]>max )
				{
					max=priorities[rd_ptr->thread_id];
					rd_ptr_best=rd_ptr;
				}
			}
		}
		
		if(rd_ptr_best!=NULL)
		{
			current_rank[channel]=rd_ptr_best->dram_addr.rank;
			current_bank[channel]=rd_ptr_best->dram_addr.bank;
			current_row[channel]=rd_ptr_best->dram_addr.row;
			last_service_time[rd_ptr_best->thread_id]=CYCLE_VAL;
		}
		
		return;
	}
}


void scheduler_stats()
{

	int i;	
    float sum=0;
    printf ("\n\n---------------------------\n");
    printf ("id | low   meduim  high  \n");
    printf ("---------------------------\n");
    for(i=0;i<NUMCORES;++i)
    {
        sum=low_mlp[i]+medium_mlp[i]+high_mlp[i];
        printf( "%2d | %3.2f %3.2f   %3.2f \n" ,i, 100*low_mlp[i]/sum, 100*medium_mlp[i]/sum, 100*high_mlp[i]/sum);
    }
    printf ("---------------------------\n\n\n");
}


