/* 
 * File:   segutilities.c
 * Author: machiry
 *
 * Created on April 6, 2013, 1:24 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h> 
#include <string.h>
#include <fcntl.h>
#include "rvmprivate.h"
#include "utilities.h"

int isSegmentExists(const char *directory, const char *segment, unsigned long *size) {
    int toRet = 0;
    char* segInfoFileName = combinePaths(directory, SEGINFO_FILE);
    struct stat fileInfo;
    if (!stat(segInfoFileName, &fileInfo)) {
        FILE* fd = fopen(segInfoFileName, "r");
        if (fd) {
            char line[1024];
            while (readLineFromFile(fd,line,sizeof(line))) {
                if (startsWith(SEGNAMESTR, line) && !strcmp(line + strlen(SEGNAMESTR), segment)) {
                    //We found the segment, read the size
                    toRet = 1;
                    readLineFromFile(fd,line,sizeof(line));
                    *size = atol(line + strlen(SEGSIZESTR));
                    break;
                }
            }
            fclose(fd);
        } else {
            fprintf(stderr, "Corrupted File:Problem occured while trying to open seg info file:%s\n", segInfoFileName);
            remove(segInfoFileName);
            createFile(segInfoFileName);
        }

    } else {
        createFile(segInfoFileName);
    }
    free(segInfoFileName);
    return toRet;
}

void createNewSegment(const char *directory, const char *segment, unsigned long size, int extendSegment) {
    char* segInfoFileName = combinePaths(directory, SEGINFO_FILE);
    FILE* fd = fopen(segInfoFileName, "r");
    if (fd) {
        char line[1024];
        char *tempFileName = combinePaths(directory, "lrvmSEGINFO""tmp");
        FILE* tempFIleFd = fopen(tempFileName, "w");
        while (readLineFromFile(fd,line,sizeof(line))) {
            if (startsWith(SEGNAMESTR, line) && !strcmp(line + strlen(SEGNAMESTR), segment)) {
                readLineFromFile(fd,line,sizeof(line));
                fprintf(tempFIleFd, "%s%s\n", SEGNAMESTR, segment);
                sprintf(line, "%s%lu", SEGSIZESTR, size);
                fprintf(tempFIleFd, "%s\n", line);
            } else {
                fprintf(tempFIleFd, "%s\n", line);
            }
        }
        if (!extendSegment) {
            fprintf(tempFIleFd, "%s%s\n", SEGNAMESTR, segment);
            sprintf(line, "%s%lu", SEGSIZESTR, size);
            fprintf(tempFIleFd, "%s\n", line);
        }
        fclose(tempFIleFd);
        fclose(fd);
        copyFile(tempFileName, segInfoFileName);
        deleteFile(tempFileName);
        free(tempFileName);

    } else {
        fprintf(stderr, "STDERR:unable to read seg info file\n");
        abort();
    }

    free(segInfoFileName);
    struct stat fileInfo;
    char* targetSegmentFile = combinePaths(directory, segment);
    if (extendSegment && !stat(targetSegmentFile, &fileInfo)) {
        char tempSegFileName[1024];
        sprintf(tempSegFileName, "%s/%s%s%s", directory, SEGINFO_FILE, segment, "tmp");
        copyExtended(targetSegmentFile, tempSegFileName, size);
        copyFile(tempSegFileName, targetSegmentFile);
        deleteFile(tempSegFileName);
    } else {
        createNullFile(targetSegmentFile, size);
    }
    free(targetSegmentFile);
}

int isSegmentMapped(RVMINFO* currNode, const char* segname) {
    int to_ret = 0;
    if (currNode->segments) {
        SEGINFO* curr = currNode->segments;
        while (curr && strcmp(curr->name, segname)) {
            curr = curr->next;
        }
        to_ret = curr != NULL;
    }
    return to_ret;
}

SEGINFO* getSegmentByMappedBase(RVMINFO* currNode, void *base) {
    SEGINFO* to_ret = NULL;
    if (currNode->segments) {
        SEGINFO* curr = currNode->segments;
        while (curr && curr->mappedBase != base) {
            curr = curr->next;
        }
        to_ret = curr;
    }
    return to_ret;
}

void insertMappedSegment(RVMINFO* currNode, const char* segname, void* mappedBase, unsigned long size) {
    if (currNode) {
        SEGINFO* newNode = (SEGINFO*) malloc(sizeof (SEGINFO));
        if (newNode) {
            memset(newNode, 0, sizeof (SEGINFO));
            newNode->name = strdup(segname);
            newNode->mappedBase = mappedBase;
            newNode->size = size;
            newNode->next = currNode->segments;
            currNode->segments = newNode;
        } else {
            fprintf(stderr, "Insufficent memory:Unable to create a SEGINFO node\n");
            abort();
        }
    }
}

SEGINFO* isSegmentValid(TRANSINFO* currNode, void *segbase, int offset, int size) {
    SEGINFO* to_ret = 0;
    if (currNode) {
        int i = 0;
        for (i = 0; i < currNode->noOfMappedSegments; i++) {
            if (currNode->mappedSegments[i]->mappedBase == segbase && currNode->mappedSegments[i]->size >= offset + size) {
                to_ret = currNode->mappedSegments[i];
                break;
            }
        }
    }
    return to_ret;
}

int areSegsFree(RVMINFO* currNode, int numsegs, void **segbases) {
    int i = 0;
    for (i = 0; i < numsegs; i++) {
        SEGINFO* currSeg = getSegmentByMappedBase(currNode, segbases[i]);
        if (!currSeg || currSeg->isInTransaction) {
            return 0;
        }
    }
    return 1;
}

void unmapSegment(RVMINFO* currNode, SEGINFO* currSeg) {
    SEGINFO* parent = NULL;
    SEGINFO* current = currNode->segments;
    while (current && current != currSeg) {
        current = current->next;
    }
    if (parent) {
        parent->next = currSeg->next;
    } else {
        currNode->segments = currSeg->next;
    }
    free(currSeg->mappedBase);
    free(currSeg->name);
    free(currSeg);
}

void removeSegmentInfoInformation(RVMINFO* currNode, const char* segment) {
    char* segInfoFileName = combinePaths(currNode->directory, SEGINFO_FILE);
    FILE* fd = fopen(segInfoFileName, "r");
    if (fd) {
        char line[1024];
        char *tempFileName = combinePaths(currNode->directory, "lrvmSEGINFO""tmp");
        FILE* tempFIleFd = fopen(tempFileName, "w");
        while (readLineFromFile(fd,line,sizeof(line))) {
            if (startsWith(SEGNAMESTR, line) && !strcmp(line + strlen(SEGNAMESTR), segment)) {
                readLineFromFile(fd,line,sizeof(line));
            } else {
                fprintf(tempFIleFd, "%s\n", line);
            }
        }
        fclose(tempFIleFd);
        fclose(fd);
        copyFile(tempFileName, segInfoFileName);
        deleteFile(tempFileName);
        free(tempFileName);
        char* targetSegmentFile = combinePaths(currNode->directory, segment);

        //Delete the backing store
        deleteFile(targetSegmentFile);
        free(targetSegmentFile);
    }
    free(segInfoFileName);
}


