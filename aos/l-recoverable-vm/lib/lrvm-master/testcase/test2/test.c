#include "rvm.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/wait.h>
#include "testdefs.h"
#include <assert.h>

/* proc1 writes some data, commits it, then exits */
void proc1() 
{
     rvm_t rvm;
     trans_t trans;
     char* segs[1];
     
     rvm = rvm_init("log");
     rvm_destroy(rvm, "testseg");
     segs[0] = (char *) rvm_map(rvm, "testseg", 10000);

     trans = rvm_begin_trans(rvm, 1, (void **) segs);
     rvm_about_to_modify(trans, segs[0], 0, 100);
     sprintf(segs[0], "one");
     rvm_commit_trans(trans);

     trans = rvm_begin_trans(rvm, 1, (void **) segs);
     rvm_about_to_modify(trans, segs[0], 0, 100);
     sprintf(segs[0], "two");
     abort();
}


/* proc2 opens the segments and reads from them */
void proc2() 
{
     char* segs[1];
     rvm_t rvm;

     rvm = rvm_init("log");

     segs[0] = (char *) rvm_map(rvm, "testseg", 10000);
     assert(strcmp(segs[0], "one") == 0);

     printf("OK\n");
}


int main(int argc, char **argv)
{
     int pid;

     pid = fork();
     if(pid < 0) {
	  perror("fork");
	  exit(2);
     }
     if(pid == 0) {
	  proc1();
	  exit(0);
     }

     waitpid(pid, NULL, 0);

     proc2();

     return 0;
}
