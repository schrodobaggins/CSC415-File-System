/**************************************************************
* Class:  CSC415
* Name: Abhijai Singh
Luis Alfaro
Michael Schroeder
Peter Ijeoma

* Student ID: 
* Project: File System
*
* File: fs_ORC.c
*
* Description: Open, Read, Close functions
*
**************************************************************/
#include "mfs.h"

void outputFdDirCWD(fdDir *dirp)
{
    printf("\n\n-----------------------------------------------\n");
    printf("Printing directory\n");
    printf("-----------------------------------------------");
    printf("Current Working Directory: %s\n", dirp->cwd_path);
    printf("Start Location: %ld\n", dirp->directoryStartLocation);
    printf("Directory Entry Position: %d\n", dirp->dirEntryPosition);
}

