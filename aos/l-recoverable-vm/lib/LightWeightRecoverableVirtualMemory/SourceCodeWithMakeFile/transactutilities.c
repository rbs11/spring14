/* 
 * File:   transactutilities.c
 * Author: machiry
 *
 * Created on April 6, 2013, 1:31 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "rvmprivate.h"
#include "utilities.h"
#include <string.h>
#include "rvm.h"

TRANSINFO* getTransactionInfo(trans_t id) {
    TRANSINFO* current = transactions;
    while (current && current->id != id) {
        current = current->next;
    }
    return current;
}

int isModificationRecordPresent(TRANSINFO* currNode, void *segbase, int offset, int size) {
    int to_ret = 0;
    MODRECORD* modRecords = currNode->modificationRecords;
    while (modRecords) {
        if (modRecords->segmentUserBase == segbase && modRecords->offset == offset && modRecords->size == size) {
            to_ret = 1;
            break;
        }
        modRecords = modRecords->next;
    }
    return to_ret;
}

void addModificationRecord(TRANSINFO* currNode, void *segbase, int offset, int size) {
    MODRECORD* toadd = (MODRECORD*) malloc(sizeof (MODRECORD));
    if (!toadd) {
        fprintf(stderr, "Insufficient memory: unable to add modification record\n");
        abort();
    }
    toadd->next = currNode->modificationRecords;
    currNode->modificationRecords = toadd;
    toadd->offset = offset;
    toadd->size = size;
    toadd->segmentUserBase = segbase;
    toadd->backupRegion = (void*) malloc(size);
    memcpy(toadd->backupRegion, segbase + offset, size);

}

void freeTransactionInfo(TRANSINFO* currNode) {
    MODRECORD* to_delete = currNode->modificationRecords;
    MODRECORD* current_mod = to_delete;
    MODRECORD* temp = NULL;
    while (current_mod) {
        temp = current_mod->next;
        free(current_mod->backupRegion);
        free(current_mod);
        current_mod = temp;
    }
    currNode->modificationRecords = NULL;

    TRANSINFO* parent = NULL;
    TRANSINFO* current_trans = transactions;
    while (current_trans && current_trans != currNode) {
        parent = current_trans;
        current_trans = current_trans->next;
    }

    if (parent) {
        parent->next = currNode->next;
    } else {
        transactions = currNode->next;
    }

    int i = 0;
    for (i = 0; i < currNode->noOfMappedSegments; i++) {
        currNode->mappedSegments[i]->isInTransaction = 0;
        currNode->mappedSegments[i]->transactionID = 0;
    }

    free(currNode->mappedSegments);
    free(currNode);
}

