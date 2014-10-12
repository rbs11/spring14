#include "rvm.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include "testdefs.h"
#include <assert.h>

#define SIZE 10000


/* proc1 writes some data, commits it, then exits */
void proc1() 
{
     rvm_t rvm;
     trans_t trans;
     char* segs[1];
     unsigned char *foo;
     int i;
     
     rvm = rvm_init("log");
     rvm_destroy(rvm, "testseg");
     segs[0] = (char *) rvm_map(rvm, "testseg", SIZE);
     
     trans = rvm_begin_trans(rvm, 1, (void **) segs);
     
     rvm_about_to_modify(trans, segs[0], 0, SIZE);
     foo = (unsigned char *) segs[0];

     for(i = 0; i < SIZE; i++)
       foo[i] = i % 255;
     
     rvm_commit_trans(trans);

     exit(0);
}


/* proc2 opens the segments and reads from them */
void proc2() 
{
     char* segs[1];
     rvm_t rvm;
     unsigned char *foo;
     int i;
     
     rvm = rvm_init("log");

     segs[0] = (char *) rvm_map(rvm, "testseg", 10000);

     foo = (unsigned char *) segs[0];
     for(i = 0; i < SIZE; i++)
       assert(foo[i] == i%255);

     printf("OK\n");
     exit(0);
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
