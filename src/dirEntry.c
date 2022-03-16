/**************************************************************
* Class:  CSC415 
* Name: Abhijai Singh
Luis Alfaro
Michael Schroeder
Peter Ijeoma

* Student ID: 
* Project: File System
*
* File: dirEntry.c
*
* Description: This file implements functions in dirEntry.h
*        Creates and modifies a directory entry
*		 Also implements extent features  that manage the
*		 allocation of memory for files.
*
**************************************************************/

#include "mfs.h"

void initEntry(dirEntry *dE)
{
	//initialize location variables
	dE->dataLocation = DEFAULT_LBA; //valid data location will be between block 0-19531
	dE->locationLBA = DEFAULT_LBA;  //location of this entry in logical block
	dE->entryIndex = -1;	  //the position of this entry in the array of entries
	dE->childLBA = DEFAULT_LBA;

	//initialize a default name for this child
	dE->name[0] = '%';
	dE->name[1] = '\0';

	dE->sizeOfFile = DEFAULT_SIZE; // the number of bytes of the file data
	dE->numBlocks = DEFAULT_SIZE;	// the number of blocks occupied by the file
	//dE->id = -1; //the id number for the entry

	time(&(dE->dateCreated));  // date the file was created
	time(&(dE->dateModified)); // date the file was last modified
	time(&(dE->dateAccessed)); // date the file was last accessed

	//initialize extent block 
	dE->extents = DEFAULT_LBA;
	dE->numExtents = DEFAULT_SIZE;
	dE->numExtentBlocks = DEFAULT_SIZE;

	dE->locationMetadata = DEFAULT_LBA; //512 file per directory
	dE->isBeingUsed = 0;		  //this file is currently not being used
	dE->type = 'D';				  //initially this will be a directory until datalocation is != DEFAULT_LBA
}

/*
This function will take an fd_struct object, and a directory entry and updates
certain information pertaining to the modification of file data.
*/
int updateEntry(int fd, dirEntry* dE)
{
	short entryIndex = dE->entryIndex;

	//if we have a valid fd and dE
	if((fd > -1) && dE) {
		//create a buffer for directory with dE in it
		dirEntry* buf = (dirEntry*)malloc(MBR_st->dirBufMallocSize);
		if(!buf){
			printf("Malloc failed ln61. returning 1\n");
			return 1;
		}
		LBAread(buf, MBR_st->dirNumBlocks, dE->locationLBA);

		//copy over important data
		buf[entryIndex].sizeOfFile = fileOpen[fd].sizeOfFile;
		buf[entryIndex].numBlocks = fileOpen[fd].numBlocks;

		//modify the time stamps
	   	time(&(dE->dateModified)); // date the file was last modified
    	time(&(dE->dateAccessed)); // date the file was last accessed

		//if fd is about to be closed
		if(fileOpen[fd].flaggedForClose) {
			if(dE->numBlocks < dE->numExtentBlocks) {
				//returnWastedExtents(dE); //give back wasted extents
			}
		}
		else { 
			if(dE->numBlocks == dE->numExtentBlocks)
				addAnExtent(dE); //add an extent
		}

		//write all updated entry info to disk
		unsigned long written = LBAwrite(buf, MBR_st->dirNumBlocks, dE->locationLBA);

		//free
		if(buf){
			free(buf);
			buf = NULL;
		}

		//printf("entry successfully updated\n");
		return 0;
	}
	printf("error: this entry is null. returning 1\n");
	return 1;
}

int wipeExtents(dirEntry* dE)
{
	//create extents buffer
	unsigned long* ptr = (unsigned long*)malloc(BLOCK_SIZE);
	if(!ptr)
	{
		//printf("error with malloc\n");
		return 1;
	}
	LBAread(ptr, 1, dE->extents);

	//create directory buffer
	dirEntry* buf = (dirEntry*)malloc(MBR_st->dirBufMallocSize);
	if(!buf){
		//printf("error with mallopc\n");
		free(ptr);
		ptr = NULL;
		return 1;
	}
	LBAread(buf, MBR_st->dirNumBlocks, dE->locationLBA);

	//loop through all extents, freeing and setting all to 0
	for(int i = 0; i < EXTENT_MAX_ELEMENTS; i += 2)
	{
		if(ptr[i] == DEFAULT_SIZE) break; //if the extents were not full
		free_mem(ptr[i], ptr[i+1]); //give this extent back to free space
		ptr[i] = DEFAULT_SIZE; //reset the LBA start location of extent
		ptr[i+1] = DEFAULT_SIZE; //reset the number of blocks of extent
	}

	//save the extents blob
	LBAwrite(ptr, 1, dE->extents);

	//set extents to default lba
	dE->extents = DEFAULT_LBA;

	//save directory
	LBAwrite(buf, MBR_st->dirNumBlocks, dE->locationLBA);

	//free
	if(ptr) {
		free(ptr);
		ptr = NULL;
	}
	if(buf) {
		free(buf);
		buf = NULL;
	}

	return 0;
}

int returnWastedExtents(dirEntry* dE)
{

	//create a buffer for our extents array
	unsigned long* ptr = (unsigned long*)malloc(BLOCK_SIZE);
	if(!ptr) {
		printf("Malloc failed!\n");
		return 1;
	}
	dirEntry* buf = (dirEntry*)malloc(MBR_st->dirBufMallocSize);
	if(!buf) {
		printf("Malloc failed!\n");
		return 1;
	}
	LBAread(ptr, 1, dE->extents); //read the extents blob to buffer
	LBAread(buf, MBR_st->dirNumBlocks, dE->locationLBA); //read a directory to buffer

	unsigned long blocksToReturn = dE->numExtentBlocks - dE->numBlocks;

	//find last element in our extents array blob
	unsigned long lastElem = (dE->numExtentBlocks * 2) - 1;

	//find the actual LBA block start that we're going to free
	unsigned long lbaToReturn = ptr[lastElem - 1];
	lbaToReturn += (ptr[lastElem] - blocksToReturn);
	free_mem(lbaToReturn, blocksToReturn); //free the blocksToReturn
	ptr[lastElem] -= blocksToReturn; //decrement the last element in our extents blob by blocksToReturn
	dE->numExtentBlocks -= blocksToReturn; //decrement numExtentBlocks
	LBAwrite(ptr, 1, dE->extents); //do LBAwrite of file blob to save changes
	LBAwrite(buf, MBR_st->dirNumBlocks, dE->locationLBA); //do LBAwrite of directory to save new entry changes

	if(ptr){
		free(ptr);
		ptr = NULL;
	}
	if(buf) {
		free(buf);
		buf = NULL;
	}
	return 0;
}


// helper function to resolve a logical extent element into an LBA block?                                           
unsigned long getExtentLBA(int fd, _Bool isForWrite)
{
	//TODO
	}


unsigned long initExtents(dirEntry* dE)
{
	//TODO
}

unsigned long addAnExtent(dirEntry* dE) 
{
//TODO
}

/*GETTERS TO BE IMPLEMENTED HERE*/
//TODO: getters for LBA and Index files


/*SETTERS TO BE IMPLEMENTED HERE*/
//TODO: setters for LBA and Index files


