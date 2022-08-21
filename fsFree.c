/**************************************************************
* Class:  CSC-415-01 Fall 2021
* Names: Michael Davichick, Jeffery Friedrich, Brian Cheng , Ives-Christian Jadman 
* Student IDs:920196625 , 920095797, 918757468, 920450086
* GitHub Name: Ijadman
* Group Name: No Preference
* Project: Basic File System
*
* File: fsFree.c
*
* Description: Free space operations and management
*
**************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "fsLow.h"
#include "fsVcb.h"
#include "fsFree.h"

// converts size in bytes to size in blocks using int math
int getBlockSizeFromBytes(int bytes);
// creates a byte buffer of blocks size and updates VCB memory
FSM *getFreeBlocks(int blocks);

// TODO: Make safer for small block sizes
int initFreeSpace()
{
    int blockCount = getFreeBlockCount();
    if (blockCount == FREESPACEUNINTIALIZEDFLAG)
    {
        blockCount = getTotalBlockCount() - FREESPACESTARTOFFSET;
        printf("Initializing free space...\n");

        char *fsm = malloc(getBlockSize());
        ((FSM *)fsm)->currentBlock = FREESPACESTARTOFFSET;
        ((FSM *)fsm)->nextBlock = LASTBLOCK;
        ((FSM *)fsm)->size = blockCount;

        LBAwrite(fsm, 1, FREESPACESTARTOFFSET);
        setFirstFree((FSM *)fsm);
        setLastFree((FSM *)fsm);
        free(fsm);
        setFreeBlockCount(blockCount);
        printf("Finished initializing the free space...\n");
    }
    return 0;
}

int getFreeByteCount()
{
    return getFreeBlockCount() * getBlockSize();
}

int fsFree(FSM *firstBlock, FSM *lastBlock, int fileSize)
{
    FSM *lastFree = getLastFree();
    /*********************************/
    if (lastFree->currentBlock != LASTBLOCK)
    {
        char *writeBlock = malloc(getBlockSize());
        memcpy(writeBlock, lastFree, sizeof(FSM));
        ((FSM *)writeBlock)->nextBlock = firstBlock->currentBlock;
        LBAwrite(writeBlock, 1, ((FSM *)writeBlock)->currentBlock);
    }
    /********************************/
    setLastFree(lastBlock);
    setFreeBlockCount(getFreeBlockCount() + fileSize);
    return 0;
}

FSM *getFreeBytes(int bytes)
{
    int requestedBlocks = getBlockSizeFromBytes(bytes);
    if (getFreeBlockCount() < requestedBlocks)
    {
        return NULL;
    }
    return getFreeBlocks(requestedBlocks);
}

int defragmentFreeSpace()
{
    printf("Reached unimplemented function defragmentFreeSpace. Infinite loop imminent. Exiting before crash.");
    exit(-1);
    // return 0;
}

FSM *copyFSM(FSM *oldFSM)
{
    FSM *retObject = NULL;
    if (oldFSM != NULL)
    {
        retObject = malloc(sizeof(FSM));
        memcpy(retObject, oldFSM, sizeof(FSM));
    }
    return retObject;
}

int getBlockSizeFromBytes(int bytes)
{
    return (bytes + getBlockSize() - 1) / getBlockSize();
}

FSM *getFreeBlocks(int blocks)
{
    // pointer to first free space info from vcb
    FSM *head = getFirstFree();

    // pointer to return new allocated block(s) info
    FSM *ret = malloc(sizeof(FSM));
    // ret->currentBlock;
    ret->nextBlock = LASTBLOCK;
    ret->size = blocks;

    int changeLastBlockFlag = 0;
    if (getFirstFree() == getLastFree())
    {
        changeLastBlockFlag = 1;
    }

    // size < blocks
    if (head->size < blocks)
    {
        // find new block that works
        char *blockIterator = malloc(getBlockSize());
        memcpy(blockIterator, head, sizeof(FSM));

        char *blockBefore = malloc(getBlockSize());

        while (((FSM *)blockIterator)->size < blocks && ((FSM *)blockIterator)->nextBlock != LASTBLOCK)
        {
            memcpy(blockBefore, blockIterator, sizeof(FSM));
            LBAread(blockIterator, 1, ((FSM *)blockIterator)->nextBlock);
        }

        // if never find size > blocks
        //      defrag and start over
        if (((FSM *)blockIterator)->size < blocks)
        {
            defragmentFreeSpace();
            return getFreeBlocks(blocks);
        }

        // rewrite to block before
        //      currentBlock = currentBlock
        //      nextBlock = nextBlock of found block (if size = blocks) or block after found block (if size > blocks)
        //      size = size
        if (((FSM *)blockIterator)->size == blocks)
        {
            ((FSM *)blockBefore)->nextBlock = ((FSM *)blockIterator)->nextBlock;
        }
        else
        {
            ((FSM *)blockBefore)->nextBlock = ((FSM *)blockIterator)->currentBlock + ((FSM *)blockIterator)->size;
        }
        LBAwrite(blockBefore, 1, ((FSM *)blockBefore)->currentBlock);
        free(blockBefore);
        blockBefore = NULL;

        // write to block after if new found size > blocks
        //      currentBlock = found block currentblock + found block size
        //      nextBlock = nextBlock of found block
        //      size = size - blocks
        if (((FSM *)blockIterator)->size > blocks)
        {
            char *blockAfter = malloc(getBlockSize());
            ((FSM *)blockAfter)->currentBlock = ((FSM *)blockIterator)->currentBlock + ((FSM *)blockIterator)->size;
            ((FSM *)blockAfter)->nextBlock = ((FSM *)blockIterator)->nextBlock;
            ((FSM *)blockAfter)->size = ((FSM *)blockIterator)->size - blocks;
            LBAwrite(blockAfter, 1, ((FSM *)blockAfter)->currentBlock);
            free(blockAfter);
            blockAfter = NULL;
        }

        // rewrite to return block if new found blocks > size OR nextBlock != LASTBLOCK
        //      currentBlock = currentBlock
        //      nextBlock = LASTBLOCK
        //      size = blocks
        ret->currentBlock = ((FSM *)blockIterator)->currentBlock;
        LBAwrite(blockIterator, 1, ((FSM *)blockIterator)->currentBlock);
        free(blockIterator);
        blockIterator = NULL;

        // size > blocks
    }
    else if (head->size > blocks)
    {
        ret->currentBlock = head->currentBlock;

        // write to block after
        //      currentBlock = block currentblock + blocks
        //      nextBlock = nextBlock of block
        //      size = size - blocks
        char *blockAfter = malloc(getBlockSize());
        ((FSM *)blockAfter)->currentBlock = head->currentBlock + blocks;
        ((FSM *)blockAfter)->nextBlock = head->nextBlock;
        ((FSM *)blockAfter)->size = head->size - blocks;
        LBAwrite(blockAfter, 1, ((FSM *)blockAfter)->currentBlock);

        // change first free = block after
        setFirstFree((FSM *)blockAfter);
        if (changeLastBlockFlag)
        {
            setLastFree((FSM *)blockAfter);
        }
        free(blockAfter);
        blockAfter = NULL;

        // rewrite to return block
        //      currentBlock = currentBlock
        //      nextBlock = LASTBLOCK
        //      size = blocks
        char *currentBlock = malloc(getBlockSize());
        memcpy(currentBlock, ret, sizeof(FSM));
        LBAwrite(currentBlock, 1, ((FSM *)currentBlock)->currentBlock);
        free(currentBlock);
        currentBlock = NULL;

        // size = blocks
    }
    else
    {
        ret->currentBlock = head->currentBlock;

        // rewrite to return block if nextBlock != LASTBLOCK
        //      currentBlock = currentBlock
        //      nextBlock = LASTBLOCK
        //      size = blocks
        if (head->nextBlock != LASTBLOCK)
        {
            char *currentBlock = malloc(getBlockSize());
            memcpy(currentBlock, ret, sizeof(FSM));
            LBAwrite(currentBlock, 1, ((FSM *)currentBlock)->currentBlock);
            free(currentBlock);
            currentBlock = NULL;

            // change first free = nextBlock
            char *nextBlock = malloc(getBlockSize());
            LBAread(nextBlock, 1, head->nextBlock);
            setFirstFree((FSM *)nextBlock);
            if (changeLastBlockFlag)
            {
                setLastFree((FSM *)nextBlock);
            }
            free(nextBlock);
            nextBlock = NULL;
        }
        else
        {
            FSM *noBlock = malloc(sizeof(FSM));
            noBlock->currentBlock = LASTBLOCK;
            noBlock->nextBlock = LASTBLOCK;
            noBlock->size = 0;

            setFirstFree(noBlock);
            if (changeLastBlockFlag)
            {
                setLastFree(noBlock);
            }
            free(noBlock);
            noBlock = NULL;
        }
    }

    // change free block count
    setFreeBlockCount(getFreeBlockCount() - blocks);

    return ret;
}