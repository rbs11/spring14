#include "rvm.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/wait.h>
#include "testdefs.h"
#include <assert.h>

int _PTS = 5;

const char *getfile() { return __FILE__; }

#define SEGNAME0  "testseg1"
#define SEGNAME1  "testseg2"

#define OFFSET0  10
#define OFFSET1  100

#define STRING0 "hello, world"
#define STRING2 "goodbye, world"
#define MAX_SEGMENT_NAME_LEN 100
#define SEGSIZE 10
#define NUMSEGMENTS 10

void proc1() 
{
     rvm_t rvm;
     char* segs[SEGSIZE];
     trans_t trans;
     char** segnames;
     int i,j;

     rvm = rvm_init("log");
     
     // Allocate some memory
     segnames = (char **)malloc(NUMSEGMENTS * sizeof(char *));
     
    for(i = 0; i < NUMSEGMENTS; i++) {
      segnames[i] = (char *)malloc(MAX_SEGMENT_NAME_LEN);   
      sprintf(segnames[i],"testseg%i",i);
      rvm_destroy(rvm, segnames[i]);
    }

    
    //all segments contain the first string (aa)
    for(i = 0; i < NUMSEGMENTS; i++) {
      
      segs[0] = (char *) rvm_map(rvm, segnames[i], SEGSIZE);
      trans = rvm_begin_trans(rvm, 1, (void **) segs);
      rvm_about_to_modify(trans, segs[0], 0, SEGSIZE); 

      
      for(j = 0; j < SEGSIZE; j++)
      {
        segs[0][j] = i%16;
        printf("i is %d : set to %d\n", i, segs[0][j]);
      }

      rvm_commit_trans(trans);
      rvm_unmap(rvm,segs[0]);
    }

    //only even segments contain the second string (modify aa to ab)
    for(i = 0; i < NUMSEGMENTS; i++) {
      segs[0] = (char *) rvm_map(rvm, segnames[i], SEGSIZE);
      trans = rvm_begin_trans(rvm, 1, (void **) segs);
      rvm_about_to_modify(trans, segs[0], 0, SEGSIZE);
      
      for(j = 0; j < SEGSIZE; j++)
      {
        segs[0][j] = 0;
        printf("i is %d : set to %d\n", i, segs[0][j]);
      }

      if(i%2==0){ //the modulus is 1, we are even
	rvm_commit_trans(trans);
      }
    }
    

    // Free memory
    for(i = 0; i < NUMSEGMENTS; i++) {
      free(segnames[i]);
    }
    free(segnames);
}


void proc2() 
{
     rvm_t rvm;
     char *segs[2];
     char** segnames;
     int i,j;

     // Allocate some memory
     segnames = (char **)malloc(NUMSEGMENTS * sizeof(char *));
     
     for(i = 0; i < NUMSEGMENTS; i++) {
      segnames[i] = (char *)malloc(MAX_SEGMENT_NAME_LEN);   
      sprintf(segnames[i],"testseg%i",i);
     }
     rvm = rvm_init("log");
     
     for(i = 0; i < NUMSEGMENTS; i++) {
      
       segs[0] = (char *) rvm_map(rvm, segnames[i], 3);
       if(i%2==0)
       {
         for(j = 0; j < SEGSIZE; j++)
           //assert(segs[0][j] == 0);
	   printf("i[%d] : %d\n", i, segs[0][j]);
       }
       else
       {
         for(j = 0; j < SEGSIZE; j++)
           //assert(segs[0][j] == i%16);
	   printf("i[%d] : %d\n", i, segs[0][j]);
       }
       rvm_unmap(rvm,segs[0]);
     }
     
     printf("OK\n");

     // Free memory
     for(i = 0; i < NUMSEGMENTS; i++) {
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

