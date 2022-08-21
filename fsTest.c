/**************************************************************
* Class:  CSC-415-01 Fall 2021
* Names: Michael Davichick, Jeffery Friedrich, Brian Cheng , Ives-Christian Jadman 
* Student IDs:920196625 , 920095797, 918757468, 920450086
* GitHub Name: Ijadman
* Group Name: No Preference
* Project: Basic File System
*
* File: fsTest.c
*
* Description: Implements any tests we would like to do for debugging
*
**************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "fsString.h"

// return values:
// testStrings is 1
#define STRINGERROR 1

int testStrings();
int testStrRev(char *oldString, char *expectedString);
int testStrRemoveLastElement(char *oldString, char *expectedString, int lastElementSize);

int testFileSystem()
{
    //initialize return value
    int testReturn = 0;

    // run all tests and record results
    testReturn += testStrings();

    //return test results
    return testReturn;
}

int testStrings()
{
    //initialize return value
    int returnValue = 0;

    // run str_rev tests and record results
    returnValue += testStrRev("/", "/");
    returnValue += testStrRev("even", "neve");
    returnValue += testStrRev("Hello", "olleH");
    returnValue += testStrRemoveLastElement("/", "/", 0);
    returnValue += testStrRemoveLastElement("/foo", "/", 3);
    returnValue += testStrRemoveLastElement("/foo/bar", "/foo", 4);

    //return test results
    return (returnValue != 0) ? STRINGERROR : 0;
}

int testStrRev(char *oldString, char *expectedString)
{
    // variable initialization
    int returnValue = 0;
    int stringSize = strlen(oldString);
    char *newString = malloc(sizeof(char) * (stringSize + 1)); // +1 for null

    // function test
    int strrevValue = str_rev(oldString, newString);

    // check function return and external modified values
    returnValue += (strrevValue != stringSize) ? 1 : 0;
    returnValue += strncmp(newString, expectedString, stringSize);

    // cleanup
    free(newString);

    // return test results
    // printf("testStrRev return value is %i\n\n", returnValue);
    return (returnValue != 0) ? 1 : 0;
}

int testStrRemoveLastElement(char *oldString, char *expectedString, int elementRemovedSize)
{
    // variable initialization
    int returnValue = 0;
    int stringSize = strlen(oldString);
    char *newString = malloc(sizeof(char) * (stringSize + 1)); // +1 for null

    // function test
    int strRemoveValue = strRemoveLastElement(oldString, newString);

    // check function return and external modified values
    returnValue += (strRemoveValue != (stringSize - elementRemovedSize)) ? 1 : 0;
    returnValue += strncmp(newString, expectedString, stringSize);

    // cleanup
    free(newString);

    // return test results
    return (returnValue != 0) ? 1 : 0;
}