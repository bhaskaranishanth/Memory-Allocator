/*! \file
 * Implementation of a simple memory allocator.  The allocator manages a small
 * pool of memory, provides memory chunks on request, and reintegrates freed
 * memory back into the pool.
 *
 * Adapted from Andre DeHon's CS24 2004, 2006 material.
 * Copyright (C) California Institute of Technology, 2004-2010.
 * All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include "myalloc.h"


/*!
 * These variables are used to specify the size and address of the memory pool
 * that the simple allocator works against.  The memory pool is allocated within
 * init_myalloc(), and then myalloc() and free() work against this pool of
 * memory that mem points to.
 */
int MEMORY_SIZE;
unsigned char *mem;

static unsigned char *freeptr;
static unsigned char *locPtr;

/*!
 * This function initializes both the allocator state, and the memory pool. It
 * must be called before myalloc() or myfree() will work at all.
 *
 * Note that we allocate the entire memory pool using malloc().  This is so we
 * can create different memory-pool sizes for testing.  Obviously, in a real
 * allocator, this memory pool would either be a fixed memory region, or the
 * allocator would request a memory region from the operating system (see the
 * C standard function sbrk(), for example). 
 */
void init_myalloc() {
    /*
     * Allocate the entire memory pool, from which our simple allocator will
     * serve allocation requests. This entire operation obviously occurs in
     CONSTANT TIME or O(1).
     */
    mem = (unsigned char *) malloc(MEMORY_SIZE);
    if (mem == 0) {
        fprintf(stderr,
                "init_myalloc: could not get %d bytes from the system\n",
                MEMORY_SIZE);
        abort();
    }

    //The variable freeptr and locPtr and going to be used as bookkeeping
    //for us throughout the functions. Will hold temporary locations of
    //memory locations. 
    freeptr = mem;
    //The rest of the code is creating a header, footer, and then making
    //a payload with the entire memory size subtracted from the size
    //of the header and footer.
    *(int *)freeptr = MEMORY_SIZE - 2 * sizeof(int);
    freeptr = freeptr + MEMORY_SIZE - sizeof(int);
    *(int *)freeptr = MEMORY_SIZE - 2 * (sizeof(int));

}

/*!
 * Attempt to allocate a chunk of memory of "size" bytes.  Return 0 if
 * allocation fails.
 */
/* Here we are implementing a BEST-FIT STRATEGY. We have a header and 
a footer, and we loop through the entire memory and are finding the
smallest potential block that can satisfy out size specifications. 
WHEN THIS STRATEGY IS GOOD: This strategy is usually optimal because
we are finding the smallest block that can fit our requirements. With
first - fit we might be using memory blocks that are excessively big
for our use and so is not an efficient use of the blocks.
WHEN THIS STRATEGY IS BAD: Memory blocks that are SLIGHTLY bigger than
the size will not be used because of additional header footer space.
Additionally, pretend we had a block that is exactly twice as big 
as our size, using this would be far more effective than using a smaller
block (as best fit would usually do). Also, best fit creates instances
where when we do block-splitting, we have an extremely small extra
memory chunk that essentially becomes useless. 
RUNTIME: This run-time will be O(B) where B is the number of blocks.
In other words it is linear on the size of the blocks. 
*/
unsigned char *myalloc(int size) {
    //The numbers 8 and 4 in the code here on out correspond to the size
    //of either the header,footer, or both respectively.

    //In the beginning freeptr starts at memory, and locptr is null. 
    freeptr = mem;
    locPtr = NULL;
    //We are going to proceed until our freeptr is out of the scope
    //of the memory that we initially allocated.
    while ((freeptr - mem) < MEMORY_SIZE)
    {
        //If locPtr is null, the first available block with sufficient
        //size will be of the current "optimal" block.
        if (locPtr == NULL)
        {
            if ((*(int*)freeptr >= size + 8))
            {
                locPtr = freeptr;
            }
        }   
        //Otherwise, locPtr will only change if we come across a smaller
        //block that is more optimal. 
        else 
        {
            if ((*(int*)freeptr >= size + 8))
            {
                if (*(int*)freeptr <= *(int*)locPtr)
                {
                    locPtr = freeptr;
                }
            }
        }
        //Then, we increment our freeptr to go to the next block. 
        freeptr = freeptr + abs(*(int*)freeptr) + 8;
    }
    //If we couldn't find a suitable block of memory, then we aren't able
    //to allocate memory for it. 
    if ((locPtr == NULL))
    {
        fprintf(stderr, "myalloc: cannot service request of size %d \n"
                , size);
        return (unsigned char *) 0;
    }
    //First we set the header for the extra memory that we had
    *(int *)(locPtr + size + 8) = *(int *)(locPtr) - size - 8;
    //Now we set the header for our allocated memory with a negative 
    //value
    *(int *)locPtr = size * -1;
    //Now we are moving locPtr over to the footer of our allocated
    //memory.
    locPtr = locPtr + size + 4;
    //Now we set the footer for our allocated memory with a negative 
    //value
    *(int *)(locPtr) = size * -1;
    //Now we go to the start of the extra memory
    locPtr = locPtr + 4;
    //Now we set the footer for the extra memory
    *(int *)(*(int *)locPtr + locPtr + 4) = *(int *)locPtr;
    //In the next two steps we move to the start of our usable memory.
    locPtr = locPtr - 4;
    locPtr = locPtr + *(int *)locPtr;
    //We now have the chunk of memory and return the pointer for the
    //first memory location of the chunk. 
    return locPtr;
}
/*!
 * Free a previously allocated pointer. oldptr should be an address returned by
 * myalloc().
 */
 /* Our deallocator runs in CONSTANT TIME. O(1).
    For deallocation we have a header and a footer so we simply look to 
    the left and to the right, make sure they aren't out of the bounds of
    memory, and then if they are free, coalesce them.
 */
void myfree(unsigned char *oldptr) {

    //Here we just mark it as unused now.
    *(int*)(oldptr - 4) = abs(*(int*)(oldptr-4));
    *(int*)(oldptr + *(int*)(oldptr - 4)) = *(int*)(oldptr-4);

    //Now if the block on the right is free, coalesce the two. 
    if (((*(int*)(oldptr + (abs(*(int*)(oldptr - 4))) + 4)) >= 0) 
        && (((((oldptr + (abs(*(int*)(oldptr - 4))) + 4)) 
        != mem + MEMORY_SIZE))))
       {
            //Updating the new header and footer labels. 
            *(int*)(oldptr - 4) += (*(int*)(oldptr + 
                (abs(*(int*)(oldptr - 4)))+ 4));
            *(int*)(oldptr - 4) += 8;
            *(int*)(oldptr + (*(int*)(oldptr - 4))) 
            = *(int*)(oldptr - 4);
       }
    //If the block on the left is free, coalesce the two.
    if ((*(int*)(oldptr - 8)) >= 0 && ((oldptr - 8) > mem))
    {
        //Updating the new header and footer labels. 
        *(int*)(oldptr - (*(int*)(oldptr - 8)) - 12) += \
         (*(int*)(oldptr - 4) + 8);
        *(int*)(oldptr + (*(int*)(oldptr - 4))) = \
        *(int*)(oldptr - (*(int*)(oldptr - 8)) - 12);
    }
    //Now we are done. 
}
