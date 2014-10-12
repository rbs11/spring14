#include <stdio.h>
#include "utlist.h"
#include "utils.h"

#include "memory_controller.h"
#include "params.h"

/* 
   A basic FCFS policy augmented to issue a refresh whenever the read queue and write
   queues are empty.
 */


extern long long int CYCLE_VAL;

/* A data structure to see if a bank is a candidate for precharge. */
int recent_colacc[MAX_NUM_CHANNELS][MAX_NUM_RANKS][MAX_NUM_BANKS];

int refreshes[MAX_NUM_CHANNELS][MAX_NUM_RANKS];

/* Keeping track of how many aggressive refreshes are performed. */
long long int num_aggr_refresh = 0;

/* Keeping track of when the last aggressive refresh was so we don't do it too often */
//long long int cycle_last_aggressive_refresh = 0;

void init_scheduler_vars()
{
	// initialize all scheduler variables here

	int i,j;
	for (i=0;i<MAX_NUM_CHANNELS;i++) {
	  for (j=0;j<MAX_NUM_RANKS;j++) {
	    refreshes[i][j]=0;
	  }
	}

	return;
}

// write queue high water mark; begin draining writes if write queue exceeds this value
#define HI_WM 40

// end write queue drain once write queue has this many writes in it
#define LO_WM 20

// 1 means we are in write-drain mode for that channel
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
  

  if ((CYCLE_VAL % (8*T_REFI)) == 0) {
    for (i=0;i<NUM_RANKS;i++) {
      refreshes[channel][i] = 0;
    }
  }

  // if in write drain mode, keep draining writes until the
  // write queue occupancy drops to LO_WM
  if (drain_writes[channel] && (write_queue_length[channel] > LO_WM)) 
    {
      drain_writes[channel] = 1; // Keep draining.
    }
  else 
    {
      drain_writes[channel] = 0; // No need to drain.
    }
  
  // initiate write drain if either the write queue occupancy
  // has reached the HI_WM , OR, if there are no pending read
  // requests
  if(write_queue_length[channel] > HI_WM)
    {
      drain_writes[channel] = 1;
    }
  else 
    {
      if (!read_queue_length[channel])
	{
	  drain_writes[channel] = 1;
	}
    }
  
  // If in write drain mode, look through all the write queue
  // elements (already arranged in the order of arrival), and
  // issue the command for the first request that is ready
  if(drain_writes[channel])
    {
      
      LL_FOREACH(write_queue_head[channel], wr_ptr)
	{
	  if(wr_ptr->command_issuable)
	    {
	      issue_request_command(wr_ptr);
	      return;;
	    }
	}
    }
  
  // do a refresh if there aren't many reads waiting
  if((read_queue_length[channel]==0))// && (CYCLE_VAL-cycle_last_aggressive_refresh > T_RFC))
    {
      // it's safe to try to do a refresh if there's no more pending work in this channel
      int i;
      for(i=0; i<NUM_RANKS; i++)
	{
	  if((is_refresh_allowed(channel,i)) && (refreshes[channel][i] < 8))
	    {
	      issue_refresh_command(channel,i);
//	      cycle_last_aggressive_refresh = CYCLE_VAL;
	      num_aggr_refresh++;
	      refreshes[channel][i]++;
	      printf("Issuing a refresh to c%d r%d at %lld\n",channel,i,CYCLE_VAL);
	      return;
	    }
	}
    }

  // Draining Reads
  // look through the queue and find the first request whose
  // command can be issued in this cycle and issue it 
  // Simple FCFS 
  if(!drain_writes[channel])
    {
      LL_FOREACH(read_queue_head[channel],rd_ptr)
	{
	  if(rd_ptr->command_issuable)
	    {
	      issue_request_command(rd_ptr);
	      return;
	    }
	}
    }
  
  return;
}

void scheduler_stats()
{
  /* Nothing to print for now. */
  printf("Number of Aggressive Refreshes: %lld\n", num_aggr_refresh);
}

