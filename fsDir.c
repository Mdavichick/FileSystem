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
* Description: VCB structure and functions
*
**************************************************************/
/*
Initialize the Root Directory
First again, you need memory – how much?
Decide how many Directory Entries (DE) you want for a directory (at least an initial amount). I will use 50 for this example.
Now multiply the size of your directory entry by the number of entries. For example, if my directory entry is 60 bytes, I would multiply 60 * 50 = 3000 bytes needed.
Now you need to determine how many blocks you need. In the example that would be 6 blocks. But wait…. 6 blocks is 3072 bytes and I am only using 3000 bytes, and my directory entry is only 60 bytes, so I can actually fit 51 directory entries in the space of 3072 bytes (now only wasting 12 bytes instead of 72).
Now you have a pointer to an array of directory entries.  Loop through them (in our case for (i = 0; i < 51; i++) and initialize each directory entry structure to be in a known free state.
Now ask the free space system for 6 blocks. It should return to you the starting block number for those 6 blocks.
Now set the first directory entry to the “.” Directory. So, the name is “.”, the size is 3060 bytes, it starts at block number (whatever the free space system returned above), the time stamps are the current time, it is a directory, etc.
Now set the second directory entry to “..”, but since this is the root, it is the same as the “.” except the name is “..” rather than “.”
Now write the root directory – 6 blocks starting from block (what was returned from the free space allocation).
Return the starting block number of the root directory (or set it in the VCB yourself)
*/
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "fsLow.h"
#include "fsVcb.h"
#include "fsFree.h"
#include "fsDir.h"

Directory *rootDirectory;
Directory *cwd;
char cwdPath[DIRMAX_LEN];

//DirEntry *initDirEntryArrayBlock(DirEntry *dirEntryArray); //init directory entries

DirEntry *initDirEntryArrayBlock(DirEntry *dirEntryArray)
{
    // TODO: Compare MAXDIRENTRIES with current implementation
    // one is size that fits in space, other is absolute max
    // int arraySize = getDirNum() * dirEntryArray[0].dirEntBlockInfo.size;
    int arraySize = MAXDIRENTRIES;
    for (size_t i = 2; i < arraySize; i++)
    {
        dirEntryArray[i].isFree = 1;
    }
    return dirEntryArray;
}

DirEntry *createDirEntry(char *name, int size, unsigned char isDir)
{
    printf("-------------------------Directory Entry Initialization-------------------------\n");
    DirEntry *retDirEnt = malloc(sizeof(DirEntry));
    FSM *dirEntFSM = getFreeBytes(size);
    memcpy(retDirEnt, dirEntFSM, sizeof(FSM));
    free(dirEntFSM);
    strncpy(retDirEnt->name, name, NAMESIZE);
    retDirEnt->dateCreated = time(NULL);
    retDirEnt->dateLastAccessed = time(NULL);
    retDirEnt->datelastModified = time(NULL);
    retDirEnt->isDir = isDir;
    retDirEnt->isFree = 0;
    printf("-------------------------Directory Entry Initialization Finished-------------------------\n");
    return retDirEnt;
}

Directory *createDirectory(DirEntry *dirEnt, Directory *parent)
{
    printf("-------------------------Directory Initialization-------------------------\n");
    Directory *newDir = malloc(sizeof(Directory));
    memcpy(&(newDir->dirArray[0]), dirEnt, sizeof(DirEntry));
    memcpy(&(newDir->dirArray[1]), parent, sizeof(DirEntry));
    strcpy(newDir->dirArray[0].name, ".");
    strcpy(newDir->dirArray[1].name, "..");
    initDirEntryArrayBlock(newDir->dirArray);
    printf("-------------------------Directory Initialization Finished-------------------------\n");

    return newDir;
}

int assignDirEntryToDirectory(DirEntry *dirEnt, Directory *parent)
{
    for (int i = 2; i < MAXDIRENTRIES; i++)
    {
        if (parent->dirArray[i].isFree == 1)
        {
            memcpy(&(parent->dirArray[i]), dirEnt, sizeof(DirEntry));
            return 0;
        }
    }
    printf("Failed to add Directory Entry: No space available.\n");
    return 1;
}

int unassignDirEntry(DirEntry *dirEnt)
{
    dirEnt->isFree = 1;
    return 0;
}

int deleteDirectory(Directory *directory)
{
    if (directory == getCWD())
    {
        cwd = rootDirectory;
        strncpy(cwdPath, "/", 2);
    }
    fsFree((FSM *)directory, (FSM *)directory, directory->dirArray[0].dirEntBlockInfo.size);
    return 0;
}

int deleteDirEntry(DirEntry *dirEnt)
{
    fsFree((FSM *)dirEnt, (FSM *)dirEnt, dirEnt->dirEntBlockInfo.size);
    return 0;
}

DirEntry *copyDirEntry(DirEntry *dirEnt)
{
    DirEntry *returnDirEnt = malloc(sizeof(DirEntry));
    memcpy(returnDirEnt, dirEnt, sizeof(DirEntry));
    return returnDirEnt;
}

FSM *copyEntryFSM(DirEntry *entry)
{
    return copyFSM(&(entry->dirEntBlockInfo));
}

FSM *getFSM(Directory *directory)
{
    return &(directory->dirArray[0].dirEntBlockInfo);
}

FSM *getParentFSM(Directory *directory)
{
    return &(directory->dirArray[1].dirEntBlockInfo);
}

DirEntry *getParent(Directory *directory)
{
    return &(directory->dirArray[1]);
}

DirEntry *getSelf(Directory *directory)
{
    return &(directory->dirArray[0]);
}

int initRootDir()
{
    FSM *possibleRoot = getRootLocation();
    if (possibleRoot->currentBlock == ROOTUNINITIALIZEDFLAG)
    {
        printf("-------------------------Root Directory Initialization-------------------------\n");
        DirEntry *rootDirEnt = createDirEntry("root", sizeof(Directory), 1);
        Directory *rootDirect = createDirectory(rootDirEnt, (Directory *)rootDirEnt);
        free(rootDirEnt);
        setRootLocation((FSM *)rootDirect);
        writeDirectory(rootDirect);
        rootDirectory = rootDirect;
        setCWD(rootDirect);

        printf("-------------------------Root Directory Initialization Finished-------------------------\n");
    }
    else
    {
        Directory *rootDirect = malloc(getBlockSize() * possibleRoot->size);
        LBAread(rootDirect, possibleRoot->size, possibleRoot->currentBlock);
        rootDirectory = rootDirect;
        setCWD(rootDirect);
    }
    setCWDPath("/");

    return 0;
}

uint64_t writeDirectory(Directory *directory)
{
    return LBAwrite(directory, directory->dirArray[0].dirEntBlockInfo.size, directory->dirArray[0].dirEntBlockInfo.currentBlock);
}

uint64_t writeDirectoryEntry(FSM *dirEntry, void *buffer)
{
    return LBAwrite(buffer, dirEntry->size, dirEntry->currentBlock);
}

uint64_t writePartialDirectoryEntry(FSM *dirEntry, void *buffer, int blockOffset, int blockCount)
{
    if (blockOffset + blockCount > dirEntry->size)
    {
        return -1;
    }
    return LBAwrite(buffer, blockCount, dirEntry->currentBlock + blockOffset);
}

uint64_t readPartialDirEntry(FSM *dirEntry, int blockOffset, int blockCount, void *buffer, int bufferIndex)
{
    if (blockOffset + blockCount > dirEntry->size)
    {
        return -1;
    }
    return LBAread(buffer + bufferIndex, blockCount, dirEntry->currentBlock + blockOffset);
}

void *readDirEntry(DirEntry *dirEntry)
{

    void *buffer = malloc(dirEntry->dirEntBlockInfo.size * getBlockSize());
    LBAread(buffer, dirEntry->dirEntBlockInfo.size, dirEntry->dirEntBlockInfo.currentBlock);

    return buffer;
}

DirEntry *searchDirectory(Directory *directory, char *name)
{

    int arraySize = MAXDIRENTRIES;
    for (int i = 0; i < MAXDIRENTRIES; i++)
    {
        if (directory->dirArray[i].isFree == 0 && strncmp(directory->dirArray[i].name, name, NAMESIZE) == 0)
        {

            return &(directory->dirArray[i]);
        }
    }

    return NULL;
}

DirEntry *searchDirectoryByBlock(Directory *directory, FSM *blockInfo)
{

    int arraySize = MAXDIRENTRIES;
    for (int i = 0; i < MAXDIRENTRIES; i++)
    {
        if (directory->dirArray[i].isFree == 0 && directory->dirArray[i].dirEntBlockInfo.currentBlock == blockInfo->currentBlock)
        {

            return &(directory->dirArray[i]);
        }
    }

    return NULL;
}

int freeDirectoryPtr(Directory *dir)
{
    if (dir != NULL && dir != getRootDirectory() && dir != getCWD())
    {
        free(dir);
    }
    return 0;
}

int freeDirs()
{
    if (rootDirectory != NULL)
    {
        if (cwd == rootDirectory)
        {
            cwd = NULL;
        }
        free(rootDirectory);
        rootDirectory = NULL;
    }
    if (cwd != NULL)
    {
        free(cwd);
        cwd = NULL;
    }
    return 0;
}

Directory *getRootDirectory()
{
    return rootDirectory;
}

Directory *getCWD()
{
    return cwd;
}

int setCWD(Directory *directory)
{
    Directory *temp = cwd;
    cwd = directory;
    freeDirectoryPtr(temp);
    return 0;
}

char *getCWDPath()
{
    return cwdPath;
}

int setCWDPath(char *newpath)
{
    strncpy(cwdPath, newpath, DIRMAX_LEN);
    return 0;
}

int isDirectoryEmpty(Directory *directory)
{
    for (int i = 2; i < MAXDIRENTRIES; i++)
    {
        if (directory->dirArray[i].isFree == 0)
        {
            return 0;
        }
    }
    return 1;
}

int getDirNum()
{
    return getBlockSize() / sizeof(DirEntry); //returns the number of directory entries per directory per block.
}

Directory *lbaToDirectory(int blockNumber, int size)
{
    Directory *directory = malloc(size * getBlockSize());
    LBAread(directory, size, blockNumber);
    return directory;
}

int getSizeofBlocks()
{
    return getBlockSize();
}