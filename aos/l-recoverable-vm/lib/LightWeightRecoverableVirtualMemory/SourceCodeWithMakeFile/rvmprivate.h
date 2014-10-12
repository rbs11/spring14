/* 
 * File:   rvmprivate.h
 * Author: machiry
 *
 * Created on April 5, 2013, 1:25 PM
 */

#ifndef RVMPRIVATE_H
#define	RVMPRIVATE_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#define LOGNAME_PREFIX "lrvmlog"
#define SEGINFO_FILE "lrvmSEGINFO"
#define SEGNAMESTR "SEGNAME:"
#define SEGSIZESTR "SEGSIZE:"
#define SEGOFFSET "OFFSET:"
#define DIFFSIZE "DIFFS:"
#define TRANSSTART "STARTTRANSACT:"
#define TRANSEND "ENDTRANSACT"

     typedef struct SegmentsInfo{
        char *name;
        unsigned long size;
        void *mappedBase;
        int isInTransaction;
        unsigned long transactionID;
        struct SegmentsInfo* next;
    } SEGINFO;
    
    typedef struct TruncateStruct{
        char *name;
        unsigned long size;
        void *mappedBase;
        int fd;
        struct TruncateStruct* next;
    } SEGTRUNCATE;
    
    typedef struct ModRecord{
        char *segmentUserBase;
        unsigned long offset;
        unsigned long size;
        char *backupRegion;
        struct ModRecord *next;
    } MODRECORD;
    
    typedef struct RvmInfo{
        unsigned long rvm_t;
        char *directory;
        char *logFilePath;
        FILE* logFileFD;
        SEGINFO* segments;
        struct RvmInfo *next;
    } RVMINFO;
    
    typedef struct TransactionInfo{
        unsigned long id;
        unsigned long noOfMappedSegments;
        MODRECORD* modificationRecords;
        RVMINFO* targetRVM;
        struct TransactionInfo *next;        
        SEGINFO **mappedSegments;
    } TRANSINFO;
    
    
    extern RVMINFO* rvmInfoList;
    extern TRANSINFO* transactions;
#ifdef	__cplusplus
}
#endif

#endif	/* RVMPRIVATE_H */

