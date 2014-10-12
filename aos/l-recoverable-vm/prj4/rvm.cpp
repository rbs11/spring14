#include "rvm.h"
#include "rvm_internal.h"
#include <unistd.h>
#include <stdio.h>
#include <stddef.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

RVM_Metadata_t *gRVMMetadata;
Transactions_t *gTransList;
rvm_t g_rvmID = 0;
trans_t g_transID = 0;

//#define DEBUG
//#define ERROR
#ifdef DEBUG
#define	print_dbg(fmt, arg...) 					\
do {											\
	printf("%s Line %d:", __FILE__, __LINE__);	\
	printf(fmt, ## arg);					\
	printf("\n");							\
} while (0);
#else
#define	print_dbg(fmt, arg...)
#endif
#ifdef ERROR
#define	print_err(fmt, arg...) 					\
do {											\
	printf("Error: %s Line %d:", __FILE__, __LINE__);	\
	printf(fmt, ## arg);					\
	printf("\n");							\
} while (0);
#else
#define	print_err(fmt, arg...)
#endif

rvm_t rvm_init(const char *directory) {
	struct stat dirstats;
	int32_t errno;
	bool bDirCreated = 0;
	if (!stat(directory, &dirstats)) {
		print_dbg("Directory Already Exists");
		bDirCreated = 1;
	}

	gRVMMetadata = (RVM_Metadata_t*) malloc(sizeof(RVM_Metadata_t));
	gRVMMetadata->DirName = strdup(directory);
	if (!bDirCreated) {
		if ((errno = mkdir(directory, 0777))) {
			print_err("Error[%d] while creating Directory\n", errno);
		}
	}
	gRVMMetadata->RVM_Id = ++g_rvmID;
	if (g_rvmID > 1) {
		print_err("Init being called more than once. This condition is not required to be handled.\n");
		return -1;
	}

	return gRVMMetadata->RVM_Id;

}

void *rvm_map(rvm_t rvm, const char *segname, int size_to_create) {
	bool bSegFound;
	int fd = -1;
	struct stat filestats;
	char segpath[100];

	if (gRVMMetadata->RVM_Id != rvm) {
		print_err("Invalid RVM for this request");
		return NULL;
	}

	sprintf(segpath, "%s/%s", gRVMMetadata->DirName, segname);
	print_dbg("%s\n", segpath);
	/*Check if segment already exists on disk*/
	if (!stat(segpath, &filestats)) {
		print_dbg("File Already Exists");
		fd = open(segpath, O_RDWR, 0777);
		/*If so, check if size is the same as of that requested*/
		if (filestats.st_size < size_to_create) {
			print_dbg("File on disk is smaller than requested size");
			if (truncate(segpath, size_to_create)) {
				print_err("Failed to set file to size");
			}
		}
		bSegFound = 1;

	}
	/*If not create a fresh segment on disk and map it to memory*/
	if (!bSegFound) {
		print_dbg("Creating file under path %s", segpath);
		fd = open(segpath, O_CREAT | O_RDWR, 0777);
		if (fd < 0)
			print_err("Failed to create file");
		if (truncate(segpath, size_to_create)) {
			print_err("Failed to set file to size");
		}
	}
	if (fd < 0) {
		fd = open(segpath, O_RDWR, 0777);
		if (fd < 0) {
			print_err("Error Opening File");
			//return NULL;
		}
	}

	void *segbase = malloc(size_to_create);
	SegList_t *Seg_list_cur;
	int nSegs = 0;

	/*segbase = mmap(NULL, size_to_create, PROT_EXEC | PROT_READ | PROT_WRITE,
	 MAP_SHARED, fd, 0);*/

	read(fd, segbase, size_to_create);
	nSegs = gRVMMetadata->nSegs++;

	Seg_list_cur = gRVMMetadata->Seg_list;
	print_dbg("Total Segs[%d]", nSegs);
	/*Find the next free node*/
	if (Seg_list_cur != NULL) {
		while (Seg_list_cur->next != NULL) {
			Seg_list_cur = Seg_list_cur->next;
			nSegs--;
		}
		Seg_list_cur->next = (SegList_t*) malloc(sizeof(SegList_t));
		Seg_list_cur = Seg_list_cur->next;
	} else {
		gRVMMetadata->Seg_list = (SegList_t*) malloc(sizeof(SegList_t));
		Seg_list_cur = gRVMMetadata->Seg_list;
	}
	Seg_list_cur->SegBase = segbase;
	Seg_list_cur->SegID = gRVMMetadata->nSegs;
	Seg_list_cur->SegName = strdup((char*) segname);
	Seg_list_cur->SegSize = size_to_create;
	Seg_list_cur->next = NULL;
#if 1
	/*Apply redo changes*/
	char redo_filename[200], *buffer;
	uint32_t Offset, nBytes;
	FILE *redo_file;
	sprintf(redo_filename, "%s/REDO_LOG_%s", (char *) gRVMMetadata->DirName,
			Seg_list_cur->SegName);
	print_dbg("Opening redo file with name %s",redo_filename);
	redo_file = fopen(redo_filename, "r");
	if (redo_file == NULL) {
		print_err("Error: Failed to open Redo Log File");
	} else {
		while (!feof(redo_file)) {
			if (!fscanf(redo_file, "#REDO LOG#%s\n", Seg_list_cur->SegName))
				break;
			fscanf(redo_file, "%d %d\n", &Offset, &nBytes);
			buffer = (char *) malloc(nBytes);
			fread(buffer, nBytes, 1, redo_file);
			memcpy((char*)Seg_list_cur->SegBase + Offset, buffer, nBytes);
			fscanf(redo_file, "\n");
			if (buffer)
				free(buffer);
		}
	}
#endif
	close(fd);
	return segbase;
}

void rvm_unmap(rvm_t rvm, void *segbase) {
	if (gRVMMetadata->RVM_Id != rvm) {
		print_err("Invalid RVM for this request");
		return;
	}
	SegList_t *sglist_cur = gRVMMetadata->Seg_list;
	SegList_t *prev = NULL;
	int32_t bflag;
	while (sglist_cur != NULL) {
		if (sglist_cur->SegBase == segbase) {
			bflag = 1;
			print_dbg("Unmapping Segment %d", sglist_cur->SegID)
			//munmap(sglist_cur->SegBase, sglist_cur->SegSize);
			if (sglist_cur->SegBase)
				free(sglist_cur->SegBase);
			sglist_cur->SegBase = NULL;
			if (sglist_cur->next != NULL) {
				if (prev != NULL)
					prev->next = sglist_cur->next;
				else {
					if (gRVMMetadata->Seg_list != sglist_cur)
						print_err("Shouldn't happen");
					gRVMMetadata->Seg_list = sglist_cur->next;
				}
			}
			if (sglist_cur)
				free(sglist_cur);
			sglist_cur = NULL;

			break;
		}
		prev = sglist_cur;
		sglist_cur = sglist_cur->next;
	}
	if (!bflag) {
		print_err("Couldn't find segment base on list");
	}
}

void rvm_destroy(rvm_t rvm, const char *segname) {
	if (gRVMMetadata->RVM_Id != rvm) {
		print_err("Invalid RVM for this request");
		return;
	}
	SegList_t *sglist_cur = gRVMMetadata->Seg_list;
	SegList_t *prev = NULL;
	int32_t bflag;
	char cmd[500];

	while (sglist_cur != NULL) {
		if (strcmp(sglist_cur->SegName, (char*) segname) == 0) {
			bflag = 1;
			print_err("Destroy called before Unmap segment %s", segname);
			if (sglist_cur->SegBase)
				munmap(sglist_cur->SegBase, sglist_cur->SegSize);
			if (sglist_cur->next != NULL) {
				if (prev != NULL)
					prev->next = sglist_cur->next;
				else {
					if (gRVMMetadata->Seg_list != sglist_cur)
						print_err("Shouldn't happen");
					gRVMMetadata->Seg_list = sglist_cur->next;
				}
			}
			if (sglist_cur)
				free(sglist_cur);
			sglist_cur = NULL;
			break;
		}
		prev = sglist_cur;
		sglist_cur = sglist_cur->next;
	}
	if (!bflag) {
		print_err("Couldn't find segment base on list");
	}
	print_dbg("Issuing Command rm -f %s/%s", gRVMMetadata->DirName,segname);
	sprintf(cmd, "rm -f %s/%s", gRVMMetadata->DirName, segname);
	system(cmd);
	sprintf(cmd, "rm -f %s/REDO_LOG_%s", gRVMMetadata->DirName, segname);
	system(cmd);
}

trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases) {
	if (gRVMMetadata->RVM_Id != rvm) {
		print_err("Invalid RVM for this request");
		return -1;
	}
	SegList_t *sglist_cur = gRVMMetadata->Seg_list;
	int32_t bflags[numsegs];
	for (int i = 0; i < numsegs; i++) {
		while (sglist_cur != NULL) {
			if (sglist_cur->SegBase == segbases[i]) {
				if (sglist_cur->Busy == 1) {
					print_err("Transaction already active on this segment");
				} else {
					sglist_cur->Busy = 1;
					bflags[i] = 1;
					break;
				}
			}
			sglist_cur = sglist_cur->next;
		}
		if (!bflags[i]) {
			print_err("Couldn't find segment base on list");
		}
	}
	Transactions_t* curr_trans = gTransList;
	if (gTransList == NULL) {
		gTransList = (Transactions_t*) malloc(sizeof(Transactions_t));
		curr_trans = gTransList;
	} else {
		while (curr_trans->next != NULL) {
			curr_trans = curr_trans->next;
		}
		curr_trans->next = (Transactions_t*) malloc(sizeof(Transactions_t));
		curr_trans = curr_trans->next;
	}
	curr_trans->next = NULL;
	curr_trans->RVM_Metadata = gRVMMetadata;
	srand(1);
	curr_trans->TID = rand();

	return curr_trans->TID;
}

void rvm_about_to_modify(trans_t tid, void *segbase, int offset, int size) {
	Transactions_t* curr_trans = gTransList;
	bool bTransFound = 0;
	while (curr_trans != NULL) {
		if (tid == curr_trans->TID) {
			bTransFound = 1;
			break;
		}
		curr_trans = curr_trans->next;
	}
	if (!bTransFound) {
		print_err("Wrong transaction ID");
		return;
	}
	curr_trans->nModifyRegions++;
	/*Need to backup region to undo record in case of abort or failure*/
	/*Find free undo record*/
	UndoRecord_t *undo_cur = curr_trans->UndoRecord;
	if (curr_trans->UndoRecord == NULL) {
		curr_trans->UndoRecord = (UndoRecord_t*) malloc(sizeof(UndoRecord_t));
		undo_cur = curr_trans->UndoRecord;
	} else {
		while (undo_cur->next != NULL) {
			undo_cur = undo_cur->next;
		}
		undo_cur->next = (UndoRecord_t*) malloc(sizeof(UndoRecord_t));
		undo_cur = undo_cur->next;
	}
	undo_cur->Offset = offset;
	undo_cur->nBytes = size;

	undo_cur->SegBase = segbase;
	undo_cur->buf = malloc(size);
	undo_cur->next = NULL;

	/*Prepare redo logs*/
	RedoLogs_t *redo_cur = curr_trans->RedoLog;
	if (curr_trans->RedoLog == NULL) {
		curr_trans->RedoLog = (RedoLogs_t*) malloc(sizeof(RedoLogs_t));
		redo_cur = curr_trans->RedoLog;
	} else {
		while (redo_cur->next != NULL) {
			redo_cur = redo_cur->next;
		}
		redo_cur->next = (RedoLogs_t*) malloc(sizeof(RedoLogs_t));
		redo_cur = redo_cur->next;
	}
	/* Also update redo log with content */
	redo_cur->Offset = offset;
	redo_cur->nBytes = size;
	redo_cur->buf = ((char*) segbase + offset);
	redo_cur->next = NULL;

	/*Find the segment in the map and copy contents to undo record's buffer*/

	SegList_t *seg_cur = curr_trans->RVM_Metadata->Seg_list;
	bool bSegFound = 0;

	while (seg_cur != NULL) {
		if (seg_cur->SegBase == segbase) {
			memcpy(undo_cur->buf, (void*) ((char*) seg_cur->SegBase + offset),
					size);
			redo_cur->SegName = strdup(seg_cur->SegName);
			bSegFound = 1;
			break;
		}
		seg_cur = seg_cur->next;
	}
	if (!bSegFound) {
		print_err("Segment to modify not found in list");
		rvm_abort_trans(tid);
	}
	return;
}

void rvm_abort_trans(trans_t tid) {
	Transactions_t* curr_trans = gTransList;
	bool bTransFound = 0;
	while (curr_trans != NULL) {
		if (tid == curr_trans->TID) {
			bTransFound = 1;
			break;
		}
		curr_trans = curr_trans->next;
	}
	if (!bTransFound) {
		print_err("Wrong transaction ID");
		return;
	}
	/*Write all undo records to mapped memory segments*/
	UndoRecord_t *undo_cur = curr_trans->UndoRecord;
	UndoRecord_t *next;
	while (curr_trans->nModifyRegions) {
		SegList_t *seg_cur = curr_trans->RVM_Metadata->Seg_list;
		bool bSegFound = 0;
		uint32_t offset = undo_cur->Offset;
		uint32_t size = undo_cur->nBytes;
		curr_trans->nModifyRegions--;
		while (seg_cur != NULL) {
			if (seg_cur->SegBase == undo_cur->SegBase) {
				memcpy((void*) ((char*) seg_cur->SegBase + offset),
						undo_cur->buf, size);
				bSegFound = 1;
				seg_cur->Busy = 0;
				if (undo_cur->buf)
					free(undo_cur->buf);
				undo_cur->buf = NULL;
				break;
			}
			seg_cur = seg_cur->next;
		}
		if (!bSegFound) {
			print_err("Segment to restore not found in list");
		}
		if (undo_cur->next != NULL) {
			undo_cur = undo_cur->next;
		} else {
			if (curr_trans->nModifyRegions)
				print_err("Region Mismatch");
		}
	}
	undo_cur = curr_trans->UndoRecord;
	next = undo_cur->next;
	while (undo_cur != NULL) {
		if (undo_cur)
			free(undo_cur);
		undo_cur = next;
		if (undo_cur != NULL)
			next = undo_cur->next;
	}
	/*Cleanup transaction data structure
	 * Need to delete undo and redo records for this transaction
	 * Need to delete transaction itself
	 * Reset busy segments */
	RedoLogs_t *RedoLog = curr_trans->RedoLog;
	//RedoLogs_t *rprev;
	while (RedoLog) {
		//rprev = RedoLog;
		RedoLog->buf = NULL;//will be freed only on an unmap
		if (RedoLog->SegName)
			free(RedoLog->SegName);
		RedoLog = RedoLog->next;
		/*		if (rprev)
		 free(rprev);*/
	}
	if (gTransList->TID == tid) {
		Transactions_t* tprev = gTransList;
		gTransList = gTransList->next;
		bTransFound = 1;
		if (tprev)
			free(tprev);
	} else {
		Transactions_t* tprev = gTransList;
		curr_trans = gTransList->next;
		while (curr_trans != NULL) {
			if (tid == curr_trans->TID) {
				bTransFound = 1;
				tprev->next = curr_trans->next;
				if (curr_trans)
					free(curr_trans);
				curr_trans = NULL;
				break;
			}
			tprev = curr_trans;
			curr_trans = curr_trans->next;
		}
	}
	curr_trans->UndoRecord = NULL;
	curr_trans->RedoLog = NULL;
	if (!bTransFound) {
		print_err("Wrong transaction ID");
		return;
	}
}

void Cleanup_transaction(trans_t tid) {

	Transactions_t* curr_trans = gTransList;
	bool bTransFound = 0;
	while (curr_trans != NULL) {
		if (tid == curr_trans->TID) {
			bTransFound = 1;
			break;
		}
		curr_trans = curr_trans->next;
	}
	if (!bTransFound) {
		print_err("Wrong transaction ID");
		return;
	}
	/*Discard undo records as this is called only after a commit*/
	UndoRecord_t *undo_cur = curr_trans->UndoRecord;
	UndoRecord_t *prev;

	while (curr_trans->nModifyRegions) {
		SegList_t *seg_cur = gRVMMetadata->Seg_list;
		bool bSegFound = 0;
		curr_trans->nModifyRegions--;
		while (seg_cur != NULL) {
			if (seg_cur->SegBase == undo_cur->SegBase) {
				bSegFound = 1;
				seg_cur->Busy = 0;
				if (undo_cur->buf)
					free(undo_cur->buf);
				undo_cur->buf = NULL;
				break;
			}
			seg_cur = seg_cur->next;
		}
		if (!bSegFound) {
			print_err("Segment to restore not found in list");
		}
		if (undo_cur->next != NULL) {
			prev = undo_cur;
			undo_cur = undo_cur->next;
			if (prev)
				free(prev);
			prev = NULL;
		} else {
			if (curr_trans->nModifyRegions)
				print_err("Region Mismatch");
		}
	}

	/*Cleanup transaction data structure
	 * Need to delete undo and redo records for this transaction
	 * Need to delete transaction itself
	 * Reset busy segments */
	RedoLogs_t *RedoLog = curr_trans->RedoLog;
	//RedoLogs_t *rprev;
	while (RedoLog) {
		//rprev = RedoLog;
		RedoLog->buf = NULL;//will be freed only on an unmap
		if (RedoLog->SegName)
			free(RedoLog->SegName);
		RedoLog = RedoLog->next;
		/*		if (rprev)
		 free(rprev);*/
	}

	if (gTransList->TID == tid) {
		Transactions_t* tprev = gTransList;
		gTransList = gTransList->next;
		bTransFound = 1;
		if (tprev)
			free(tprev);
	} else {
		Transactions_t* tprev = gTransList;
		curr_trans = gTransList->next;
		while (curr_trans != NULL) {
			if (tid == curr_trans->TID) {
				bTransFound = 1;
				tprev->next = curr_trans->next;
				if (curr_trans)
					free(curr_trans);
				curr_trans = NULL;
				break;
			}
			tprev = curr_trans;
			curr_trans = curr_trans->next;
		}
	}
	curr_trans->UndoRecord = NULL;
	curr_trans->RedoLog = NULL;
	if (!bTransFound) {
		print_err("Wrong transaction ID");
		return;
	}
}

void rvm_commit_trans(trans_t tid) {
	char seg_filename[100];
	RedoLogs_t *redo_copy;
	Transactions_t *temp_trans;
	temp_trans = gTransList;
	while (temp_trans != NULL) {
		if (temp_trans->TID != tid) {
			temp_trans = temp_trans->next;
			continue;
		} else {
			redo_copy = temp_trans->RedoLog;
			FILE *redo_log;
			while (redo_copy != NULL) {
				strcpy(seg_filename, (char *) gRVMMetadata->DirName);
				strcat(seg_filename, "/");
				strcat(seg_filename, "REDO_LOG_");
				print_dbg("%s\n",seg_filename);
				strcat(seg_filename, redo_copy->SegName);
				print_dbg("%s\n",seg_filename);

				redo_log = fopen(seg_filename, "a");
				fprintf(redo_log, "#REDO LOG#%s\n", redo_copy->SegName);
				fprintf(redo_log, "%d ", redo_copy->Offset);
				fprintf(redo_log, "%d\n", redo_copy->nBytes);
				for (unsigned int i = 0; i < redo_copy->nBytes; i++) {
					fprintf(redo_log, "%c", ((char*) redo_copy->buf)[i]);
				}
				fprintf(redo_log, "\n");
				redo_copy = redo_copy->next;
				fclose(redo_log);

			}
			break;
		}
	}
	Cleanup_transaction(tid);
}

void rvm_truncate_log(rvm_t rvm) {
	char redo_filename[100], seg_filename[200], segment_name[100], *buffer;
	uint32_t Offset, nBytes;
	FILE *redo_file;
	SegList_t *temp_seg;
	temp_seg = gRVMMetadata->Seg_list;
	while (temp_seg != NULL) {
		strcpy(redo_filename, (char *) gRVMMetadata->DirName);
		strcat(redo_filename, "/");
		strcat(redo_filename, "REDO_LOG_");
		strcat(redo_filename, temp_seg->SegName);
		redo_file = fopen(redo_filename, "r");
		if (redo_file == NULL) {
			printf("Error: Failed to open %s\n", redo_filename);
			temp_seg = temp_seg->next;
			continue;
		}
		while (!feof(redo_file)) {
			fscanf(redo_file, "#REDO LOG#%s\n", segment_name);
			{
				int seg_file = 0;
				strcpy(seg_filename, (char *) gRVMMetadata->DirName);
				strcat(seg_filename, "/");
				strcat(seg_filename, segment_name);
				seg_file = open(seg_filename, O_RDWR, 0777);
				if (!seg_file) {
					printf("Error: Segment File does not exist!\n");
					break;
				}
				fscanf(redo_file, "%d %d\n", &Offset, &nBytes);
				print_dbg("%d %d\n",Offset, nBytes);
				buffer = (char *) malloc(sizeof(char) * nBytes);
				fread(buffer, nBytes, 1, redo_file);
				lseek(seg_file, Offset, SEEK_SET);
				write(seg_file, (void *) buffer, nBytes);
				close(seg_file);
				fscanf(redo_file, "\n");
				if (buffer)
					free(buffer);

			}
		}
		temp_seg = temp_seg->next;
		fclose(redo_file);
		unlink(redo_filename);

	}
}
