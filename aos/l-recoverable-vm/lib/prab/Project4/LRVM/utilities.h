/* 
 * File:   utilities.h
 * Author: machiry
 *
 * Created on April 6, 2013, 1:17 PM
 */

#ifndef UTILITIES_H
#define	UTILITIES_H

#ifdef	__cplusplus
extern "C" {
#endif

int copyFile(char *src, char *dst);
int deleteFile(char *file);
void copyExtended(const char *src, const char *dst, unsigned long size);
void createNullFile(const char* file, unsigned long size);
void inSufficientMemory(const char* errMsg);
char* combinePaths(const char* dir, const char* fileName);
int startsWith(const char *pre, const char *str);
int readLineFromFile(FILE* toread,char *line,unsigned long size);
void createFile(const char* fileName);
void writeBytesToFile(FILE* src, FILE* dst, unsigned long noOfBytes);
void writeBytesToFileFromMemory(FILE* toFile, void *memory, unsigned long size);
void writeBytesToMemory(FILE* fromFile, void *memory, unsigned long size);
unsigned long getNextRandomID();
unsigned long getNextTransactionID();


#ifdef	__cplusplus
}
#endif

#endif	/* UTILITIES_H */

