#include <stdio.h>
#include <stdlib.h>
#include "utlist.h"
#include "utils.h"

#include "memory_controller.h"
#include "params.h"
#include "scheduler.h"
#include "processor.h"

#define OPT1 /*Optimization related to MLP and loclity of reference*/
#define OPT2 /*Optimization related to aggressive precharge*/

extern int NUMCORES;
extern struct robstructure *ROB;

long long int *current_row;
int *current_bank;
int *current_rank;
int thread_id = 0;

/* A data structure to see if a bank is a candidate for precharge. */
int recent_colacc[MAX_NUM_CHANNELS][MAX_NUM_RANKS][MAX_NUM_BANKS];

/* Keeping track of how many auto precharges and preemptive precharges are performed. */
long long int num_auto_precharge = 0;
long long int num_aggr_precharge = 0;


void init_scheduler_vars()
{
	int i, j, k;
		for (i=0; i<MAX_NUM_CHANNELS; i++) {
		  for (j=0; j<MAX_NUM_RANKS; j++) {
		    for (k=0; k<MAX_NUM_BANKS; k++) {
		      recent_colacc[i][j][k] = 0; /*Required to hold candidate information*/
		    }
		  }
		}
		/*Initialize data structures to store the previous incoming requests*/
	current_row = (long long int *) malloc(sizeof(long long int *)*NUM_CHANNELS);
		current_bank = (int *) malloc(sizeof(int *)*NUM_CHANNELS);
	        current_rank = (int *) malloc(sizeof(int *)*NUM_CHANNELS);
}


request_t * get_read_head (int channel) {
    long long int p_addr = 0;
    int numc;
    request_t * req = NULL;

    for(numc=0; numc<NUMCORES; numc++) {
    	p_addr = ROB[thread_id].mem_address[ROB[thread_id].head];
        LL_SEARCH_SCALAR(read_queue_head[channel],req, physical_address ,p_addr);
        if (req) {
            if(req->command_issuable && req->next_command == ACT_CMD) {
            	thread_id ++;
                thread_id = thread_id % NUMCORES;
                return req;
            }
            thread_id ++;
            thread_id = thread_id % NUMCORES;
        }
    }
    return NULL;
}

// write queue high water mark; begin draining writes if write queue exceeds this value
#define HI_WM 60

// end write queue drain once write queue has this many writes in it
#define LO_WM 50

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
	request_t * req_ptr = NULL;
	request_t * test_ptr = NULL;
	request_t * curr_ptr = NULL;
	dram_address_t temp;
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

    int read_issued = 0;
    int write_issued = 0;

	// If in write drain mode, try write hits and then issue possible requests
	if(drain_writes[channel])
	{

        // issue writes and let row hits go first
        LL_FOREACH(write_queue_head[channel], wr_ptr) {
            if (wr_ptr->command_issuable && wr_ptr->next_command == COL_WRITE_CMD) {
                issue_request_command(wr_ptr);
                write_issued = 1;
#ifdef OPT2
				recent_colacc[channel][wr_ptr->dram_addr.rank][wr_ptr->dram_addr.bank] = 1;
				/*If similar requests exist then don't close the row*/
				LL_FOREACH(read_queue_head[channel], curr_ptr) {
					if (!curr_ptr->request_served
						&& curr_ptr->dram_addr.rank == wr_ptr->dram_addr.rank
						&& curr_ptr->dram_addr.bank == wr_ptr->dram_addr.bank
						&& curr_ptr->dram_addr.row == wr_ptr->dram_addr.row
						&& curr_ptr != wr_ptr) {
						recent_colacc[channel][wr_ptr->dram_addr.rank][wr_ptr->dram_addr.bank] = 0;
					}
				}
#endif
               break;
            }
        }

        // issue other commands if possible
        if (!write_issued) {
            LL_FOREACH(write_queue_head[channel], wr_ptr)
		    {
    			if(wr_ptr->command_issuable)
	    		{
	     			if (wr_ptr->next_command == COL_WRITE_CMD) {
                        // should not happen here
		    		}
		    		if (wr_ptr->next_command == ACT_CMD) {
		    			recent_colacc[channel][wr_ptr->dram_addr.rank][wr_ptr->dram_addr.bank] = 0;
	    			}
	    			if (wr_ptr->next_command == PRE_CMD) {
	    				recent_colacc[channel][wr_ptr->dram_addr.rank][wr_ptr->dram_addr.bank] = 0;
						if (dram_state[channel][wr_ptr->dram_addr.rank][wr_ptr->dram_addr.bank].state == PRECHARGING) {
							continue;
						}
						int row_hit = 0;
						LL_FOREACH(write_queue_head[channel], test_ptr) {
							if (test_ptr->dram_addr.rank == wr_ptr->dram_addr.rank && test_ptr->dram_addr.bank == wr_ptr->dram_addr.bank && test_ptr->next_command == COL_WRITE_CMD) {
								row_hit = 1;
						 break;
							}
						}
						if (row_hit) {
							continue;
						}
                        num_aggr_precharge ++;
			    	}
			    	issue_request_command(wr_ptr);
			    	write_issued = 1;
                    break;
                }
            }

            /*If previous write request doesn't go through, then use this cycle
             * to issue an appropriate read request.*/
            if (!write_issued) {
                LL_FOREACH(read_queue_head[channel],rd_ptr) {
                	/*Compare previously stored request with the elements
                	 * in the queue. If there's a match, serve that request first.*/
                	if (rd_ptr->command_issuable) {
#ifdef OPT1
                    		temp= rd_ptr->dram_addr;
    						if( temp.rank ==  current_rank[channel]
    						 && temp.bank == current_bank[channel]
    						 && temp.row == current_row[channel] )
    						{
    							issue_request_command(rd_ptr);
    							read_issued = 1;
    							break;
    						}
#endif

                		if (rd_ptr->next_command == COL_READ_CMD) {

                			issue_request_command(rd_ptr);
                			current_rank[channel]=rd_ptr->dram_addr.rank;
							current_bank[channel]=rd_ptr->dram_addr.bank;
							current_row[channel]=rd_ptr->dram_addr.row;
                			read_issued = 1;
                    		break;
                    	}
                    }
                }
            }

        }
                if (!write_issued && !read_issued) {
                    return; // no request issued, quit
                }
                if (!write_issued && read_issued) {
                    wr_ptr = rd_ptr;
                }

                /*For a request which has already completed, but not precharged, we check if this
                 * particular row can result in future hits by comparing with requests in the queue.
                 * Once we are sure that there can be no potential row hits, we can issue a auto-precharge
                 * in the same cycle.*/
		if (cas_issued_current_cycle[channel][wr_ptr->dram_addr.rank][wr_ptr->dram_addr.bank]) {
			LL_FOREACH(write_queue_head[channel], req_ptr) {
				if (!req_ptr->request_served
					&& req_ptr->dram_addr.rank == wr_ptr->dram_addr.rank
					&& req_ptr->dram_addr.bank == wr_ptr->dram_addr.bank
                                        && req_ptr->dram_addr.row == wr_ptr->dram_addr.row) {
					return;
				}
			}
			LL_FOREACH(read_queue_head[channel], req_ptr) {
				if (!req_ptr->request_served
					&& req_ptr->dram_addr.rank == wr_ptr->dram_addr.rank
					&& req_ptr->dram_addr.bank == wr_ptr->dram_addr.bank
			        && req_ptr->dram_addr.row == wr_ptr->dram_addr.row) {
					return; // has hit, no auto precharge
				}
			}
			// no hit pending, auto precharge
			if (issue_autoprecharge(channel, wr_ptr->dram_addr.rank, wr_ptr->dram_addr.bank)) {
				num_auto_precharge++;
			}
		}
	}

	// Draining Reads
	if(!drain_writes[channel])
	{
        // try to issue read hit first
        LL_FOREACH(read_queue_head[channel],rd_ptr) {
        	if(rd_ptr->command_issuable)
			{
#ifdef OPT1
        		temp= rd_ptr->dram_addr;
				if( temp.rank ==  current_rank[channel]
				 && temp.bank == current_bank[channel]
				 && temp.row == current_row[channel] )
				{
					issue_request_command(rd_ptr);
					read_issued = 1;
					break;
				}
#endif
			}
        	if (rd_ptr->command_issuable && rd_ptr->next_command == COL_READ_CMD) {

				issue_request_command(rd_ptr);
				read_issued = 1;
#ifdef OPT2
				recent_colacc[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank] = 1;
				/*If similar requests exist then don't close the row*/
				LL_FOREACH(read_queue_head[channel], curr_ptr) {
					if (!curr_ptr->request_served
						&& curr_ptr->dram_addr.rank == rd_ptr->dram_addr.rank
						&& curr_ptr->dram_addr.bank == rd_ptr->dram_addr.bank
						&& curr_ptr->dram_addr.row == rd_ptr->dram_addr.row
						&& curr_ptr != rd_ptr) {
						recent_colacc[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank] = 0;
					}
				}
#endif
				break;
			}
		}

		if (!read_issued) {
			LL_FOREACH(read_queue_head[channel],rd_ptr)
			{
				if(rd_ptr->command_issuable)
				{
					if (rd_ptr->next_command == COL_READ_CMD) {
						//recent_colacc[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank] = 1;
					}
					if (rd_ptr->next_command == ACT_CMD) {
						recent_colacc[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank] = 0;
						request_t * tmp_ptr = get_read_head(channel);
						if (tmp_ptr != NULL) {
							rd_ptr = tmp_ptr;
						}
					}
					if (rd_ptr->next_command == PRE_CMD) {
						recent_colacc[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank] = 0;
						if (dram_state[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank].state == PRECHARGING) {
							continue;
						}
						int row_hit = 0;
						LL_FOREACH(read_queue_head[channel], test_ptr) {
							if (test_ptr->dram_addr.rank == rd_ptr->dram_addr.rank && test_ptr->dram_addr.bank == rd_ptr->dram_addr.bank && test_ptr->next_command == COL_READ_CMD) {
								row_hit = 1;
								break;
							}
						}
						if (row_hit) {
							continue;
						}
						num_aggr_precharge ++;
					}
					issue_request_command(rd_ptr);
					current_rank[channel]=rd_ptr->dram_addr.rank;
					current_bank[channel]=rd_ptr->dram_addr.bank;
					current_row[channel]=rd_ptr->dram_addr.row;
					read_issued = 1;
                	break;
				}
			}
			if (!read_issued) {
				LL_FOREACH(write_queue_head[channel],wr_ptr) {
					if (wr_ptr->command_issuable) {
						if (wr_ptr->next_command == COL_WRITE_CMD) {
							issue_request_command(wr_ptr);
							write_issued = 1;
							break;
						}
					}
				}
			}
		}
		if (!write_issued && !read_issued) {
			return;
		}
		if (write_issued && !read_issued) {
			rd_ptr = wr_ptr;
		}

		if (cas_issued_current_cycle[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank]) {
			LL_FOREACH(read_queue_head[channel], req_ptr) {
				if (!req_ptr->request_served
					&& req_ptr->dram_addr.rank == rd_ptr->dram_addr.rank
					&& req_ptr->dram_addr.bank == rd_ptr->dram_addr.bank
                    && req_ptr->dram_addr.row == rd_ptr->dram_addr.row) {
					return;
				}
			}
			LL_FOREACH(write_queue_head[channel], req_ptr) {
				if (!req_ptr->request_served
					&& req_ptr->dram_addr.rank == rd_ptr->dram_addr.rank
					&& req_ptr->dram_addr.bank == rd_ptr->dram_addr.bank
			        && req_ptr->dram_addr.row == rd_ptr->dram_addr.row) {
					return;
				}
			}
			if (issue_autoprecharge(channel, rd_ptr->dram_addr.rank, rd_ptr->dram_addr.bank)) {
				num_auto_precharge ++;
			}
		}
	}
	/*In case nothing else happens in this cycle, we can go ahead and precharge
	 * the candidate blocks*/
	if (!command_issued_current_cycle[channel]) {
	  for (int i=0; i<NUM_RANKS; i++) {
		for (int j=0; j<NUM_BANKS; j++) {  /* For all banks on the channel.. */
		  if (recent_colacc[channel][i][j]) {  /* See if this bank is a candidate. */
			if (is_precharge_allowed(channel,i,j)) {  /* See if precharge is doable. */
			  if (issue_precharge_command(channel,i,j)) {
				num_aggr_precharge++;
				recent_colacc[channel][i][j] = 0;
			  }
			}
		  }
		}
	  }
	}
}

void scheduler_stats()
{
  printf("Number of auto precharges: %lld\n", num_auto_precharge);
  printf("Number of aggressive precharges: %lld\n", num_aggr_precharge);
}
