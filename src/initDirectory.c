/**************************************************************
* Class:  CSC-415
* Name: Abhijai Singh
Luis Alfaro
Michael Schroeder
Peter Ijeoma

* Student ID: 
* Project: File System
*
* File: initDirectory.c
*
* Description: This function initializes the directory
*            	and saves it on disk
*
**************************************************************/

#include "mfs.h" //for LBAWrite()

long initDirectory(int parentLBA)
{

	int i = 0; //iterator for array of entries

	//get an address for the starting block
	int startingBlock = find_free_index(MBR_st->dirNumBlocks);

	//create a space in RAM to start manipulating
	dirEntry *ptr = (dirEntry *)malloc(MBR_st->dirBufMallocSize);
	if (ptr == NULL)
	{
		return -1;
	}
	else

	//fill the root struct with default info
	initEntry(&ptr[i]); 
	ptr[i].locationLBA = startingBlock;
	ptr[i].childLBA = startingBlock;
	ptr[i].entryIndex = 0;
	ptr[i].name[0] = '.';
	ptr[i].name[1] = '\0';
	ptr[i].isBeingUsed = 1;
	ptr[i].type = 'D';
	i++; //increment to next directory


	//initialize an array of directory entries, all set to unused
	for (i; i < STARTING_NUM_DIR; i++)
	{
		initEntry(&ptr[i]);

		//initialize the .. entry
		if (i < 2)
		{
			//set the second entry's name
			ptr[i].name[0] = '.';
			ptr[i].name[1] = '.';
			ptr[i].name[2] = '\0';
			if (parentLBA == 0)
			{
				ptr[i].childLBA = startingBlock;
			}
			else
			{
				ptr[i].childLBA = parentLBA;
			}
			ptr[i].isBeingUsed = 1;
		}
		ptr[i].entryIndex = i;
		ptr[i].locationLBA = startingBlock;
		ptr[i].type = 'D';
	}
	
	//call LBA write to put this directory on disk
	LBAwrite(ptr, MBR_st->dirNumBlocks, startingBlock);

	free(ptr);


	return startingBlock; //returning the location of the directory in LBA
}

