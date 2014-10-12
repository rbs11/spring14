/* 
 * File:   transactutilities.h
 * Author: machiry
 *
 * Created on April 6, 2013, 1:31 PM
 */

#ifndef TRANSACTUTILITIES_H
#define	TRANSACTUTILITIES_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "rvmprivate.h"
    
TRANSINFO* getTransactionInfo(trans_t id);
int isModificationRecordPresent(TRANSINFO* currNode, void *segbase, int offset, int size);
void addModificationRecord(TRANSINFO* currNode, void *segbase, int offset, int size);
void freeTransactionInfo(TRANSINFO* currNode);


#ifdef	__cplusplus
}
#endif

#endif	/* TRANSACTUTILITIES_H */

