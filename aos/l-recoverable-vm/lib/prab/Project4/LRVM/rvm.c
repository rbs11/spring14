/* 
 * File:   rvm.c
 * Author: machiry
 *
 * Created on April 5, 2013, 5:19 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h> 
#include <fcntl.h>
#include <rpc/pmap_clnt.h>
#include <sys/mman.h>
#include "rvm.h"
#include "rvmprivate.h"
#include "utilities.h"
#include "segutilities.h"
#include "transactutilities.h"

RVMINFO* rvmInfoList;
TRANSINFO* transactions;

//RVM Structure editing functions

static int isRVMPresent(const char* directory, rvm_t* toret) {
    if (rvmInfoList) {
        RVMINFO* curr = rvmInfoList;
        while (curr) {
            if (!strcmp(directory, curr->directory)) {
                *toret = curr->rvm_t;
                return 1;
            }
            curr = curr->next;
        }
    }
    return 0;
}

static RVMINFO* getRVMINFONode(rvm_t target) {
    if (rvmInfoList) {
        RVMINFO* curr = rvmInfoList;
        while (curr && curr->rvm_t != target) {
            curr = curr->next;
        }
        return curr;
    }
    return NULL;
}

static void insertIntoRVMList(RVMINFO* toInsert) {
    if (toInsert) {
        toInsert->next = rvmInfoList;
        rvmInfoList = toInsert;
    }
}

static void freeRVMINFONode(RVMINFO* toDelete){
    if(toDelete && rvmInfoList){    
        char command[1024];
        //Unlink RVMINFO node
        RVMINFO* parent = NULL;
        RVMINFO* curr = rvmInfoList;
        while(curr && curr != toDelete){
            parent = curr;
            curr = curr->next;
        }
        if(parent){
            parent->next = curr->next;
        } else{
            rvmInfoList = curr->next;
        }
        
        //Remove all Transaction info of the rvm
        TRANSINFO* currTransaction = transactions;
        TRANSINFO* tempt;
        while(currTransaction){
            tempt = currTransaction->next;
            if(currTransaction->targetRVM == toDelete){
                freeTransactionInfo(currTransaction);
            }
            currTransaction = tempt;
        }
        
        //Remove all segments from the rvm
        SEGINFO* current = toDelete->segments;
        SEGINFO* temp;
        while(current){
            temp = current->next;
            unmapSegment(toDelete,current);
            current = temp;
        }
        
        //Close the log file
        if(toDelete->logFileFD){
            fclose(toDelete->logFileFD);
            toDelete->logFileFD = NULL;
        }
        sprintf(command,"rm -rf %s/*",toDelete->directory);
        free(toDelete->logFilePath);
        free(toDelete->directory);
        free(toDelete);
        system(command);
    }
}

//Log Editing function

static void quarentineLogEntiresForSegment(RVMINFO* currNode, const char* segment, void* writeTo) {
    if (currNode && segment) {
        if (currNode->logFileFD) {
            fflush(currNode->logFileFD);
            fclose(currNode->logFileFD);
            currNode->logFileFD = NULL;
        }
        FILE* flogFile = fopen(currNode->logFilePath, "r");
        char tempLogFileName[1024];
        sprintf(tempLogFileName, "%s/%s%s", currNode->directory, LOGNAME_PREFIX, "tmp");
        FILE* ftempLogFile = fopen(tempLogFileName, "w");
        char line[1024];
        char transactionHeader[1024];
        int transactionHeaderWritten = 0;
        while (readLineFromFile(flogFile, line, sizeof (line))) {
            if (startsWith(TRANSSTART, line)) {
                strcpy(transactionHeader, line);
                transactionHeaderWritten = 0;
                while (readLineFromFile(flogFile, line, sizeof (line))) {
                    if (!startsWith(TRANSEND, line)) {
                        if (startsWith(SEGNAMESTR, line)) {
                            if (!strcmp(line + strlen(SEGNAMESTR), segment)) {
                                readLineFromFile(flogFile, line, sizeof (line));
                                unsigned long segOffset = atol(line + strlen(SEGOFFSET));
                                readLineFromFile(flogFile, line, sizeof (line));
                                unsigned long noOfBytesToWrite = atol(line + strlen(DIFFSIZE));
                                writeBytesToMemory(flogFile, (writeTo ? (writeTo + segOffset) : NULL), noOfBytesToWrite);
                            } else {
                                if (!transactionHeaderWritten) {
                                    fprintf(ftempLogFile, "%s\n", transactionHeader);
                                    transactionHeaderWritten = 1;
                                }
                                fprintf(ftempLogFile, "%s\n", line);
                                readLineFromFile(flogFile, line, sizeof (line));
                                fprintf(ftempLogFile, "%s\n", line);
                                unsigned long segOffset = atol(line + strlen(SEGOFFSET));
                                readLineFromFile(flogFile, line, sizeof (line));
                                fprintf(ftempLogFile, "%s\n", line);
                                unsigned long noOfBytesToWrite = atol(line + strlen(DIFFSIZE));
                                writeBytesToFile(flogFile, ftempLogFile, noOfBytesToWrite);
                            }
                        } else {
                            if (!transactionHeaderWritten) {
                                fprintf(ftempLogFile, "%s\n", transactionHeader);
                                transactionHeaderWritten = 1;
                            }
                            if (transactionHeaderWritten) {
                                fprintf(ftempLogFile, "%s\n", line);
                            }
                        }
                    } else {
                        if (transactionHeaderWritten) {
                            fprintf(ftempLogFile, "%s\n", line);
                        }
                    }
                }
            } else {
                fprintf(ftempLogFile, "%s\n", line);
            }
        }
        fclose(flogFile);
        fclose(ftempLogFile);
        copyFile(tempLogFileName, currNode->logFilePath);
        deleteFile(tempLogFileName);
    }
}

void* getMappedTruncateRegion(SEGTRUNCATE* freeSegments, char *segmentname, char *directory) {
    while (freeSegments && strcmp(freeSegments->name, segmentname)) {
        freeSegments = freeSegments->next;
    }
    if (freeSegments && !freeSegments->mappedBase) {
        char* targetSegmentFile = combinePaths(directory, segmentname);
        int fd = open(targetSegmentFile, O_RDWR);
        if (fd > 0) {
            freeSegments->mappedBase = mmap(NULL, freeSegments->size, PROT_WRITE, MAP_SHARED, fd, 0);
            freeSegments->fd = fd;
        }
        free(targetSegmentFile);
    }
    return freeSegments ? freeSegments->mappedBase : NULL;
}

static TRANSINFO* getTransactionByMappedBase(SEGINFO *mappedBase) {
    TRANSINFO* curr = transactions;
    while (curr) {
        int i = 0;
        for (i = 0; i < curr->noOfMappedSegments; i++) {
            if (curr->mappedSegments[i] == mappedBase) {
                return curr;
            }
        }
        curr = curr->next;
    }
    return NULL;
}

static void removeSegbaseFromTransaction(TRANSINFO* currInfo, SEGINFO *mappedBase) {
    if (currInfo->noOfMappedSegments <= 1) {
        freeTransactionInfo(currInfo);
    } else {
        int i = 0;
        int j = 0;
        i = 0;
        j = 0;
        for (i = 0; i < currInfo->noOfMappedSegments; i++) {
            if (currInfo->mappedSegments[i] != mappedBase) {
                currInfo->mappedSegments[j++] = currInfo->mappedSegments[i];
            }
        }
        currInfo->mappedSegments[j++] = NULL;
        currInfo->noOfMappedSegments = currInfo->noOfMappedSegments - 1;

        MODRECORD* modRecords = currInfo->modificationRecords;
        MODRECORD* temp = NULL;
        MODRECORD* parent = NULL;
        //Remove all the modification records of the mapped region
        while (modRecords) {
            temp = modRecords->next;
            if (modRecords->segmentUserBase != mappedBase->mappedBase) {
                parent = modRecords;
            } else {
                free(modRecords->backupRegion);
                free(modRecords);
                if (parent) {
                    parent->next = temp;
                } else {
                    currInfo->modificationRecords = temp;
                }
            }
            modRecords = temp;
        }

    }

}

//Library Functions

rvm_t rvm_init(const char *directory) {
    rvm_t toret;
    struct stat dirInfo;
    if (directory) {
        int directoryExists = 0;


        //1. If we have already initialized with this directory then delete the directory
        if (isRVMPresent(directory, &toret)) {
            freeRVMINFONode(getRVMINFONode(toret));
        }

        //2. Check if there exists a file with this name and its a directory
        if (!stat(directory, &dirInfo)) {
            directoryExists = 1;
            if (!S_ISDIR(dirInfo.st_mode)) {
                fprintf(stderr, "rvm_init:File with name:%s, already exists\n", directory);
                abort();
            }
        }


        //3. Create a new RVMINFO node and store it
        RVMINFO* toInsert = (RVMINFO*) malloc(sizeof (RVMINFO));
        if (!toInsert) {
            inSufficientMemory("in rvm_init");
        }

        //4. Initialization of the structures
        memset(toInsert, 0, sizeof (RVMINFO));
        toInsert->directory = strdup(directory);
        if (!(toInsert->directory)) {
            inSufficientMemory("in rvm_init");
        }
        toInsert->logFilePath = combinePaths(toInsert->directory, LOGNAME_PREFIX);
        if (!(toInsert->logFilePath)) {
            inSufficientMemory("in rvm_init");
        }
        toInsert->rvm_t = getNextRandomID();
        toret = toInsert->rvm_t;

        //5. Creating the directory
        if (!directoryExists) {
            if (mkdir(directory, 0777)) {
                fprintf(stderr, "rvm_init: Problem occured while trying to create the directory provided\n");
                abort();
            }
        }

        //6. Creating the log file
        if (!isFileExists(directory, LOGNAME_PREFIX)) {
            createFile(toInsert->logFilePath);
        }

        //7. Insert into RVM List
        insertIntoRVMList(toInsert);
    } else {
        fprintf(stderr, "Invalid directory for rvm_init\n");
        abort();
    }
    return toret;
}

void *rvm_map(rvm_t rvm, const char *segname, int size_to_create) {
    void *to_ret = NULL;
    if (!segname || startsWith(LOGNAME_PREFIX, segname) || startsWith(SEGINFO_FILE, segname)) {
        fprintf(stderr, "rvm_map:Invalid segment file name, lrvm uses these to store its own metadata\n");
    } else {
        RVMINFO* currNode = getRVMINFONode(rvm);
        unsigned long currSegSize = 0;
        if (currNode && !isSegmentMapped(currNode, segname)) {
            if (isSegmentExists(currNode->directory, segname, &currSegSize)) {
                if (currSegSize > size_to_create) {
                    //Wrong: segment exists and the size is MORE than the requested size
                    //fprintf(stderr, "rvm_map:provided size is less then the actual size, it can be same or more\n");
                    size_to_create = currSegSize;
                } else if (currSegSize == size_to_create) {
                    //Best case: segment exists and the size is same as the stored size
                    //don't do any thing
                } else {
                    //Worst case: segment exists and the size is less than the requested size
                    createNewSegment(currNode->directory, segname, size_to_create, 1);
                }

            } else {
                createNewSegment(currNode->directory, segname, size_to_create, 0);
            }

            //Map the segment file to the memory.
            //Read log file for any diffs, apply them to the mapped memory
            //After applying the diffs, copy the new modified region to the malloced region
            //Unmap the segment file
            char* targetSegmentFile = combinePaths(currNode->directory, segname);
            int fd = open(targetSegmentFile, O_RDWR);
            if (fd > 0) {
                void* copiedContents = mmap(NULL, size_to_create, PROT_WRITE, MAP_SHARED, fd, 0);
                /*if (currNode->logFileFD) {
                    fclose(currNode->logFileFD);
                    currNode->logFileFD = NULL;
                }
                FILE* flogFile = fopen(currNode->logFilePath, "r");
                char tempLogFileName[1024];
                sprintf(tempLogFileName, "%s/%s%s", currNode->directory, LOGNAME_PREFIX, "tmp");
                FILE* ftempLogFile = fopen(tempLogFileName, "w");
                char line[1024];
                int transactionHeaderWritten = 0;
                char transactionHeader[1024];
                while (readLineFromFile(flogFile, line, sizeof (line))) {
                    if (startsWith(TRANSSTART, line)) {
                        transactionHeaderWritten = 0;
                        strcpy(transactionHeader, line);
                        while (readLineFromFile(flogFile, line, sizeof (line))) {
                            if (!startsWith(TRANSEND, line)) {
                                if (startsWith(SEGNAMESTR, line)) {
                                    if (!strcmp(line + strlen(SEGNAMESTR), segname)) {
                                        readLineFromFile(flogFile, line, sizeof (line));
                                        unsigned long segOffset = atol(line + strlen(SEGOFFSET));
                                        readLineFromFile(flogFile, line, sizeof (line));
                                        unsigned long noOfBytesToWrite = atol(line + strlen(DIFFSIZE));
                                        writeBytesToMemory(flogFile, copiedContents + segOffset, noOfBytesToWrite);
                                    } else {
                                        if (!transactionHeaderWritten) {
                                            fprintf(ftempLogFile, "%s\n", transactionHeader);
                                            transactionHeaderWritten = 1;
                                        }
                                        fprintf(ftempLogFile, "%s\n", line);
                                        readLineFromFile(flogFile, line, sizeof (line));
                                        fprintf(ftempLogFile, "%s\n", line);
                                        readLineFromFile(flogFile, line, sizeof (line));
                                        fprintf(ftempLogFile, "%s\n", line);
                                        unsigned long noOfBytesToWrite = atol(line + strlen(DIFFSIZE));
                                        writeBytesToFile(flogFile, ftempLogFile, noOfBytesToWrite);
                                    }
                                } else {
                                    if (!transactionHeaderWritten) {
                                        fprintf(ftempLogFile, "%s\n", transactionHeader);
                                        transactionHeaderWritten = 1;
                                    }
                                    fprintf(ftempLogFile, "%s\n", line);
                                }
                            } else {
                                if (transactionHeaderWritten) {
                                    fprintf(ftempLogFile, "%s\n", line);
                                }
                            }

                        }
                    } else {
                        fprintf(ftempLogFile, "%s\n", line);
                    }

                }

                fclose(flogFile);
                fclose(ftempLogFile);
                copyFile(tempLogFileName, currNode->logFilePath);
                deleteFile(tempLogFileName);*/
                quarentineLogEntiresForSegment(currNode, segname, copiedContents);
                void* newSegBase = malloc(size_to_create);
                if (!newSegBase) {
                    fprintf(stderr, "rvm_map:In sufficient memory\n");
                    abort();
                }
                to_ret = newSegBase;
                memcpy(newSegBase, copiedContents, size_to_create);
                munmap(copiedContents, size_to_create);
                close(fd);
                insertMappedSegment(currNode, segname, to_ret, size_to_create);
            }
            free(targetSegmentFile);
        }
    }
    return to_ret;
}

void rvm_destroy(rvm_t rvm, const char *segname) {
    RVMINFO* currNode = getRVMINFONode(rvm);
    if (currNode && segname) {
        if (isSegmentMapped(currNode, segname)) {
            fprintf(stderr, "rvm_destroy:Segment:%s is currently mapped and cannot be destroyed\n", segname);
            abort();
        }
        quarentineLogEntiresForSegment(currNode, segname, NULL);
        removeSegmentInfoInformation(currNode, segname);
    }
}

void rvm_unmap(rvm_t rvm, void *segbase) {
    RVMINFO* currNode = getRVMINFONode(rvm);
    if (currNode && segbase) {
        SEGINFO* targetSegInfo = getSegmentByMappedBase(currNode, segbase);
        if (targetSegInfo) {
            if (targetSegInfo->isInTransaction) {
                TRANSINFO* targetTrans = getTransactionByMappedBase(targetSegInfo);
                while (targetTrans) {
                    removeSegbaseFromTransaction(targetTrans, targetSegInfo);
                    targetTrans = getTransactionByMappedBase(targetSegInfo);
                }
            }
            unmapSegment(currNode, targetSegInfo);
        }
    }
}

trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases) {
    RVMINFO* currNode = getRVMINFONode(rvm);
    int i = 0;
    if (currNode && numsegs > 0 && segbases && areSegsFree(currNode, numsegs, segbases)) {
        TRANSINFO* currTrans = (TRANSINFO*) malloc(sizeof (TRANSINFO));
        currTrans->id = getNextRandomID();
        currTrans->next = transactions;
        currTrans->targetRVM = currNode;
        currTrans->noOfMappedSegments = numsegs;
        currTrans->mappedSegments = (SEGINFO**) malloc(numsegs * sizeof (SEGINFO*));
        for (i = 0; i < numsegs; i++) {
            SEGINFO* currSegNode = getSegmentByMappedBase(currNode, segbases[i]);
            currSegNode->isInTransaction = 1;
            currSegNode->transactionID = currTrans->id;
            currTrans->mappedSegments[i] = currSegNode;
        }
        currTrans->modificationRecords = NULL;
        transactions = currTrans;
        return currTrans->id;
    }
    return -1;
}

void rvm_about_to_modify(trans_t tid, void *segbase, int offset, int size) {
    TRANSINFO* currTransInfo = getTransactionInfo(tid);
    if (segbase && currTransInfo && isSegmentValid(currTransInfo, segbase, offset, size) && !isModificationRecordPresent(currTransInfo, segbase, offset, size)) {
        addModificationRecord(currTransInfo, segbase, offset, size);
    }
}

void rvm_commit_trans(trans_t tid) {
    TRANSINFO* currTransInfo = getTransactionInfo(tid);
    if (currTransInfo) {
        if (!currTransInfo->targetRVM->logFileFD) {
            currTransInfo->targetRVM->logFileFD = fopen(currTransInfo->targetRVM->logFilePath, "a");
        }
        if (currTransInfo->modificationRecords) {
            FILE* logFD = currTransInfo->targetRVM->logFileFD;
            char line[1024];
            sprintf(line, "%s%lu", TRANSSTART, currTransInfo->id);
            fprintf(logFD, "%s\n", line);
            MODRECORD* currModRec = currTransInfo->modificationRecords;
            while (currModRec) {
                SEGINFO* currSegInfo = getSegmentByMappedBase(currTransInfo->targetRVM, currModRec->segmentUserBase);
                fprintf(logFD, "%s%s\n", SEGNAMESTR, currSegInfo->name);
                sprintf(line, "%s%lu", SEGOFFSET, currModRec->offset);
                fprintf(logFD, "%s\n", line);
                sprintf(line, "%s%lu", DIFFSIZE, currModRec->size);
                fprintf(logFD, "%s\n", line);
                writeBytesToFileFromMemory(logFD, currModRec->segmentUserBase + currModRec->offset, currModRec->size);
                currModRec = currModRec->next;
            }
            sprintf(line, "%s", TRANSEND);
            fprintf(logFD, "%s\n", line);
            fflush(logFD);
        }
        if (currTransInfo->targetRVM->logFileFD) {
            fclose(currTransInfo->targetRVM->logFileFD);
            currTransInfo->targetRVM->logFileFD = NULL;
        }
        freeTransactionInfo(currTransInfo);
    }
}

void rvm_abort_trans(trans_t tid) {
    TRANSINFO* currTransInfo = getTransactionInfo(tid);
    if (currTransInfo) {
        MODRECORD* currRecord = currTransInfo->modificationRecords;
        while (currRecord) {
            //Restore the memeory from backed up region
            memcpy(currRecord->segmentUserBase + currRecord->offset, currRecord->backupRegion, currRecord->size);
            currRecord = currRecord->next;
        }
        freeTransactionInfo(currTransInfo);
    }
}

void rvm_truncate_log(rvm_t rvm) {
    RVMINFO* currNode = getRVMINFONode(rvm);
    if (currNode) {
        SEGTRUNCATE* freeSegments = NULL;
        char currSegmentName[1024];
        char* segInfoFileName = combinePaths(currNode->directory, SEGINFO_FILE);
        struct stat fileInfo;
        if (!stat(segInfoFileName, &fileInfo)) {
            FILE* fd = fopen(segInfoFileName, "r");
            if (fd) {
                char line[1024];
                while (readLineFromFile(fd, line, sizeof (line))) {
                    if (startsWith(SEGNAMESTR, line)) {
                        strcpy(currSegmentName, line + strlen(SEGNAMESTR));
                        readLineFromFile(fd, line, sizeof (line));
                        long size = atol(line + strlen(SEGSIZESTR));
                        //if (!isSegmentMapped(currNode, currSegmentName)) {
                            SEGTRUNCATE* toAppend = (SEGTRUNCATE*) malloc(sizeof (SEGTRUNCATE));
                            toAppend->name = strdup(currSegmentName);
                            toAppend->size = size;
                            toAppend->mappedBase = NULL;
                            toAppend->next = freeSegments;
                            freeSegments = toAppend;
                        //}
                    }
                }
                fclose(fd);
            }

        }
        free(segInfoFileName);

        if (freeSegments) {
            FILE* flogFile = fopen(currNode->logFilePath, "r");
            char tempLogFileName[1024];
            sprintf(tempLogFileName, "%s/%s%s", currNode->directory, LOGNAME_PREFIX, "tmp");
            FILE* ftempLogFile = fopen(tempLogFileName, "w");
            char line[1024];
            char transactionHeader[1024];
            int transactionHeaderWritten = 0;
            while (readLineFromFile(flogFile, line, sizeof (line))) {
                if (startsWith(TRANSSTART, line)) {
                    strcpy(transactionHeader, line);
                    transactionHeaderWritten = 0;
                    while (readLineFromFile(flogFile, line, sizeof (line))) {
                        if (!startsWith(TRANSEND, line)) {
                            if (startsWith(SEGNAMESTR, line)) {
                                strcpy(currSegmentName, line + strlen(SEGNAMESTR));
                                if (1/*!isSegmentMapped(currNode, currSegmentName)*/) {
                                    readLineFromFile(flogFile, line, sizeof (line));
                                    unsigned long segOffset = atol(line + strlen(SEGOFFSET));
                                    readLineFromFile(flogFile, line, sizeof (line));
                                    unsigned long noOfBytesToWrite = atol(line + strlen(DIFFSIZE));
                                    writeBytesToMemory(flogFile, getMappedTruncateRegion(freeSegments, currSegmentName, currNode->directory) + segOffset, noOfBytesToWrite);
                                } else {
                                    if (!transactionHeaderWritten) {
                                        fprintf(ftempLogFile, "%s\n", transactionHeader);
                                        transactionHeaderWritten = 1;
                                    }
                                    fprintf(ftempLogFile, "%s\n", line);
                                    readLineFromFile(flogFile, line, sizeof (line));
                                    fprintf(ftempLogFile, "%s\n", line);
                                    unsigned long segOffset = atol(line + strlen(SEGOFFSET));
                                    readLineFromFile(flogFile, line, sizeof (line));
                                    fprintf(ftempLogFile, "%s\n", line);
                                    unsigned long noOfBytesToWrite = atol(line + strlen(DIFFSIZE));
                                    writeBytesToFile(flogFile, ftempLogFile, noOfBytesToWrite);
                                }
                            } else {
                                if (!transactionHeaderWritten) {
                                    fprintf(ftempLogFile, "%s\n", transactionHeader);
                                    transactionHeaderWritten = 1;
                                }
                                fprintf(ftempLogFile, "%s\n", line);
                            }

                        } else {
                            if (transactionHeaderWritten) {
                                fprintf(ftempLogFile, "%s\n", line);
                            }
                        }
                    }
                } else {
                    fprintf(ftempLogFile, "%s\n", line);
                }

            }

            fclose(flogFile);
            fclose(ftempLogFile);
            copyFile(tempLogFileName, currNode->logFilePath);
            deleteFile(tempLogFileName);


            SEGTRUNCATE* temp;
            while (freeSegments) {
                temp = freeSegments->next;
                if (freeSegments->mappedBase) {
                    munmap(freeSegments->mappedBase, freeSegments->size);
                    close(freeSegments->fd);
                }
                free(freeSegments->name);
                free(freeSegments);
                freeSegments = temp;
            }
        }
    }
}
