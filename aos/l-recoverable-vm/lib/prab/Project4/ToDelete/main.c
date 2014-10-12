/* 
 * File:   main.c
 * Author: machiry
 *
 * Created on April 5, 2013, 7:02 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>

/*
 * 
 */
int main(int argc, char** argv) {

    DIR *dir;
struct dirent *ent;
if ((dir = opendir ("/media/data/OnlineData/DropBoxSyncFolder/Dropbox/GatechCourseInfo/AOS/Project4/LRVM/")) != NULL) {
  /* print all the files and directories within directory */
  while ((ent = readdir (dir)) != NULL) {
    printf ("%s\n", ent->d_name);
  }
  closedir (dir);
} else {
  /* could not open directory */
  perror ("");
  return EXIT_FAILURE;
}
}

