/* abort.c - test that aborting a modification returns the segment to
 * its initial state */

#include "rvm.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_STRING1 "hello, world"
#define TEST_STRING2 "bleg!"
#define OFFSET2 1000


int main(int argc, char **argv)
{
     rvm_t rvm;
     char *seg;
     void *segs[1];
     trans_t trans;
     
	 rvm = rvm_init(__FILE__ ".d");
     
     rvm_destroy(rvm, "testseg");
     
     segs[0] = (char *) rvm_map(rvm, "testseg", 10000);
	 seg = (char *) segs[0];

     trans = rvm_begin_trans(rvm, 1, segs);
     rvm_about_to_modify(trans, seg, 0, 100);
     sprintf(seg, TEST_STRING1);
     
     rvm_about_to_modify(trans, seg, OFFSET2, 100);
     sprintf(seg+OFFSET2, TEST_STRING2);
     
     rvm_commit_trans(trans);


	 printf("Before Truncation:\n");
	 system("ls -l " __FILE__ ".d");

	 rvm_truncate_log(rvm);
	 
	 printf("\nAfter Truncation:\n");
	 system("ls -l " __FILE__ ".d");

     rvm_unmap(rvm, seg);
	 exit(0);
}

