/* Writes work with simple FR-FCFS. 
Reads employ FR-RR, which is a combination of "first-ready", and "round-robin". */


#include <stdio.h>
#include "utlist.h"
#include "utils.h"

#include "memory_controller.h"


extern long long int CYCLE_VAL;

extern int NUMCORES;


int rr_next = 0;

void init_scheduler_vars()
{
  // initialize all scheduler variables here


  return;
}

// write queue high water mark; begin draining writes if write queue exceeds this value
#define HI_WM 40

// end write queue drain once write queue has this many writes in it
#define LO_WM 32

// when switching to write drain mode, write at least this many times before switching back to read mode
#define MIN_WRITES_ONCE_WRITING_HAS_BEGUN 1

// 1 means we are in write-drain mode for that channel
int drain_writes[MAX_NUM_CHANNELS];

// how many writes have been performed since beginning current write drain
int writes_done_this_drain[MAX_NUM_CHANNELS];

// flag saying that we're only draining the write queue because there are no reads to schedule
int draining_writes_due_to_rq_empty[MAX_NUM_CHANNELS];

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
  
  // begin write drain if we're above the high water mark
  if((write_queue_length[channel] > HI_WM) && (!drain_writes[channel]))
    {
      drain_writes[channel] = 1;
      writes_done_this_drain[channel] = 0;
    }

  // also begin write drain if read queue is empty
  if((read_queue_length[channel] < 1) && (write_queue_length[channel] > 0) && (!drain_writes[channel]))
    {
      drain_writes[channel] = 1;
      writes_done_this_drain[channel] = 0;
      draining_writes_due_to_rq_empty[channel] = 1;
    }

  // end write drain if we're below the low water mark
  if((drain_writes[channel]) && (write_queue_length[channel] <= LO_WM) && (!draining_writes_due_to_rq_empty[channel]))
    {
      drain_writes[channel] = 0;
    }

  // end write drain that was due to read_queue emptiness only if at least one write has completed
  if((drain_writes[channel]) && (read_queue_length[channel] > 0) && (draining_writes_due_to_rq_empty[channel]) && (writes_done_this_drain[channel] > MIN_WRITES_ONCE_WRITING_HAS_BEGUN))
    {
      drain_writes[channel] = 0;
      draining_writes_due_to_rq_empty[channel] = 0;
    }

  // make sure we don't try to drain writes if there aren't any
  if(write_queue_length[channel] == 0)
    {
      drain_writes[channel] = 0;
    }

  // drain from write queue now
  if(drain_writes[channel])
    {
      // prioritize open row hits
      LL_FOREACH(write_queue_head[channel], wr_ptr)
	{
	  // if COL_WRITE_CMD is the next command, then that means the appropriate row must already be open
	  if(wr_ptr->command_issuable && (wr_ptr->next_command == COL_WRITE_CMD))
	    {
	      writes_done_this_drain[channel]++;
	      issue_request_command(wr_ptr);
	      return;
	    }
	}

      // if no open rows, just issue any other available commands
      LL_FOREACH(write_queue_head[channel], wr_ptr)
	{
	  if(wr_ptr->command_issuable)
	    {
	      issue_request_command(wr_ptr);
	      return;
	    }
	}

      // nothing issuable this cycle
      return;
    }

  // do a read



    //FR-RR - First ready plus round robin


     //see if you can get a row-buffer hit with correct thread id,
     //and advance rr_next
     LL_FOREACH(read_queue_head[channel],rd_ptr){

         if((rd_ptr->thread_id == rr_next) && 
            (rd_ptr->command_issuable) && 
            (rd_ptr->next_command == COL_READ_CMD)){

                issue_request_command( rd_ptr);
                
                rr_next = (rr_next + 1)%NUMCORES;

                return;
         }
     }


     //if not, see if you can get a row-buffer hit with wrong thread id,
     //but don't advance lo
     LL_FOREACH(read_queue_head[channel],rd_ptr){

         if((rd_ptr->command_issuable) && 
            (rd_ptr->next_command == COL_READ_CMD)){

                issue_request_command( rd_ptr);
                
                return;
         }
     }


     //if no row buffer hits, try other command from rr_next,
     //and advance rr_next 
     LL_FOREACH(read_queue_head[channel],rd_ptr){

         if((rd_ptr->thread_id == rr_next) &&
            (rd_ptr->command_issuable)){ 

                issue_request_command( rd_ptr);
                
                rr_next = (rr_next + 1)%NUMCORES;
                
                return;
         }
     }

     //if even that doesn't work, simply issue a random request, 
     //but don't advance rr_next

     LL_FOREACH(read_queue_head[channel],rd_ptr){

         if(rd_ptr->command_issuable){ 

                issue_request_command( rd_ptr);
                
                return;
         }
     }


}

void scheduler_stats()
{
  /* Nothing to print for now. */
}

