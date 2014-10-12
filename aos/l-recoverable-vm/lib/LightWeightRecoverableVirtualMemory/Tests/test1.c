#include "rvm.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int main(int argc, char **argv)
{
     rvm_t rvm;
     char *seg;
     void *segs[1];
     trans_t trans;
     
     rvm = rvm_init("log");
     
     rvm_destroy(rvm, "testseg");
     
     segs[0] = (char *) rvm_map(rvm, "testseg", 10000);
	 seg = (char *) segs[0];

     /* write some data and commit it */
     trans = rvm_begin_trans(rvm, 1, segs);
     rvm_about_to_modify(trans, seg, 0, 100);
     sprintf(seg, "test1");
     
     rvm_about_to_modify(trans, seg, 100, 100);
     sprintf(seg+100, "test2");
     
     rvm_commit_trans(trans);

     /* start writing some different data, but abort */
     trans = rvm_begin_trans(rvm, 1, segs);
     rvm_about_to_modify(trans, seg, 0, 100);
     sprintf(seg, "test3");

     rvm_abort_trans(trans);

     /* test that the data was restored */
     assert(strcmp(seg, "test1") == 0);
     assert(strcmp(seg+100, "test2") == 0);

     rvm_unmap(rvm, seg);
     printf("OK\n");
     exit(0);
}

