/**************************************************************
* Class:  CSC-415-01 Fall 2021
* Names: Michael Davichick, Jeffery Friedrich, Brian Cheng , Ives-Christian Jadman 
* Student IDs:920196625 , 920095797, 918757468, 920450086
* GitHub Name: Ijadman
* Group Name: No Preference
* Project: Basic File System
*
* File: fsVcb.c
*
* Description: VCB structure and functions
*
**************************************************************/
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "fsLow.h"
#include "fsVcb.h"

typedef struct VCB
{
    char header[16];
    u_int32_t totalBlockCount;   // total # of blocks
    u_int32_t numFreeBlockCount; // # of free blocks
    u_int32_t blockSize;         // block size
    u_int32_t freeFCBCount;      // free FCB count
    FSM root;              // pointer to root directory (0 index)
    FSM firstFreeSpace;    // free block pointer
    FSM lastFreeSpace;     // last free block pointer
    // bFCB nextFCB;        // free FCB pointer
    // type of volume (file system)  // need more research to understand implementation
} VCB;

// reads VCB from disk to memory
// WARNING: WILL OVERWRITE MEMORY VCB
int reloadVCB();

int loadFirstFree();
int loadLastFree();

VCB* vcb;
FSM* firstFree;
FSM* lastFree;

int initVcb(uint64_t numberOfBlocks, uint64_t blockSize)
{
    //3 error cases we need to implement:
    //if LbaRead,Lbawrite,malloc
    vcb = (VCB *)malloc(blockSize); // Block 0 buffer
    LBAread(vcb, 1, 0);

    if (strcmp(vcb->header, "VCB") != 0)
    {
        printf("Creating VCB in Block 1\n");
        stpcpy(vcb->header, "VCB");
        vcb->totalBlockCount = numberOfBlocks;
        vcb->numFreeBlockCount = FREESPACEUNINTIALIZEDFLAG;
        vcb->blockSize = blockSize;
        vcb->root.currentBlock = ROOTUNINITIALIZEDFLAG;
    }
    firstFree = NULL;
    lastFree = NULL;
    return 0;
}

int rewriteVCB()
{
    return LBAwrite(vcb, 1, 0);
}

int getTotalBlockCount()
{
    return vcb->totalBlockCount;
}

int getFreeBlockCount()
{
    return vcb->numFreeBlockCount;
}

void setFreeBlockCount(int newBlockCount)
{
    vcb->numFreeBlockCount = newBlockCount;
}

int getBlockSize()
{
    return vcb->blockSize;
}

FSM* getFirstFree()
{
    if (firstFree == NULL || (firstFree->currentBlock != (vcb->firstFreeSpace).currentBlock))
    {
        if (loadFirstFree() == 0)
        {
            return firstFree;
        }
        else
        {
            return NULL;
        }
    }
    return firstFree;
}

int setFirstFree(FSM *blockNumber)
{
    memcpy(&(vcb->firstFreeSpace), blockNumber, sizeof(FSM));
    if(firstFree == NULL) {
        loadFirstFree();
    }
    memcpy(firstFree, blockNumber, sizeof(FSM));
    return 0;
}

FSM* getLastFree()
{
    if (lastFree == NULL)
    {
        if (loadLastFree() != 0)
        {
            return NULL;
        }
    }
    if((lastFree->currentBlock != (vcb->lastFreeSpace).currentBlock)) {
        printf("ERROR: FREE SPACE MISMATCH DETECTED! FILES OR DIRECTORIES MAY BE LOST!");
        return NULL;
    }
    return lastFree;
}

int setLastFree(FSM *blockNumber)
{
    memcpy(&(vcb->lastFreeSpace), blockNumber, sizeof(FSM));
    if(lastFree == NULL) {
        loadLastFree();
    }
    memcpy(lastFree, blockNumber, sizeof(FSM));
    return 0;
}

int setRootLocation(FSM* rootDirectory) {
    memcpy(&(vcb->root), rootDirectory, sizeof(FSM));
    return 0;
}

FSM* getRootLocation() {
    return &(vcb->root);
}

int freeVCB()
{
    //rewriteVCB();
    if(vcb != NULL) {
        free(vcb);
        vcb = NULL;
    }
    if (firstFree != NULL)
    {
        free(firstFree);
        firstFree = NULL;
    }
    if (lastFree != NULL)
    {
        free(lastFree);
        lastFree = NULL;
    }
    return 0;
}

int reloadVCB()
{
    return LBAread(vcb, 1, 0);
}

int loadFirstFree()
{
    char *firstFreeBuffer = malloc(vcb->blockSize);
    LBAread(firstFreeBuffer, 1, (vcb->firstFreeSpace).currentBlock);
    if(firstFree == NULL) {
        firstFree = malloc(sizeof(FSM));
    }
    memcpy(firstFree, firstFreeBuffer, sizeof(FSM));
    free(firstFreeBuffer);
    return 0;
}

int loadLastFree()
{
    char *lastFreeBuffer = malloc(vcb->blockSize);
    LBAread(lastFreeBuffer, 1, (vcb->lastFreeSpace).currentBlock);
    if(lastFree == NULL) {
        lastFree = malloc(sizeof(FSM));
    }
    memcpy(lastFree, lastFreeBuffer, sizeof(FSM));
    free(lastFreeBuffer);
    return 0;
}