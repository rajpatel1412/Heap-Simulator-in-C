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
    if(ptr == NULL && size > 0) 
        mm_malloc(size);
    else if(ptr != NULL && size == 0)
        mm_free(ptr);
    
    BlockInfo * blockInfo = ((BlockInfo *) UNSCALED_POINTER_SUB(ptr, WORD_SIZE));
    size_t oldSize = blockInfo->sizeAndTags;
    size_t precedingTagUsed = blockInfo->sizeAndTags & TAG_PRECEDING_USED;
    //size_t tagUsed = blockInfo->sizeAndTags & TAG_USED;
    
    //examine_heap();

    if(oldSize == size) // if oldsize and new size are the same, do nothing and return the pointer
        return UNSCALED_POINTER_ADD(blockInfo, WORD_SIZE);
    else if(oldSize > size) // if new size is less than oldsize, update the header, add the now free block into the freelist if >+ MIN_SIZE
    {
        if(size < MIN_BLOCK_SIZE) // new size should be at least the minblock size
            size = MIN_BLOCK_SIZE;
        size_t freeSize = oldSize - size; // size of freeblock
        blockInfo->sizeAndTags = size | precedingTagUsed; //current allocated block header updated
        BlockInfo * freeBlock = UNSCALED_POINTER_ADD(blockInfo, size); // pointer to recently freed block
        freeBlock->sizeAndTags = freeSize | TAG_PRECEDING_USED; //setting header of free block
        BlockInfo* nextBlock = UNSCALED_POINTER_ADD(freeBlock, freeSize);
        nextBlock->sizeAndTags ^= TAG_PRECEDING_USED; // updating header of the next block to show previous block is now free
        ((BlockInfo*)UNSCALED_POINTER_SUB(nextBlock, WORD_SIZE))->sizeAndTags = freeBlock->sizeAndTags; // updating footer of freeblock
        insertFreeBlock(freeBlock); // inserting into freelist
        coalesceFreeBlock(freeBlock); // coalescing free blocks if need be
        return UNSCALED_POINTER_ADD(blockInfo, WORD_SIZE);
    }
    else 
    {
        if(((((BlockInfo*) UNSCALED_POINTER_ADD(blockInfo, oldSize))->sizeAndTags) & TAG_USED) == 1) // if next block is not free call malloc, move everything over and free old block
        {
            BlockInfo* newBlock = mm_malloc(size);
            for(int i = 1; i < oldSize; i++)
                newBlock[i-1] = blockInfo[i];
            mm_free(blockInfo);
            return newBlock;
        }
        else // there is a free block next to the block we are interested in
        {
            BlockInfo* freeBlock = UNSCALED_POINTER_ADD(blockInfo, oldSize);
            if((SIZE(blockInfo->sizeAndTags) + SIZE(freeBlock->sizeAndTags)) < size) //the new size requested is more than the free and current sizes
            {
                BlockInfo* newBlock = mm_malloc(size);
                for(int i = 1; i < oldSize; i++)
                    newBlock[i-1] = blockInfo[i];
                mm_free(blockInfo);
                return newBlock;
            }
            else // there is a free block and it is large enough to accomodate the new size constraint
            {
                size_t totalSize = oldSize + SIZE(freeBlock->sizeAndTags);
                blockInfo->sizeAndTags = size | precedingTagUsed;
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
                    insertFreeBlock(splitBlock);
                    coalesceFreeBlock(splitBlock);
                    removeFreeBlock(freeBlock);
                    return UNSCALED_POINTER_ADD(blockInfo, WORD_SIZE);
                }
                    
            }
        }
    }


    return NULL;
}
