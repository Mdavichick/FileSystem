/**************************************************************
* Class:  CSC-415-01 Fall 2021
* Names: Michael Davichick, Jeffery Friedrich, Brian Cheng , Ives-Christian Jadman 
* Student IDs:920196625 , 920095797, 918757468, 920450086
* GitHub Name: Ijadman
* Group Name: No Preference
* Project: Basic File System
*
* File: fsString.c
*
* Description: Implements a function for reversing strings
* Referenced: https://www.daniweb.com/programming/software-development/threads/311002/strrev-equivalent-in-linux
*
**************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "fsString.h"

char *delim = "/";

int str_rev(char *oldString, char *newStringBuffer)
{
    int backwardJ = strnlen(oldString, MAXSTRINGLENGTH) - 1;
    int forwardI = 0;

    while (backwardJ >= 0)
    {
        newStringBuffer[forwardI++] = oldString[backwardJ--];
    }
    newStringBuffer[forwardI] = '\0';

    return forwardI;
}

int strRemoveLastElement(char *oldString, char *newStringBuffer)
{
    char *intermediateStringBuffer = malloc(sizeof(char) * strnlen(oldString, MAXSTRINGLENGTH));
    char *temp = oldString;

    str_rev(oldString, intermediateStringBuffer);

    strtok(intermediateStringBuffer, delim);

    int rmTok = strlen(intermediateStringBuffer) + 1;

    char *rootCheck = strtok(NULL, delim);

    if (rootCheck == NULL)
    {
        strncpy(intermediateStringBuffer, "/", 2);
    }
    else
    {

        for (int i = 0; i < rmTok; i++)
        {
            intermediateStringBuffer[i] = '\0';
        }

        strncat(intermediateStringBuffer, oldString, strlen(oldString) - rmTok);
    }

    char *temp2 = malloc(sizeof(char) * strnlen(intermediateStringBuffer, MAXSTRINGLENGTH));
    str_rev(intermediateStringBuffer, temp2);
    int length = str_rev(temp2, newStringBuffer);

    free(intermediateStringBuffer);
    free(temp2);

    return length;
}