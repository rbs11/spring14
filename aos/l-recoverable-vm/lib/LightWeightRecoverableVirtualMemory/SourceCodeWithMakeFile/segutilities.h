/* 
 * File:   segutilities.h
 * Author: machiry
 *
 * Created on April 6, 2013, 1:25 PM
 */

#ifndef SEGUTILITIES_H
#define	SEGUTILITIES_H

#ifdef	__cplusplus
extern "C" {
#endif
#include "rvmprivate.h"
int isSegmentExists(const char *directory, const char *segment, unsigned long *size);
void createNewSegment(const char *directory, const char *segment, unsigned long size, int extendSegment);
int isSegmentMapped(RVMINFO* currNode, const char* segname);
SEGINFO* getSegmentByMappedBase(RVMINFO* currNode, void *base);
void insertMappedSegment(RVMINFO* currNode, const char* segname, void* mappedBase, unsigned long size);
SEGINFO* isSegmentValid(TRANSINFO* currNode, void *segbase, int offset, int size);
int areSegsFree(RVMINFO* currNode, int numsegs, void **segbases);
void unmapSegment(RVMINFO* currNode, SEGINFO* currSeg);
void removeSegmentInfoInformation(RVMINFO* currNode, const char* segment);


#ifdef	__cplusplus
}
#endif

#endif	/* SEGUTILITIES_H */

