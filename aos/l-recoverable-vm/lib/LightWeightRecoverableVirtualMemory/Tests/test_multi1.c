/* test_multi1.c - 
- map 100 segments
- start 100 transactions (one per segment)
- trans1: modify all segments to aa
- map/unmap with same range
- trans1: modify all segments to ab
- commit every other segment
- abort
- check the log
*/


#include "rvm.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/wait.h>
#include "testdefs.h"

int _PTS = 5;

const char *getfile() { return __FILE__; }

#define SEGNAME0  "testseg1"
#define SEGNAME1  "testseg2"

#define OFFSET0  10
#define OFFSET1  100

#define STRING0 "hello, world"
#define STRING2 "goodbye, world"
#define MAX_SEGMENT_NAME_LEN 100

void proc1() 
{
     rvm_t rvm;
     char* segs[2];
     trans_t trans;
     char** segnames;
     int i;
     int numSegments = 100;

     rvm = rvm_init(__FILE__ ".d");
     
     // Allocate some memory
     segnames = (char **)malloc(numSegments * sizeof(char *));
     
    for(i = 0; i < numSegments; i++) {
      segnames[i] = (char *)malloc(MAX_SEGMENT_NAME_LEN);   
      sprintf(segnames[i],"testseg%i",i);
      rvm_destroy(rvm, segnames[i]);
    }

    
    //all segments contain the first string (aa)
    for(i = 0; i < numSegments; i++) {
      
      segs[0] = (char *) rvm_map(rvm, segnames[i], 3);
      trans = rvm_begin_trans(rvm, 1, (void **) segs);
      rvm_about_to_modify(trans, segs[0], 0, 3); 
      sprintf(segs[0],"aa");
      rvm_commit_trans(trans);
      rvm_unmap(rvm,segs[0]);
    }

    
    //only even segments contain the second string (modify aa to ab)
    for(i = 0; i < numSegments; i++) {
      segs[0] = (char *) rvm_map(rvm, segnames[i], 3);
      trans = rvm_begin_trans(rvm, 1, (void **) segs);
      rvm_about_to_modify(trans, segs[0], 1, 1); 
      segs[0][1]='b';
      assert(strcmp("ab", segs[0]) == 0);
      if(i%2==0){ //the modulus is 1, we are even
	rvm_commit_trans(trans);
      }
    }
    

    // Free memory
    for(i = 0; i < numSegments; i++) {
      free(segnames[i]);
    }
    free(segnames);
}


void proc2() 
{
     rvm_t rvm;
     char *segs[2];
     char** segnames;
     int i;
     int numSegments = 100;

     // Allocate some memory
     segnames = (char **)malloc(numSegments * sizeof(char *));
     
     for(i = 0; i < numSegments; i++) {
      segnames[i] = (char *)malloc(MAX_SEGMENT_NAME_LEN);   
      sprintf(segnames[i],"testseg%i",i);
     }
     rvm = rvm_init(__FILE__ ".d");
     
     for(i = 0; i < numSegments; i++) {
      
       segs[0] = (char *) rvm_map(rvm, segnames[i], 3);
       if(i%2==0){
	 if(strcmp(segs[0], "ab")) { 
	   printf("ERROR in segment %i, string should be ab, but it is (%s)\n",
		  i,segs[0]);
	   exit(2);
	 } 
       } else {
	 if(strcmp(segs[0], "aa")) { 
	   printf("ERROR in segment %i, string should be aa, but it is (%s)\n",
		  i,segs[0]);
	   exit(2);
	 } 
       }
       rvm_unmap(rvm,segs[0]);
     }
     

     printf("OK\n");

     // Free memory
     for(i = 0; i < numSegments; i++) {
       free(segnames[i]);
     }
     free(segnames);
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

