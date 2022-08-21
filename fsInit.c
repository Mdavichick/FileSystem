/**************************************************************
* Class:  CSC-415-0# Fall 2021
* Names: Michael Davichick, Jeffery Friedrich, Brian Cheng , Ives-Christian Jadman 
* Student IDs:920196625 , 920095797, 918757468, 920450086
* GitHub Name: Ijadman
* Group Name: No Preference
* Project: Basic File System
*
* File: fsInit.c
*
* Description: Main driver for file system assignment.
*
* This file is where you will start and initialize your system
*
**************************************************************/

#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "fsLow.h"
#include "mfs.h"
#include "fsVcb.h"
#include "fsFree.h"
#include "fsDir.h"

#include "fsTest.h"


int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize)
{
	printf("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */

	int testReturn = testFileSystem();
	if(testReturn != 0) {
		printf("Test return was %i\n", testReturn);
	}

	if (initVcb(numberOfBlocks, blockSize) || initFreeSpace() || initRootDir())
	{
		// error handling
		exitFileSystem();
	} else {
		rewriteVCB();
	}
	
	return 0;
}

void exitFileSystem()
{
	printf("System exiting\n");
	freeDirs();
	freeVCB();
}