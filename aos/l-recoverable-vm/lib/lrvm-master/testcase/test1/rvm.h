#define trans_t long int
typedef struct rvm_t
{
char dir[200];
int rvmid;
}rvm_t;



rvm_t rvm_init(const char *directory); // Initialize the library with the specified directory as backing store.

extern void *rvm_map(rvm_t rvm, const char *segname, int size_to_create); // map a segment from disk into memory. If the segment does not already exist, then create it and give it size size_to_create. If the segment exists but is shorter than size_to_create, then extend it until it is long enough. It is an error to try to map the same segment twice.

extern void rvm_unmap(rvm_t rvm, void *segbase); // unmap a segment from memory.

extern void rvm_destroy(rvm_t rvm, const char *segname); //destroy a segment completely, erasing its backing store. This function should not be called on a segment that is currently mapped.

extern trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases); // begin a transaction that will modify the segments listed in segbases. If any of the specified segments is already being modified by a transaction, then the call should fail and return (trans_t) -1.

extern void rvm_about_to_modify(trans_t tid, void *segbase, int offset, int size); //declare that the library is about to modify a specified range of memory in the specified segment. The segment must be one of the segments specified in the call to rvm_begin_trans. Your library needs to ensure that the old memory has been saved, in case an abort is executed. It is legal call rvm_about_to_modify multiple times on the same memory area.

extern void rvm_commit_trans(trans_t tid); //commit all changes that have been made within the specified transaction. When the call returns, then enough information should have been saved to disk so that, even if the program crashes, the changes will be seen by the program when it restarts.

extern void rvm_abort_trans(trans_t tid); //undo all changes that have happened within the specified transaction.

extern void rvm_truncate_log(rvm_t rvm); //play through any committed or aborted items in the log file(s) and shrink the log file(s) as much as possible.

