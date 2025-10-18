#include "myheap.h"

static char heap[HEAP_SIZE];
static char *freep = heap;

/* ---
Function Name: alloc

Purpose: 
  Allocates a block of memory of the given size from the heap.

Input:
  size - number of bytes to allocate
  
Output:
  p - pointer to the allocated memory block
      returns NULL if not enough space is available
--- */
char *alloc(unsigned int size)
{
  if (freep + size > heap + HEAP_SIZE)
  {
    return NULL;
  }
  char *p = freep;
  freep += size;
  return p;
}


/* ---
Function Name: free_all

Purpose: 
  Frees all memory previously allocated in the heap.

Input:
  none
  
Output:
  All previously allocated memory from the custom heap is invalidated and cleared.
--- */
void free_all()
{
  freep = heap;
}

