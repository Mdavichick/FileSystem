/**************************************************************
* Class:  CSC-415-01 Fall 2021
* Names: Michael Davichick, Jeffery Friedrich, Brian Cheng , Ives-Christian Jadman 
* Student IDs:920196625 , 920095797, 918757468, 920450086
* GitHub Name: Ijadman
* Group Name: No Preference
* Project: Basic File System
*
* File: fsVCB.h
*
* Description: Diretory Entry structure and functions
*
**************************************************************/
#ifndef _DIR_H
#define _DIR_H

#include <time.h>

#ifndef uint64_t
typedef u_int64_t uint64_t;
#endif
#ifndef uint32_t
typedef u_int32_t uint32_t;
#endif

#ifndef FSMDef
#define FSMDef
typedef struct FSM {
    int currentBlock;
    int nextBlock;
    int size;
} FSM;
#endif

#ifndef DIRMAX_LEN
#define DIRMAX_LEN 4096
#endif

#ifndef DEFAULTFILESIZE
#define DEFAULTFILESIZE 4096
#endif

#define MAXDIRENTRIES 50
#define NAMESIZE 32

typedef struct DirEntry
{
    FSM dirEntBlockInfo;//this struct contains block info.
    char name[NAMESIZE];//directory entry name
    time_t dateCreated;
    time_t datelastModified;
    time_t dateLastAccessed;
    unsigned char isDir;// 1 is directory, 0 is other file;
    unsigned char isFree;// 1 is free , 0 is taken;
} DirEntry;

typedef struct Directory
{
    DirEntry dirArray[MAXDIRENTRIES];
}Directory;


//pointer to get information of the directory
DirEntry* getDirEntryInfo(char *dirName);

int unassignDirEntry(DirEntry* dirEnt);

int initRootDir();

int getDirNum();

DirEntry* createDirEntry(char* name, int size, unsigned char isDir);

Directory* createDirectory(DirEntry* dirEnt, Directory* parent);

int assignDirEntryToDirectory(DirEntry *dirEnt, Directory* parent);

int deleteDirectory(Directory* directory);

int deleteDirEntry(DirEntry* dirEnt);

DirEntry* copyDirEntry(DirEntry* dirEnt);

FSM* copyEntryFSM(DirEntry* entry);

FSM* getFSM(Directory* directory);

FSM* getParentFSM(Directory* directory);

uint64_t writeDirectory(Directory* directory);

uint64_t writeDirectoryEntry(FSM* dirEntry, void* buffer);

uint64_t writePartialDirectoryEntry(FSM* dirEntry, void* buffer, int blockOffset, int blockCount);

uint64_t readPartialDirEntry(FSM* dirEntry, int blockOffset, int blockCount, void* buffer, int bufferIndex);

void* readDirEntry(DirEntry* dirEntry);

DirEntry* searchDirectory(Directory* directory, char* name);

DirEntry* searchDirectoryByBlock(Directory* directory, FSM* blockInfo);

// safely frees a Directory pointer
// won't free root, cwd, or null pointers
int freeDirectoryPtr(Directory* dir);

int freeDirs();

Directory* getRootDirectory();

Directory* getCWD();

int setCWD(Directory* directory);

char* getCWDPath();

int setCWDPath(char* newpath);

int isDirectoryEmpty(Directory* directory);

int getSizeofBlocks();

#endif