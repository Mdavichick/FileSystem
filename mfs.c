/**************************************************************
* Class:  CSC-415-01 Fall 2021
* Names: Michael Davichick, Jeffery Friedrich, Brian Cheng , Ives-Christian Jadman 
* Student IDs:920196625 , 920095797, 918757468, 920450086
* GitHub Name: Ijadman
* Group Name: No Preference
* Project: Basic File System
*
* File: mfs.c
*
* Description: Handles directory commands from fsshell
*
**************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mfs.h"
#include "fsParse.h"
#include "fsString.h"

// extra functions

int fs_mkdir(const char *pathname, mode_t mode)
{
    int retValue = 0;
    char *nameBuffer = malloc(sizeof(char) * NAMESIZE);
    Directory *parentDir = parsePath(pathname, nameBuffer);
    if (parentDir == NULL)
    {
        printf("Path is invalid\n");
        retValue = -1;
    }
    else if (searchDirectory(parentDir, nameBuffer) != NULL)
    {
        printf("A Directory Entry with that name exists already\n");
        retValue = -1;
    }
    else
    {
        DirEntry *newDirEntry = createDirEntry(nameBuffer, sizeof(Directory), 1);
        retValue = assignDirEntryToDirectory(newDirEntry, parentDir);
        Directory *newDirectory = createDirectory(newDirEntry, parentDir);
        writeDirectory(newDirectory);
        writeDirectory(parentDir);
        free(newDirEntry);
    }
    free(nameBuffer);
    freeDirectoryPtr(parentDir);
    return retValue;
}

int fs_rmdir(const char *pathname)
{
    char *nameBuffer = malloc(sizeof(char) * NAMESIZE);
    Directory *parentDir = parsePath(pathname, nameBuffer);
    if (parentDir == NULL)
    {
        printf("Path is invalid\n");
        free(nameBuffer);
        return -1;
    }
    DirEntry *oldDirEntry = searchDirectory(parentDir, nameBuffer);
    free(nameBuffer);

    if (oldDirEntry == NULL || oldDirEntry->isDir == 0)
    {
        printf("A Directory with that name does not exist\n");
        freeDirectoryPtr(parentDir);
        return -1;
    }
    Directory *oldDirectory = readDirEntry(oldDirEntry);
    if (isDirectoryEmpty(oldDirectory) == 0)
    {
        printf("Directory is not empty. Cannot delete\n");
        return -1;
    }

    deleteDirectory(oldDirectory);
    unassignDirEntry(oldDirEntry);
    writeDirectory(parentDir);

    free(oldDirectory);
    freeDirectoryPtr(parentDir);
    return 0;
}

// Directory iteration functions
fdDir *fs_opendir(const char *pathname)
{
    // check if path to name is valid
    // set fdDir * to point to directory stream
    // stream is at first position of first entry in directory
    // at directory.dirEntryArray[0]

    // initialize???
    // myDir->d_reclen;
    // myDir->dirEntryPosition;
    // myDir->
    Directory *dir = NULL;
    if (strncmp(pathname, "/", 2) != 0)
    {
        dir = (Directory *)getObjectFromPath(pathname);
        if (dir == NULL)
        {
            printf("Failed to open directory\n");
            return NULL;
        }
    }
    else
    {
        dir = getRootDirectory();
    }

    fdDir *returnDirInfo = malloc(sizeof(fdDir));
    struct fs_diriteminfo **itemArray = malloc(sizeof(struct fs_diriteminfo *) * MAXDIRENTRIES);
    returnDirInfo->dirItemArray = itemArray;
    int i = 0;
    while (i < MAXDIRENTRIES && dir->dirArray[i].isFree == 0)
    {
        struct fs_diriteminfo *dirInfo = malloc(sizeof(struct fs_diriteminfo));
        strncpy(dirInfo->d_name, dir->dirArray[i].name, NAMESIZE);
        dirInfo->d_reclen = dir->dirArray[i].dirEntBlockInfo.size * getSizeofBlocks();
        dirInfo->fileType = dir->dirArray[i].isDir;
        returnDirInfo->dirItemArray[i] = dirInfo;
        i++;
    }
    returnDirInfo->d_reclen = i;
    returnDirInfo->dirEntryPosition = 0;
    returnDirInfo->directoryStartLocation = dir->dirArray[0].dirEntBlockInfo.currentBlock;

    setCWD(dir);
    return returnDirInfo;
}

struct fs_diriteminfo *fs_readdir(fdDir *dirp)
{
    if (dirp->dirEntryPosition < dirp->d_reclen)
    {
        return dirp->dirItemArray[dirp->dirEntryPosition++];
    }
    return NULL;
}

int fs_closedir(fdDir *dirp)
{
    for (int i = 0; i < dirp->d_reclen; i++)
    {
        free(dirp->dirItemArray[i]);
    }
    free(dirp->dirItemArray);
    free(dirp);
    return 0;
}

// Misc directory functions
char *fs_getcwd(char *buf, size_t size)
{
    return getCWDPath();
}

int fs_setcwd(char *buf) //linux chdir
{
    // set cwd to whatever this buffer is
    // parse path, see if valid
    // if valid
    // for ".."
    // parse path: cd ..
    // path is relative
    // we need ..

    // get last directory
    // set cwd pointer to that directory
    // if not valid
    // directory did not exist
    // or path was invalid
    // cwd = buf

    // cd: if path is absolute,
    // setcwd to the absolute
    // cd: if path is relative,
    // concat path to current
    if (strncmp(buf, ".", 2) == 0)
    {
        return 0;
    }

    if (strncmp(buf, "/", 2) == 0)
    {
        setCWDPath("/");
        return setCWD(getRootDirectory());
    }

    Directory *newCWD = getObjectFromPath(buf);
    if (newCWD == NULL)
    {
        printf("Could not find new working directory\n");
        return -1;
    }

    int setRet = setCWD(newCWD);
    if (setRet == 0)
    {
        if (strncmp(buf, "/", 1) != 0)
        {
            char *newCWDPath = malloc(sizeof(char) * MAXSTRINGLENGTH);
            int offset = 0;
            strncpy(newCWDPath, getCWDPath(), MAXSTRINGLENGTH);
            while (strncmp(buf + offset, "../", 3) == 0)
            {
                strRemoveLastElement(newCWDPath, newCWDPath);
                offset += 3;
            }
            if (strncmp(buf + offset, "..", 3) == 0)
            {
                strRemoveLastElement(newCWDPath, newCWDPath);
                setCWDPath(newCWDPath);
            }
            else
            {
                if (strncmp(getCWDPath(), "/", 1) != 0 || strlen(getCWDPath()) != 1)
                {

                    strcat(newCWDPath, "/");
                }
                setCWDPath(strncat(newCWDPath, buf + offset, DIRMAX_LEN));
            }
            free(newCWDPath);
        }
        else
        {
            setCWDPath(buf);
            printf("%s is now the new cwd\n", buf);
        }
    }
    else
    {
        printf("Failed to change current working directory\n");
        setRet = -1;
    }

    return setRet;
}

int fs_isFile(char *path) //return 1 if file, 0 otherwise
{
    int retValue = 0;
    DirEntry *entry = getEntryFromPath(path);

    if (entry != NULL && entry->isDir == 0)
    {
        retValue = 1;
    }
    free(entry);
    return retValue;
}
int fs_isDir(char *path) //return 1 if directory, 0 otherwise
{
    int retValue = 0;
    DirEntry *entry = getEntryFromPath(path);

    if (entry != NULL && entry->isDir == 1)
    {
        retValue = 1;
    }
    free(entry);
    return retValue;
}
int fs_delete(char *pathname)
{ //removes a file
    char *nameBuffer = malloc(sizeof(char) * NAMESIZE);
    Directory *parentDir = parsePath(pathname, nameBuffer);
    if (parentDir == NULL)
    {
        printf("Path is invalid\n");
        free(nameBuffer);
        return -1;
    }

    DirEntry *entry = searchDirectory(parentDir, nameBuffer);
    free(nameBuffer);

    if (entry == NULL || entry->isDir == 1)
    {
        printf("A File with that name does not exist\n");
        freeDirectoryPtr(parentDir);
        return -1;
    }
    deleteDirEntry(entry);

    unassignDirEntry(entry);
    writeDirectory(parentDir);
    freeDirectoryPtr(parentDir);
    return 0;
}

int fs_stat(const char *pathname, struct fs_stat *buf)
{
    DirEntry *entry = getEntryFromPath(pathname);

    if (entry == NULL)
    {
        printf("A Directory with that name does not exist\n");
        return 1;
    }

    buf->st_size = entry->dirEntBlockInfo.size * getSizeofBlocks(); /* total size, in bytes */
    buf->st_blksize = entry->dirEntBlockInfo.size;                  /* blocksize for file system I/O */
    buf->st_blocks = (buf->st_size + 511) / 512;                    /* number of 512B blocks allocated */
    buf->st_accesstime = entry->dateLastAccessed;                   /* time of last access */
    buf->st_modtime = entry->datelastModified;                      /* time of last modification */
    buf->st_createtime = entry->dateCreated;                        /* time of last status change */

    free(entry);
    return 0;
}