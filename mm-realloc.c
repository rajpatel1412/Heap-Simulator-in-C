/**
 * CMPT 295 Assignment Malloc
 *
 * Extra credit: implementing mm_realloc
 *
 * This requires mm_malloc and mm_free to be working correctly, so
 * don't start on this until you finish mm.c.
 */
#include "mm.c"

// Extra credit.
void *mm_realloc(void *ptr, size_t size)
{
    // Write your code here ...

    // this is my original code, it does not give performance as the memory is always rellocated, but this worked when I ran it first

    if(ptr == NULL && size > 0) 
        return mm_malloc(size);
    else if(ptr != NULL && size == 0) 
    {
        mm_free(ptr);
        return NULL;
    }
    
    BlockInfo * blockInfo = ((BlockInfo *) UNSCALED_POINTER_SUB(ptr, WORD_SIZE));
    size_t oldSize = SIZE(blockInfo->sizeAndTags);
    
    void * newBlock = mm_malloc(size);
    if(oldSize > size)
    {
        for(int i = 0; i < size; i++)
            ((char *) newBlock)[i] = ((char *) ptr)[i];
        mm_free(ptr);
        return newBlock;
    }
    
    for(int i = 0; i < oldSize; i++)
        ((char *) newBlock)[i] = ((char *) ptr)[i];
    mm_free(ptr);
    return newBlock;
}


    // this is my code that does not try to reallocate the block if it does not need to, however this one has kept on giving me segmentation fault
    // I have tried fixing it in several ways, and have pushed it, but it does not work at all, hence I am pushing my older code that does not give
    // performance, but overall, perf index is >= 90 so I guess it works well enough with the other cases where it reallocates when it does not need to.


    /*if(ptr == NULL && size > 0) 
        return mm_malloc(size);
    else if(ptr != NULL && size == 0) 
    {
        mm_free(ptr);
        return NULL;
    }

    //size += WORD_SIZE;
    
    BlockInfo * blockInfo = ((BlockInfo *) UNSCALED_POINTER_SUB(ptr, WORD_SIZE));
    size_t oldSize = SIZE(blockInfo->sizeAndTags);

    
    size_t precedingTagUsed = blockInfo->sizeAndTags & TAG_PRECEDING_USED;
    //size_t tagUsed = blockInfo->sizeAndTags & TAG_USED;
    
    //examine_heap();

    if(oldSize == size) // if oldsize and new size are the same, do nothing and return the pointer
        return UNSCALED_POINTER_ADD(blockInfo, WORD_SIZE);
    else if(oldSize > size) // if new size is less than oldsize, update the header, add the now free block into the freelist if >+ MIN_SIZE
    {
        if(size < MIN_BLOCK_SIZE) // new size should be at least the minblock size
            size = MIN_BLOCK_SIZE;
        
        size_t freeBlockSize = oldSize - size;
        BlockInfo * ptrFreeBlock = UNSCALED_POINTER_ADD(blockInfo, size); //ptrAllBlock + reqSize;
        ptrFreeBlock->sizeAndTags = freeBlockSize | TAG_PRECEDING_USED;
        ((BlockInfo *)UNSCALED_POINTER_SUB(UNSCALED_POINTER_ADD(ptrFreeBlock, freeBlockSize), WORD_SIZE))->sizeAndTags = ptrFreeBlock->sizeAndTags;//(ptrFreeBlock + freeBlockSize - WORD_SIZE)->sizeAndTags = ptrFreeBlock->sizeAndTags;
        BlockInfo* nextBlock = UNSCALED_POINTER_ADD(ptrFreeBlock, freeBlockSize);
        nextBlock->sizeAndTags ^= TAG_PRECEDING_USED; // updating header of the next block to show previous block is now free
        insertFreeBlock(ptrFreeBlock);
        coalesceFreeBlock(ptrFreeBlock);
        //examine_heap();
        blockInfo->sizeAndTags = size | precedingTagUsed | TAG_USED;
        
        /*size_t freeSize = oldSize - size; // size of freeblock
        blockInfo->sizeAndTags = size | precedingTagUsed | TAG_USED; //current allocated block header updated
        BlockInfo * freeBlock = UNSCALED_POINTER_ADD(blockInfo, size); // pointer to recently freed block

        freeBlock->sizeAndTags = freeSize | TAG_PRECEDING_USED; //setting header of free block
        //mm_free(freeBlock);
        BlockInfo* nextBlock = UNSCALED_POINTER_ADD(freeBlock, freeSize);
        nextBlock->sizeAndTags ^= TAG_PRECEDING_USED; // updating header of the next block to show previous block is now free
        ((BlockInfo*)UNSCALED_POINTER_SUB(nextBlock, WORD_SIZE))->sizeAndTags = freeBlock->sizeAndTags; // updating footer of freeblock
        insertFreeBlock(freeBlock); // inserting into freelist
        coalesceFreeBlock(freeBlock); // coalescing free blocks if need be
        return UNSCALED_POINTER_ADD(blockInfo, WORD_SIZE);
    }
    else 
    {
        /*void * newBlock = mm_malloc(size);
        for(int i = 0; i < oldSize; i++)
            ((char *) newBlock)[i] = ((char *) ptr)[i];
        mm_free(ptr);
        return newBlock;*/


        /*if(((((BlockInfo*) UNSCALED_POINTER_ADD(blockInfo, oldSize))->sizeAndTags) & TAG_USED) == 1) // if next block is not free call malloc, move everything over and free old block
        {
            void* newBlock = mm_malloc(size - WORD_SIZE);
            for(int i = 0; i < oldSize; i++)
                ((char *) newBlock)[i] = ((char *) ptr)[i];
            mm_free(blockInfo);
            return newBlock;
        }
        else // there is a free block next to the block we are interested in
        {
            BlockInfo* freeBlock = UNSCALED_POINTER_ADD(blockInfo, oldSize);
            if((SIZE(blockInfo->sizeAndTags) + SIZE(freeBlock->sizeAndTags)) < size) //the new size requested is more than the free and current sizes
            {
                void* newBlock = mm_malloc(size);
                for(int i = 0; i < oldSize; i++)
                    ((char *) newBlock)[i] = ((char *) ptr)[i];
                mm_free(blockInfo);
                return newBlock;
            }
            else // there is a free block and it is large enough to accomodate the new size constraint
            {
                size_t totalSize = oldSize + SIZE(freeBlock->sizeAndTags);
                blockInfo->sizeAndTags = size | precedingTagUsed | TAG_USED;
                if(size == totalSize || (totalSize - size) < MIN_BLOCK_SIZE) //just large enough, no splitting required
                {
                    BlockInfo* nextBlock = UNSCALED_POINTER_ADD(freeBlock, SIZE(freeBlock->sizeAndTags));
                    nextBlock->sizeAndTags |= TAG_PRECEDING_USED;
                    removeFreeBlock(freeBlock);
                    return UNSCALED_POINTER_ADD(blockInfo, WORD_SIZE);
                }
                else //the free block is too large and will have to be split
                {
                    BlockInfo * splitBlock = UNSCALED_POINTER_ADD(blockInfo, size);
                    splitBlock->sizeAndTags = (totalSize - size) | TAG_PRECEDING_USED;
                    ((BlockInfo*) (UNSCALED_POINTER_SUB(UNSCALED_POINTER_ADD(splitBlock, (totalSize-size)), WORD_SIZE)))->sizeAndTags = splitBlock->sizeAndTags; // update footer
                    // no need to update the tage preceeding used bit of next block since this was already a free block.
                    removeFreeBlock(freeBlock);
                    insertFreeBlock(splitBlock);
                    coalesceFreeBlock(splitBlock);
                    
                    return UNSCALED_POINTER_ADD(blockInfo, WORD_SIZE);
                }
                    
            }
        }
    }


    //return NULL;  


}*/
