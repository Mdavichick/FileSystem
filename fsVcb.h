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
// check if header already defined in scope
#ifndef _VCB_H
#define _VCB_H

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

#define VCB_HEADER        "VCB"

// 
#define FREESPACESTARTOFFSET 1
#define FREESPACEUNINTIALIZEDFLAG -1
#define ROOTUNINITIALIZEDFLAG -1

// TODO: Error returns/documentation
// initializes the VCB or reads it from disk. 0 if success
int initVcb(uint64_t numberOfBlocks, uint64_t blockSize);

// rewrites the VCB to disk. Returns block size if successful
int rewriteVCB();

// returns the total block count from VCB
int getTotalBlockCount();

// returns the total free block count from the VCB
int getFreeBlockCount();

// sets the total free block count in the VCB memory
void setFreeBlockCount(int newBlockCount);

// returns block size from the VCB
int getBlockSize();

// returns a byte pointer to a buffer of the first free block. NULL if an error occurred.
FSM* getFirstFree();

// sets first free block in VCB memory
int setFirstFree(FSM* blockNumber);

// returns a byte pointer to a buffer of the first free block. NULL if an error occurs
FSM* getLastFree();

// sets last free block in VCB memory
int setLastFree(FSM* blockNumber);

int setRootLocation(FSM* rootDirectory);

FSM* getRootLocation();

// saves the VCB to disk then frees all VCB memory
int freeVCB();

#endif