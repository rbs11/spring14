#include <stdio.h>
#include <stdlib.h>
#include "utlist.h"
#include "utils.h"

#include "memory_controller.h"
#include "params.h"
#include "scheduler.h"
#include "processor.h"

/* Try to do auto-precharge after col access if there is no pending requests on the row*/

/* Policy
 * 1) read is processed before writes (read first)
 * 2) when write queue is about to be full, process writes before reads (write first)
 * 3) when at read first, try to issue row hits first; if no row hits,
 *    issue the requests from ROB head in round-robin
 * 4) If all rob head is serviced, issue reads based on FCFS, if
 *    no reads can be processed, issue write hits
 * 5) when at write first, try to issue row hits first; if no row hits, issue writes based on FCFS, if no writes can be processed, issue read hits
 * 6) do auto precharge the open row together with column access if there is no request pending on the open row.
 */

extern int NUMCORES;
extern struct robstructure *ROB;
//extern long long int CYCLE_VAL; [REMOVED!!!]

/* Keeping track of how many auto precharges and preemptive precharges are performed. */
long long int num_auto_precharge = 0;
long long int num_aggr_precharge = 0;

// row_entry_queue groups requests to the same row together [REMOVED!!!]
// every channel has a queue [REMOVED!!!]
//same_row_entry_t row_entry_queue [MAX_NUM_CHANNELS][MAX_RE_NUM]; [REMOVED!!!]

void init_scheduler_vars()
{
    // initialize all scheduler variables here
    /*for (int j = 0; j < MAX_RE_NUM; j++) {
            row_entry_queue[i][j].rank = -1;
	        row_entry_queue[i][j].bank = -1;
        	row_entry_queue[i][j].row = -1;
	        row_entry_queue[i][j].request_num = 0;
	        row_entry_queue[i][j].row_entry_head = NULL;
        }
    }

	committed_read_request = (int*) malloc(sizeof(int)*NUMCORES);
	thread_priority_head = (int*) malloc(sizeof(int)*NUMCORES);

	for (int i = 0; i < NUMCORES; i++) {
		committed_read_request[i] = 0;
		thread_priority_head[i] = 999; // 999 means no thread id
	}

    return;*/
}

/* The following codes does not give performance benifit, REMOVED!!!!

// update priority per thread.
// the thread has the least bandwidth gets the highest priority

void update_priority() {

	// clear all previous thread prioritys
	for (int i = 0; i < NUMCORES; i++) {
		thread_priority_head[i] = 999;
	}

	for (int i = 0; i < NUMCORES; i++) {
		if (committed_read_request[i] > 0) {
			for (int j = 0; j < NUMCORES; j++) {   // insertion sort
				if (thread_priority_head[j] < 999) {
					if (0 < committed_read_request[thread_priority_head[j]]
					    && committed_read_request[thread_priority_head[j]] < committed_read_request[i]) {
						continue;
					}
					else {
						int end = NUMCORES - 1;
						for (; end > j; end--) {
							thread_priority_head[end] = thread_priority_head[end - 1];
						}
						thread_priority_head[end] = i;
						break;
					}
				}
				else {
					thread_priority_head[j] = i;
					break;
				}
			}
		}
	}

	// clear committed request record

	for (int i = 0; i < NUMCORES; i++) {
		committed_read_request[i] = 0;
	}
}

// compare and return if thread id1 is higher priority than thread id2

int priority_higher(int id1, int id2) {
	for (int i = 0; i < NUMCORES; i++) {
		if (thread_priority_head[i] == id1) {
			return 1;
		}

		if (thread_priority_head[i] == id2) {
			return 0;
		}
	}

	return 0;
}

// controller function performing every memory cycle
// group requests to the same row together
// and return the oldest request of the largest group

// improvement: for more than 2 channels, make the ILP thread run faster (removed)

void * find_best_read_to_act(int channel) {
	request_t * rd_ptr = NULL;

	// can not trigger this function when memory controller receive a new request
	// have to clear row_entry_queue every time
	for (int j = 0; j < MAX_RE_NUM; j++) {
		row_entry_queue[channel][j].rank = -1;
		row_entry_queue[channel][j].bank = -1;
		row_entry_queue[channel][j].row = -1;
		row_entry_queue[channel][j].request_num = 0;
		row_entry_queue[channel][j].row_entry_head = NULL;
	}

	LL_FOREACH(read_queue_head[channel], rd_ptr) {
		for (int j = 0; j < MAX_RE_NUM; j++) {
			if (row_entry_queue[channel][j].request_num == 0) {
				// there is free entry and the request hits a new row
				row_entry_queue[channel][j].rank = rd_ptr->dram_addr.rank;
				row_entry_queue[channel][j].bank = rd_ptr->dram_addr.bank;
				row_entry_queue[channel][j].row = rd_ptr->dram_addr.row;
				row_entry_queue[channel][j].request_num = 1;
				row_entry_queue[channel][j].row_entry_head = (void *) rd_ptr;
				break;
			}
			else if (row_entry_queue[channel][j].rank == rd_ptr->dram_addr.rank
					&& row_entry_queue[channel][j].bank == rd_ptr->dram_addr.bank
					&& row_entry_queue[channel][j].row == rd_ptr->dram_addr.row) {
				row_entry_queue[channel][j].request_num ++;
				break;
			}
		}
	}

	// this switch controls if the priority kicks in
	int priority_switch = 0; // closed

	if (NUM_CHANNELS > 2) { // turn on the switch if more channels are presented in the system
		priority_switch = 1;
	}

	// find the row entry with the most number of pending requests
	// it must not be the current open row
	int max_row = 0;
	int max_row_index = -1;
	for (int j = 0; j < MAX_RE_NUM; j++) {
		if (row_entry_queue[channel][j].request_num > 0) {
			if (((request_t *) row_entry_queue[channel][j].row_entry_head)->next_command != COL_READ_CMD
					&& ((request_t *)row_entry_queue[channel][j].row_entry_head)->command_issuable) {
				if (priority_switch == 0) {
				    if (row_entry_queue[channel][j].request_num > max_row) {
					    max_row_index = j;
					    max_row = row_entry_queue[channel][j].request_num;
				    }
				}
				else {
					if (max_row_index != -1) {
						if (priority_higher(((request_t *)row_entry_queue[channel][j].row_entry_head)->thread_id, ((request_t *)row_entry_queue[channel][max_row_index].row_entry_head)->thread_id)) {
							max_row_index = j;
							max_row = row_entry_queue[channel][j].request_num;
						}
						else if (((request_t *)row_entry_queue[channel][j].row_entry_head)->thread_id == ((request_t *)row_entry_queue[channel][max_row_index].row_entry_head)->thread_id) {
							if (row_entry_queue[channel][j].request_num > max_row) {
							    max_row_index = j;
							    max_row = row_entry_queue[channel][j].request_num;
							}
						}
					}
					else {
						if (row_entry_queue[channel][j].request_num > max_row) {
						    max_row_index = j;
						    max_row = row_entry_queue[channel][j].request_num;
						}
					}
				}
			}
		}
	}

	request_t * current_highest = NULL;

	// find the highest priority issuable command in read queue
	if (priority_switch) {
		if (max_row_index != -1) {
			current_highest = ((request_t *)row_entry_queue[channel][max_row_index].row_entry_head);
		}
		else {
			current_highest = read_queue_head[channel];
		}
		LL_FOREACH(read_queue_head[channel], rd_ptr) {
		    if (priority_higher(rd_ptr->thread_id, current_highest->thread_id)) {
		    	current_highest = rd_ptr;
		    }
	    }
		return ((void *) current_highest);
	}

	if (max_row_index != -1) {
		return (row_entry_queue[channel][max_row_index].row_entry_head);
	}
	else {
		return NULL;
	}
}
End of removed codes  */

int curr_thread = 0;

void inc_curr_thread () {
    curr_thread ++;
    curr_thread = curr_thread % NUMCORES;
}

request_t * get_rob_top (int channel) {
    long long int physical_address = 0;
    int numc;
    request_t * rq_ptr = NULL;

    for(numc=0; numc<NUMCORES; numc++) {
        physical_address = ROB[curr_thread].mem_address[ROB[curr_thread].head];
        LL_SEARCH_SCALAR(read_queue_head[channel],rq_ptr, physical_address ,physical_address);
        if (rq_ptr) {
            if(rq_ptr->command_issuable && rq_ptr->next_command == ACT_CMD) {
               inc_curr_thread();
               return rq_ptr;
            }
            inc_curr_thread();
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
	request_t * auto_ptr = NULL;
        request_t * test_ptr = NULL;

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
	    			}
	    			if (wr_ptr->next_command == PRE_CMD) {
						if (dram_state[channel][wr_ptr->dram_addr.rank][wr_ptr->dram_addr.bank].state == PRECHARGING) {
							continue;
						}
						int has_hit = 0;
						LL_FOREACH(write_queue_head[channel], test_ptr) {
							if (test_ptr->dram_addr.rank == wr_ptr->dram_addr.rank && test_ptr->dram_addr.bank == wr_ptr->dram_addr.bank && test_ptr->next_command == COL_WRITE_CMD) {
						 has_hit = 1;
						 break;
							}
						}
						if (has_hit) {
							continue;
						}
                        num_aggr_precharge ++;
			    	}
			    	issue_request_command(wr_ptr);
			    	write_issued = 1;
                    break;
                }
            }

            // try to issue read hit
            if (!write_issued) {
                LL_FOREACH(read_queue_head[channel],rd_ptr) {
                	if (rd_ptr->command_issuable) {
                		if (rd_ptr->next_command == COL_READ_CMD) {
                			issue_request_command(rd_ptr);
                			read_issued = 1;
                    		break;
                    	}
                    }
                }
            }

        }

                // try auto-precharge
                if (!write_issued && !read_issued) {
                    return; // no request issued, quit
                }

                if (!write_issued && read_issued) {
                    wr_ptr = rd_ptr;
                }

		if (cas_issued_current_cycle[channel][wr_ptr->dram_addr.rank][wr_ptr->dram_addr.bank]) {
			//if (!write_issued && read_issued) {
			//	wr_ptr = rd_ptr;
			//}
			LL_FOREACH(write_queue_head[channel], auto_ptr) {
				if (!auto_ptr->request_served
					&& auto_ptr->dram_addr.rank == wr_ptr->dram_addr.rank
					&& auto_ptr->dram_addr.bank == wr_ptr->dram_addr.bank
                                        && auto_ptr->dram_addr.row == wr_ptr->dram_addr.row) {
					return; // has hit, no auto precharge
				}
			}
			LL_FOREACH(read_queue_head[channel], auto_ptr) {
				if (!auto_ptr->request_served
					&& auto_ptr->dram_addr.rank == wr_ptr->dram_addr.rank
					&& auto_ptr->dram_addr.bank == wr_ptr->dram_addr.bank
			        && auto_ptr->dram_addr.row == wr_ptr->dram_addr.row) {
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
        	if (rd_ptr->command_issuable && rd_ptr->next_command == COL_READ_CMD) {
				issue_request_command(rd_ptr);
				read_issued = 1;
                break;
			}
		}

		// issue other requests if possible
		if (!read_issued) {
			LL_FOREACH(read_queue_head[channel],rd_ptr)
			{
				if(rd_ptr->command_issuable)
				{
					if (rd_ptr->next_command == COL_READ_CMD) {
					}
					if (rd_ptr->next_command == ACT_CMD) {
                                            request_t * tmp_ptr = get_rob_top(channel);
                                            if (tmp_ptr != NULL) {
                                                rd_ptr = tmp_ptr;
                                            }
					}
					if (rd_ptr->next_command == PRE_CMD) {
                                                if (dram_state[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank].state == PRECHARGING) {
                                                    continue;
                                                }
                                                int has_hit = 0;
                                                LL_FOREACH(read_queue_head[channel], test_ptr) {
                                                    if (test_ptr->dram_addr.rank == rd_ptr->dram_addr.rank && test_ptr->dram_addr.bank == rd_ptr->dram_addr.bank && test_ptr->next_command == COL_READ_CMD) {
                                                        has_hit = 1;
                                                        break;
                                                    }
                                                }
                                                if (has_hit) {
                                                    continue;
                                                }
						num_aggr_precharge ++;
					}
					issue_request_command(rd_ptr);
					read_issued = 1;
                	                break;
				}
			}
			// try to issue write hit
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

		// try auto-precharge
                if (!write_issued && !read_issued) {
                    return; // no request issued, quit
                }

                if (write_issued && !read_issued) {
                    rd_ptr = wr_ptr;
                }


		if (cas_issued_current_cycle[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank]) {
			//if (!read_issued && write_issued) {
			//	rd_ptr = wr_ptr;
			//}
			LL_FOREACH(read_queue_head[channel], auto_ptr) {
				if (!auto_ptr->request_served
					&& auto_ptr->dram_addr.rank == rd_ptr->dram_addr.rank
					&& auto_ptr->dram_addr.bank == rd_ptr->dram_addr.bank
                    && auto_ptr->dram_addr.row == rd_ptr->dram_addr.row) {
					return; // has hit, no auto precharge
				}
			}
			LL_FOREACH(write_queue_head[channel], auto_ptr) {
				if (!auto_ptr->request_served
					&& auto_ptr->dram_addr.rank == rd_ptr->dram_addr.rank
					&& auto_ptr->dram_addr.bank == rd_ptr->dram_addr.bank
			        && auto_ptr->dram_addr.row == rd_ptr->dram_addr.row) {
					return; // has hit, no auto precharge
				}
			}
			// no hit pending, auto precharge
			if (issue_autoprecharge(channel, rd_ptr->dram_addr.rank, rd_ptr->dram_addr.bank)) {
				num_auto_precharge ++;
			}
		}
	}

}

void scheduler_stats()
{
  /* Nothing to print for now. */
  printf("Number of auto precharges: %lld\n", num_auto_precharge);
  printf("Number of aggressive precharges: %lld\n", num_aggr_precharge);
}
