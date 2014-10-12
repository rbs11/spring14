/* 
 * File:   utilities.c
 * Author: machiry
 *
 * Created on April 6, 2013, 1:14 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h> 
#include <fcntl.h>
#include "utilities.h"
#include <time.h>

int isRandInitialized = 0;
unsigned long currTransactionID = 1;

int copyFile(char *src, char *dst) {
    char cmd[1024];
    sprintf(cmd, "cp -f %s %s", src, dst);
    return system(cmd);
}

int deleteFile(char *file) {
    char cmd[1024];
    sprintf(cmd, "rm -f %s", file);
    return system(cmd);
}

void copyExtended(const char *src, const char *dst, unsigned long size) {
    FILE* in_fd = fopen(src, "r");
    FILE* out_fd = fopen(dst, "w");
    char buf[8192];
    unsigned long noOfBytesCopied = 0;
    while (1) {
        ssize_t result = fread(&buf[0],1,sizeof (buf),in_fd);
        if (!result) break;
        fwrite(&buf[0],1,result,out_fd);
        noOfBytesCopied += result;
    }
    fclose(in_fd);
    if (noOfBytesCopied < size) {
        unsigned long toCopy = size - noOfBytesCopied;
        unsigned long temp;
        memset(&buf, 0, sizeof (buf));
        while (toCopy) {
            temp = toCopy > sizeof (buf) ? sizeof (buf) : toCopy;
            fwrite(&buf[0],1,temp,out_fd);
            toCopy = toCopy - temp;
        }
    }
    fclose(out_fd);
}

void createNullFile(const char* file, unsigned long size) {
    FILE* out_fd = fopen(file, "w");
    char buf[8192];
    unsigned long noOfBytesCopied = 0;
    if (noOfBytesCopied < size) {
        unsigned long toCopy = size - noOfBytesCopied;
        unsigned long temp;
        memset(&buf, 0, sizeof (buf));
        while (toCopy) {
            temp = toCopy > sizeof (buf) ? sizeof (buf) : toCopy;
            fwrite(&buf[0],1,temp,out_fd);
            toCopy = toCopy - temp;
        }
    }
    fclose(out_fd);
}

void inSufficientMemory(const char* errMsg) {
    fprintf(stderr, "Insufficient Memory:%s\n", errMsg);
    abort();
}

char* combinePaths(const char* dir, const char* fileName) {
    char* toRet = (char*) malloc(strlen(dir) + strlen(fileName) + strlen("/") + 1);
    if (!toRet) {
        inSufficientMemory("combine paths");
    }
    strcpy(toRet, dir);
    strcat(toRet, "/");
    strcat(toRet, fileName);
    return toRet;
}

int startsWith(const char *pre, const char *str) {
    size_t lenpre = strlen(pre),
            lenstr = strlen(str);
    return lenstr < lenpre ? 0 : strncmp(pre, str, lenpre) == 0;
}

int isFileExists(char *directory, char *fileName) {
    int to_ret=0;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(directory)) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir(dir)) != NULL) {
            to_ret = !strcmp(ent->d_name,fileName);
            if(to_ret){
                break;
            }
        }
        closedir(dir);
    }
    return to_ret;
}

void createFile(const char* fileName) {
    if (fileName) {
        FILE* fd2 = fopen(fileName, "w+");
        if (!fd2) {
            fprintf(stderr, "Problem occured while trying to create file:%s\n", fileName);
            abort();
        } else {
            fclose(fd2);
        }
    }
}

void writeBytesToFile(FILE* src, FILE* dst, unsigned long noOfBytes) {
    char buf[8192];
    unsigned long toCopy = noOfBytes;
    while (toCopy) {
        unsigned long temp = toCopy > sizeof (buf) ? sizeof (buf) : toCopy;
        ssize_t result = fread(&buf[0],1,temp,src);
        if (!result) break;
        fwrite(&buf[0],1,result,dst);
        toCopy = toCopy - temp;
    }
}

void writeBytesToFileFromMemory(FILE* toFile, void *memory, unsigned long size) {
    char buf[8192];
    unsigned long toCopy = size;
    while (toCopy) {
        unsigned long temp = toCopy > sizeof (buf) ? sizeof (buf) : toCopy;
        memcpy(buf, memory, temp);
        memory = memory + temp;
        fwrite(&buf[0],1,temp,toFile);
        toCopy = toCopy - temp;
    }
}

void writeBytesToMemory(FILE* fromFile, void *memory, unsigned long size) {
    char buf[8192];
    unsigned long toCopy = size;
    while (toCopy) {
        unsigned long temp = toCopy > sizeof (buf) ? sizeof (buf) : toCopy;
        ssize_t result = fread(&buf[0],1,temp,fromFile);
        if (!result) break;
        if (memory) {
            memcpy(memory, buf, result);
            memory += result;
        }
        toCopy = toCopy - temp;
    }
}

unsigned long getNextRandomID(){
    if(!isRandInitialized){
        srand(time(NULL));
        isRandInitialized = 1;
    }
    return rand();
    
}

unsigned long getNextTransactionID(){
    return currTransactionID++;
}

int readLineFromFile(FILE* toread,char *line,unsigned long size){
    if(fgets(line, size-1, toread)){
        line[strlen(line)-1] = 0;
        return 1;
    }
    return 0;
}

