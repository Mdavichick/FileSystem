/**************************************************************
* Class:  CSC-415-01 Fall 2021
* Names: Michael Davichick, Jeffery Friedrich, Brian Cheng , Ives-Christian Jadman 
* Student IDs:920196625 , 920095797, 918757468, 920450086
* GitHub Name: Ijadman
* Group Name: No Preference
* Project: Basic File System
*
* File: fsParse.c
*
* Description: implements the functions for parsing the path
*
**************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "fsParse.h"

#include "fsDir.h"

Directory *parsePath(const char *path, char *nameBuffer)
{
    //resets buffer to NULL incase buff isn't empty
    memset(nameBuffer, '\0', sizeof(char) * NAMESIZE);
    char *token;

    char **arguments = malloc(strlen(path) * (sizeof(char *) / 2));
    int counter = 0;

    // added + 1 for null char
    char *tempPath = malloc((sizeof(char) * strlen(path)) + 1);
    strncpy(tempPath, path, strlen(path) + 1);

    // if path[0] = / => it's an absolute path
    if (strncmp(tempPath, "/", 1) == 0)
    {
        
        arguments[counter] = "/";
        counter++;
    }
    token = strtok(tempPath, "/ ,");
    while (token != NULL)
    {
        
        arguments[counter] = token;
        counter++;
        token = strtok(NULL, "/ ,");
        
    }
    

    counter--;
    // printf("[ ");
    // for(int i = 0; i < counter; i ++){
    //     printf("%s ", arguments[i]);
    // }
    // printf("]");

    // Once here: All tokens are held within the array

    // TODO**
    // :traverse through Directories comparing array indices to DirEntry name

    // Create DirEntry pointer
    Directory *tempDir;
    // check argument 0 if = "/"
    if (strncmp(arguments[0], "/", 1) == 0)
    {
        // if it is
        // DirEntry pointer points to Root
        tempDir = getRootDirectory();
    }
    else
    {
        // else
        // DirEntry pointer points to cwd
        tempDir = getCWD();
    }

    DirEntry *tempEntry;
    Directory *oldDir;
    // traverse through DirEntry array to find argument n+1
    // /home/foo
    // * i
    for (int i = 1; i < counter; i++)
    {
        tempEntry = searchDirectory(tempDir, arguments[i]);
        if (tempEntry != NULL && tempEntry->isDir == 1)
        {
            oldDir = tempDir;
            tempDir = (Directory *)readDirEntry(tempEntry);
            freeDirectoryPtr(oldDir);
        }
        else
        {
            freeDirectoryPtr(tempDir);
            free(arguments);
            return NULL;
        }
    }

        strncpy(nameBuffer, arguments[counter], strlen(arguments[counter]));
    free(arguments);
    arguments = NULL;

    return tempDir;
}

Directory *getParentFromPath(const char *path)
{
    char *nameBuffer = malloc(sizeof(char) * NAMESIZE);
    Directory *retObject = parsePath(path, nameBuffer);
    free(nameBuffer);
    return retObject;
}

DirEntry *getEntryFromPath(const char *path)
{
    char *nameBuffer = malloc(sizeof(char) * NAMESIZE);
    Directory *parent = parsePath(path, nameBuffer);
    if (parent == NULL)
    {
        free(nameBuffer);
        return NULL;
    }
    DirEntry *retObject = searchDirectory(parent, nameBuffer);
    free(nameBuffer);
    if (retObject == NULL)
    {
        freeDirectoryPtr(parent);
        return NULL;
    }
    retObject = copyDirEntry(retObject);
    freeDirectoryPtr(parent);
    return retObject;
}

FSM *getFSMFromPath(const char *path)
{
    char *nameBuffer = malloc(sizeof(char) * NAMESIZE);
    Directory *parent = parsePath(path, nameBuffer);
    if (parent == NULL)
    {
        free(nameBuffer);
        return NULL;
    }
    DirEntry *entry = searchDirectory(parent, nameBuffer);
    free(nameBuffer);
    if (entry == NULL)
    {
        freeDirectoryPtr(parent);
        return NULL;
    }
    FSM *retObject = copyEntryFSM(entry);
    freeDirectoryPtr(parent);
    return retObject;
}

void *getObjectFromPath(const char *path)
{
    char *nameBuffer = malloc(sizeof(char) * NAMESIZE);
    Directory *parent = parsePath(path, nameBuffer);
    if (parent == NULL)
    {
        free(nameBuffer);
        return NULL;
    }
    DirEntry *entry = searchDirectory(parent, nameBuffer);
    free(nameBuffer);
    if (entry == NULL)
    {
        freeDirectoryPtr(parent);
        return NULL;
    }
    void *retObject = readDirEntry(entry);
    freeDirectoryPtr(parent);
    return retObject;
}