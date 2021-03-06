/**************************************************************
* Class:  CSC415
* Name: Abhijai Singh
Luis Alfaro
Michael Schroeder
Peter Ijeoma

* Student ID: 
* Project: Basic File System
*
* File: bitMap.c
*
* Description: This file deals with the free space creation
*           and management, utilizing functions to initialize
*	        the free space, fetch a free contiguous set of
*           blocks, and to return a set of contiguous blocks.
*           It also includes a function  that will defrag-
*           ment the freespace by rearranging directories.
*
**************************************************************/

#include "mfs.h"

int memory_map_init(int starting_block)
{

    int block_count = MBR_st->totalBlockLBA;
    int bitmap_size_in_bytes = block_count * sizeof(_Bool);
    int bitmap_size_in_blocks = (bitmap_size_in_bytes / MBR_st->blockSize) + 1;

    _Bool *bitmap = (_Bool *)calloc(bitmap_size_in_blocks * MBR_st->blockSize, sizeof(_Bool));
    if (bitmap == NULL)
    {
        return -1;
    }
    for (int i = 0; i < bitmap_size_in_blocks; i++)
    {
        bitmap[i] = 1;
    }
    LBAwrite(bitmap, bitmap_size_in_blocks, starting_block);
    free(bitmap);
    int result = bitmap_size_in_blocks + starting_block;
    return result;
}

int find_free_index(int blocks_needed)
{
    int block_count = MBR_st->totalBlockLBA;
    int bitmap_size_in_bytes = block_count * sizeof(_Bool);
    int bitmap_size_in_blocks = (bitmap_size_in_bytes / MBR_st->blockSize) + 1;
    _Bool *bitmap = (_Bool *)calloc(bitmap_size_in_blocks * MBR_st->blockSize, sizeof(_Bool));
    if (bitmap == NULL)
    {
        return -1;
    }
   
    LBAread(bitmap, bitmap_size_in_blocks, MBR_st->freeSpacePos);
    int free_blocks = 0;
    int index = 0;
    int result = -1;

    while (index < bitmap_size_in_bytes)
    {
        if (bitmap[index] == 0)
        {
            free_blocks++;
            if (free_blocks == blocks_needed)
            {
                result = index - blocks_needed + 1;
                for (int i = result; i <= index; i++)
                {
                    bitmap[i] = 1;
                }
                LBAwrite(bitmap, bitmap_size_in_blocks, MBR_st->freeSpacePos);
                return result;
            }
        }
        else
        {
            free_blocks = 0;
        }
        index++;
    }

    printf("Out of space/n");
    return (-1);
}
int free_mem(int index, int count)
{

    int block_count = MBR_st->totalBlockLBA;
    int bitmap_size_in_bytes = block_count * sizeof(_Bool);
    int bitmap_size_in_blocks = (bitmap_size_in_bytes / MBR_st->blockSize) + 1;
    _Bool *bitmap = (_Bool *)calloc(bitmap_size_in_blocks * MBR_st->blockSize, sizeof(_Bool));
    if (bitmap == NULL)
    {
        return -1;
    }
    LBAread(bitmap, bitmap_size_in_blocks, MBR_st->freeSpacePos);
    for (int i = index; i <= index + count; i++)
    {
        bitmap[i] = 0;
    }
    LBAwrite(bitmap, bitmap_size_in_blocks, MBR_st->freeSpacePos);
}

int defrag(int start)
{

    
    int block_count = MBR_st->totalBlockLBA;
    // int bitmap_size_in_bytes = block_count * sizeof(_Bool);
    int bitmap_size_in_bytes = 19000;
    int bitmap_size_in_blocks = (bitmap_size_in_bytes / MBR_st->blockSize) + 1;
    _Bool *bitmap = (_Bool *)calloc(bitmap_size_in_blocks * MBR_st->blockSize, sizeof(_Bool));
   
    if (bitmap == NULL)
    {
        return -1;
    }
    LBAread(bitmap, bitmap_size_in_blocks, MBR_st->freeSpacePos);
    int index = start; 
    int fragStart = -1; 
    int fragEnd = -1;
    int fragSize = 0;  
    while (index < bitmap_size_in_bytes - fragSize)
    {
        
        if (bitmap[index] == 0)
        {
            if (fragStart < 0) {
                fragStart = index; 
            }
        }
        else if (bitmap[index] == 1)
        {
            if (fragStart > 0 && fragEnd < 0) {
                fragEnd = index - 1; 
                fragSize = fragEnd - fragStart + 1 ;
                int index2 = fragStart; 
                while (index2 <= fragEnd)
                {
                    bitmap[index2] = bitmap[index2 + fragSize - 1];
                    index2++; 
                }
                
                
            }
        }
        if (fragStart > 0) {
            bitmap[index] = bitmap[index + fragSize - 1]; 
        }
        index++;
    }
    if (fragEnd < 0) {
        printf("Defragmentation complete...\n"); 
        return 0; 
    }
    //printf("START END, %d, %d", fragStart, fragEnd); 
    defrag_helper(NULL, fragSize - 1, fragStart);
    int toMove = bitmap_size_in_bytes - fragEnd;
    //printf("TO MOVE %d", toMove);  
    void *moveBuffer = malloc(toMove * MBR_st -> blockSize);
    if (moveBuffer == NULL) {
        printf("PROBLEM WITH MALLOC"); 
    }
    LBAread(moveBuffer, toMove, fragEnd); 
    LBAwrite(moveBuffer, toMove, fragStart); 
    LBAwrite(bitmap, bitmap_size_in_blocks, MBR_st->freeSpacePos);
    defrag(fragStart); 
}

int defrag_helper(dirEntry *startdE, int offset, int fragStart)
{
    //set toRemove to be empty 
    dirEntry *entryBuff = (dirEntry *)malloc(MBR_st->dirBufMallocSize);
    int blocks = MBR_st->dirNumBlocks;
    int originalChild = 0;
    if (startdE == NULL) {
        LBAread(entryBuff, blocks, MBR_st -> rootDirectoryPos);
    } else {
        originalChild = startdE->childLBA;
        LBAread(entryBuff, blocks, originalChild);
    }
    for (int i = 0; i < STARTING_NUM_DIR; i++) {
        if (entryBuff[i].isBeingUsed == 1) {
            if (i > 1 && entryBuff[i].type == 'D') {
                defrag_helper(&entryBuff[i], offset, fragStart);
            } else if (i > 1){
                shiftExtents(&entryBuff[i], offset, fragStart);
                
            }
            if (entryBuff[i].childLBA > fragStart) {
                entryBuff[i].childLBA = entryBuff[i].childLBA - offset;
            }
            
            if (entryBuff[i].locationLBA > fragStart) {
                    entryBuff[i].locationLBA = entryBuff[i].locationLBA - offset; 
            }
            
            
        }
    }
    if (startdE == NULL) {
        LBAwrite(entryBuff, blocks, MBR_st -> rootDirectoryPos);
    } else {
        LBAwrite(entryBuff, blocks, originalChild);
    }
}
