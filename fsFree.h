/**************************************************************
* Class:  CSC-415-01 Fall 2021
* Names: Michael Davichick, Jeffery Friedrich, Brian Cheng , Ives-Christian Jadman 
* Student IDs:920196625 , 920095797, 918757468, 920450086
* GitHub Name: Ijadman
* Group Name: No Preference
* Project: Basic File System
*
* File: fsFree.h
*
* Description: Free space operations and management interface
*
**************************************************************/

// check if header already defined in scope
#ifndef _FREE_H
#define _FREE_H

// if nextBlockOffset is set to this, its the last block in ___
#define LASTBLOCK -1

#ifndef FSMDef
#define FSMDef
// free space management structure. Every block in memory or on disk has one
typedef struct FSM {
    // LBA block value of this item
    int currentBlock;
    // LBA block value of the next item in the linked list
    int nextBlock;
    // number of contiguous blocks
    int size;
} FSM;
#endif

// initializes the free space if it has not been initialized yet
// returns 0 if already initialized or 1 if initialization happened
int initFreeSpace();

// returns the calculated number of free bytes left
int getFreeByteCount();

// TODO: retool for directory entries (maybe) and update free block count in VCB
// adds the space between two blocks to the free space and updates the VCB in memory
// returns 0 if successful
int fsFree(FSM* firstBlock, FSM* lastBlock, int fileSize);

// returns a buffer of the size given rounded up to the nearest block
// returns NULL if bytes requested is too large
FSM* getFreeBytes(int bytes);

// moves written blocks around to create more space
// WARNING: MAY BE SLOW
int defragmentFreeSpace();

FSM* copyFSM(FSM* oldFSM);

#endif