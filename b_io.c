/**************************************************************
* Class:  CSC-415-0# Fall 2021
* Names: Michael Davichick, Jeffery Friedrich, Brian Cheng , Ives-Christian Jadman 
* Student IDs:920196625 , 920095797, 918757468, 920450086
* GitHub Name: Ijadman
* Group Name: No Preference
* Project: Basic File System
*
* File: b_io.c
*
* Description: Basic File System - Key File I/O Operations
*
**************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>			// for malloc
#include <string.h>			// for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "b_io.h"
#include "fsDir.h"
#include "fsParse.h"
#include "fsString.h"

#define MAXFCBS 20
#define B_CHUNK_SIZE 512

typedef struct b_fcb
	{
	/** TODO add all the information you need in the file control block **/
	FSM* fileInfo;  //holds important info like location and size
	char* buf;		//holds the open file buffer
	off_t fileOffset; // holds the current position in the file
	int index;		//holds the current position in the buffer
	int buflen;		//holds how many valid bytes are in the buffer
	} b_fcb;
	
b_fcb fcbArray[MAXFCBS];

int startup = 0;	//Indicates that this has not been initialized

int getBlockOffset(off_t fileOffset);
int getOffsetWithinBlock(off_t fileOffset);

//Method to initialize our file system
void b_init ()
	{
	//init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
		{
		fcbArray[i].buf = NULL; //indicates a free fcbArray
		}
		
	startup = 1;
	}

//Method to get a free FCB element
b_io_fd b_getFCB ()
	{
	for (int i = 0; i < MAXFCBS; i++)
		{
		if (fcbArray[i].buf == NULL)
			{
			return i;		//Not thread safe (But do not worry about it for this assignment)
			}
		}
	return (-1);  //all in use
	}
	
// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR
b_io_fd b_open (char* filename, int flags)
	{
	b_io_fd returnFd;

	//*** TODO ***:  Modify to save or set any information needed
	//
	//
		
	if (startup == 0) b_init();  //Initialize our system
	
	returnFd = b_getFCB();				// get our own file descriptor
										// check for error - all used FCB's
	if(returnFd == -1) {
		printf("b_open failed. All available file descriptors are in use.\n");
		return -1;
	}

	// Get the file info for the given filename. 
	// If file cannot be found, return error status -1
	char* nameBuffer = malloc(sizeof(char) * NAMESIZE);
    Directory* parentDir = parsePath(filename, nameBuffer);
	if(parentDir == NULL) {
        printf("Path is invalid\n");
        return -1;
	} 
	DirEntry* entry = searchDirectory(parentDir, nameBuffer);
	if(entry == NULL) {
        if(flags & O_CREAT) {
			entry = createDirEntry(nameBuffer, DEFAULTFILESIZE, 0);
			assignDirEntryToDirectory(entry, parentDir);
			char* buffer = malloc(entry->dirEntBlockInfo.size);
			memset(buffer, '\0', entry->dirEntBlockInfo.size);
			writeDirectoryEntry(&(entry->dirEntBlockInfo), buffer);
			writeDirectory(parentDir);
			free(buffer);
		} else {
			printf("b_open failed. The filepath %s was invalid.\n", filename);
			return -1;
		}
	} else {
		entry = copyDirEntry(entry);
	}
	fcbArray[returnFd].fileInfo = copyEntryFSM(entry);
	free(nameBuffer);
	freeDirectoryPtr(parentDir);
	free(entry);


	// Allocate the buffer for this file descriptor. 
	// If file cannot be found, "free" fd and return error status -1
	fcbArray[returnFd].buf = (char*) malloc(sizeof(char) * B_CHUNK_SIZE);
	if(fcbArray[returnFd].buf == NULL) {
		printf("b_open failed. Failed to initialize buffer.\n");
		return -1;
	}

	// Initialize fileOffset to beginning of file and bufferOffset to empty (prompting a read)
	fcbArray[returnFd].index = 0;
	fcbArray[returnFd].buflen = 0;
	
	return (returnFd);						// all set
	}


// Interface to seek function	
int b_seek (b_io_fd fd, off_t offset, int whence)
	{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
	
	if(getBlockOffset(offset + whence)  > fcbArray[fd].fileInfo->size) {
		return -1; 						//offset too large
	}

	fcbArray[fd].fileOffset = offset + whence;
		
	return (0); //Change this
	}



// Interface to write function	
int b_write (b_io_fd fd, char * buffer, int count)
	{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}

	if (fcbArray[fd].buf == NULL) {		//File not open for this descriptor 
		printf("b_write failed. The file descriptor %i does not correspond to an open file.\n", fd);
		return -1;
	}
	
	int blocksWritten = writePartialDirectoryEntry(fcbArray[fd].fileInfo, buffer, getBlockOffset(fcbArray[fd].fileOffset), count);

	// fcbArray[fd].fileOffset += ;
	// printf("%i", blocksWritten);
	return blocksWritten;

	// // Bytes placed in buffer so far (return value)
	// int bytesWritten = 0;
	
	// // Remaining bytes available to write in file
	// int fileRemaining = fcbArray[fd].fileInfo->size - fcbArray[fd].fileOffset;

	// // Bytes to be written in first section min(fileRemaining, count)
	// int initialBytes = (fileRemaining > count) ? count : fileRemaining;

	// // First Section:
	// // If the file buffer is not sufficient to fulfill the write, 
	// //		copy all that remains into the input buffer. 
	// //		Update all variables accordingly.
	// if(initialBytes >= fcbArray[fd].buflen && fcbArray[fd].buflen != 0) {
	// 	memcpy(fcbArray[fd].buf + fcbArray[fd].index, buffer, B_CHUNK_SIZE - fcbArray[fd].buflen);
	// 	bytesWritten = B_CHUNK_SIZE - fcbArray[fd].buflen;

	// 	char* tempFile = malloc(sizeof(char) * getSizeofBlocks());
	// 	readPartialDirEntry(fcbArray[fd].fileInfo, getBlockOffset(fcbArray[fd].fileOffset), 1, tempFile, 0);
	// 	memcpy(tempFile + getOffsetWithinBlock(fcbArray[fd].fileOffset), fcbArray[fd].buf, B_CHUNK_SIZE);
	// 	writePartialDirectoryEntry(fcbArray[fd].fileInfo, tempFile, getBlockOffset(fcbArray[fd].fileOffset), 1);
	// 	free(tempFile);

	// 	fcbArray[fd].fileOffset += bytesWritten;
	// 	fcbArray[fd].index = 0;
	// 	fcbArray[fd].buflen = 0;
	// 	fileRemaining = fcbArray[fd].fileInfo->size - fcbArray[fd].fileOffset;
	// }

	// // How many blocks have been written already
	// int blockOffset = getBlockOffset(fcbArray[fd].fileOffset);

	// // How many bytes are left from count
	// int remainingBytes = count - bytesWritten;

	// // Blocks to be write in second section min(fileRemaining, remainingBytes)
	// int remainingFullBlocks = (fileRemaining > remainingBytes) ? remainingBytes : fileRemaining;
	// remainingFullBlocks = remainingFullBlocks / getSizeofBlocks();

	// // Second Section:
	// // If the remaining count is large enough to take block size multiples, 
	// //		copy as many blocks as will fit within bounds. 
	// // 		Update all variables accordingly.
	// if(remainingFullBlocks) {
	// 	writePartialDirectoryEntry(fcbArray[fd].fileInfo, buffer + bytesWritten, getBlockOffset(fcbArray[fd].fileOffset), remainingFullBlocks);

	// 	bytesWritten += getSizeofBlocks() * remainingFullBlocks;
	// 	fcbArray[fd].fileOffset += getSizeofBlocks() * remainingFullBlocks;
		
	// 	remainingBytes = count - bytesWritten;
	// 	fileRemaining = fcbArray[fd].fileInfo->size - fcbArray[fd].fileOffset;
	// 	blockOffset += remainingFullBlocks;
	// }

	// // Third Section:
	// // If any bytes are remaining in the write count and the file after the last two sections, 
	// //		fulfill remaining bytes requested, refilling the buffer prior if necessary.
	// //		Update only the return value and variables attached to the file descriptor afterwards.
	// if(remainingBytes && fileRemaining) {
	// 	int lastBytes = (fileRemaining > remainingBytes) ? remainingBytes : fileRemaining;
	// 	memcpy(fcbArray[fd].buf + fcbArray[fd].index, buffer + bytesWritten, lastBytes);

	// 	bytesWritten += lastBytes;
	// 	fcbArray[fd].fileOffset += lastBytes;
	// 	fcbArray[fd].index += lastBytes;
	// 	fcbArray[fd].buflen = fcbArray[fd].buflen + lastBytes;
	// }

	// // Return total bytes copied into output buffer
	// return bytesWritten;
	}



// Interface to read a buffer

// Filling the callers request is broken into three parts
// Part 1 is what can be filled from the current buffer, which may or may not be enough
// Part 2 is after using what was left in our buffer there is still 1 or more block
//        size chunks needed to fill the callers request.  This represents the number of
//        bytes in multiples of the blocksize.
// Part 3 is a value less than blocksize which is what remains to copy to the callers buffer
//        after fulfilling part 1 and part 2.  This would always be filled from a refill 
//        of our buffer.
//  +-------------+------------------------------------------------+--------+
//  |             |                                                |        |
//  | filled from |  filled direct in multiples of the block size  | filled |
//  | existing    |                                                | from   |
//  | buffer      |                                                |refilled|
//  |             |                                                | buffer |
//  |             |                                                |        |
//  | Part1       |  Part 2                                        | Part3  |
//  +-------------+------------------------------------------------+--------+
int b_read (b_io_fd fd, char * buffer, int count)
	{

	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
		
	if (fcbArray[fd].buf == NULL) {		//File not open for this descriptor 
		printf("b_read failed. The file descriptor %i does not correspond to an open file.\n", fd);
		return -1;
	}

	// Bytes placed in buffer so far (return value)
	int bytesReturned = 0;
	
	// Remaining bytes available to read in file
	int fileRemaining = fcbArray[fd].fileInfo->size - fcbArray[fd].fileOffset;

	// Bytes to be read in first section min(fileRemaining, count)
	int initialBytes = (fileRemaining > count) ? count : fileRemaining;

	// First Section:
	// If the file buffer is not sufficient to fulfill the read, 
	//		copy all that remains into the output buffer. 
	//		Update all variables accordingly.
	if(initialBytes >= fcbArray[fd].buflen && fcbArray[fd].buflen != 0) {
		memcpy(buffer, fcbArray[fd].buf + fcbArray[fd].index, fcbArray[fd].buflen);
		bytesReturned = fcbArray[fd].buflen;
		fcbArray[fd].fileOffset += bytesReturned;
		fcbArray[fd].index = B_CHUNK_SIZE;
		fcbArray[fd].buflen = 0;
		fileRemaining = fcbArray[fd].fileInfo->size - fcbArray[fd].fileOffset;
	}

	// How many blocks have been read already
	int blockOffset = getBlockOffset(fcbArray[fd].fileOffset);

	// How many bytes are left from count
	int remainingBytes = count - bytesReturned;

	// Blocks to be read in second section min(fileRemaining, remainingBytes)
	int remainingFullBlocks = (fileRemaining > remainingBytes) ? remainingBytes : fileRemaining;
	remainingFullBlocks = remainingFullBlocks / B_CHUNK_SIZE;

	// Second Section:
	// If the remaining count is large enough to take block size multiples, 
	//		copy as many blocks as will fit within bounds. 
	// 		Update all variables accordingly.
	if(remainingFullBlocks) {
		// LBAread(buffer + bytesReturned, remainingFullBlocks, fcbArray[fd].fileInfo->currentBlock + blockOffset);
		readPartialDirEntry(fcbArray[fd].fileInfo, blockOffset, remainingFullBlocks, buffer, bytesReturned);

		bytesReturned += B_CHUNK_SIZE * remainingFullBlocks;
		fcbArray[fd].fileOffset += B_CHUNK_SIZE * remainingFullBlocks;
		
		remainingBytes = count - bytesReturned;
		fileRemaining = fcbArray[fd].fileInfo->size - fcbArray[fd].fileOffset;
		blockOffset += remainingFullBlocks;
	}

	// Third Section:
	// If any bytes are remaining in the read count and the file after the last two sections, 
	//		fulfill remaining bytes requested, refilling the buffer prior if necessary.
	//		Update only the return value and variables attached to the file descriptor afterwards.
	if(remainingBytes && fileRemaining) {
		if(fcbArray[fd].index == B_CHUNK_SIZE) {
			// LBAread(fcbArray[fd].buf, 1, fcbArray[fd].fileInfo->currentBlock + blockOffset);
			readPartialDirEntry(fcbArray[fd].fileInfo, blockOffset, 1, fcbArray[fd].buf, 0);
			fcbArray[fd].index = 0;
			fcbArray[fd].buflen = B_CHUNK_SIZE;
		}

		int lastBytes = (fileRemaining > remainingBytes) ? remainingBytes : fileRemaining;
		memcpy(buffer + bytesReturned, fcbArray[fd].buf + fcbArray[fd].index, lastBytes);

		bytesReturned += lastBytes;
		fcbArray[fd].fileOffset += lastBytes;
		fcbArray[fd].index += lastBytes;
		fcbArray[fd].buflen = fcbArray[fd].buflen - lastBytes;
	}

	// Return total bytes copied into output buffer
	return bytesReturned;
}
	
// Interface to Close the file	
void b_close (b_io_fd fd)
	{
		if(fcbArray[fd].fileInfo != NULL) {
			free(fcbArray[fd].fileInfo);
			fcbArray[fd].fileInfo = NULL;
		}
		if(fcbArray[fd].buf != NULL) {
			free(fcbArray[fd].buf);
			fcbArray[fd].buf = NULL;
		}
	}

int getBlockOffset(off_t fileOffset) {
	return fileOffset / getSizeofBlocks();
}

int getOffsetWithinBlock(off_t fileOffset) {
	return fileOffset % getSizeofBlocks();
}
