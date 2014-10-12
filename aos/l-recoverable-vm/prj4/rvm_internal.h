

#ifndef RVM_INTERNAL_H
#define RVM_INTERNAL_H

#include <stdint.h>


#define MAXFILENAMESIZE	8
typedef int32_t rvm_t;
typedef int32_t trans_t;

typedef struct SegmentList{
	void* SegBase;
	char *SegName;
	uint32_t SegSize;
	uint32_t Busy;
	uint32_t SegID;
	trans_t TID;
	struct SegmentList *next;
}SegList_t;

typedef struct MappedSegments{
	rvm_t RVM_Id;
	char *DirName;
	SegList_t *Seg_list;
	uint32_t nSegs;	//Number of mapped segments
}RVM_Metadata_t;

typedef struct UndoRecords{
	void* dummy1;
	int dummy2;
	void* SegBase;
	void *buf;
	uint32_t Offset;
	uint32_t nBytes;
	struct UndoRecords *next;
}UndoRecord_t;

typedef struct RedoLogs{
	char *SegName;
	void *buf;
	uint32_t Offset;
	uint32_t nBytes;
	struct RedoLogs *next;
}RedoLogs_t;

typedef struct Transactions{
	trans_t TID;
	RVM_Metadata_t *RVM_Metadata;
	UndoRecord_t *UndoRecord;
	RedoLogs_t *RedoLog;
	uint32_t nModifyRegions;
	struct Transactions *next;
}Transactions_t;























#endif
