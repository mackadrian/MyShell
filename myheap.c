#include "myheap.h"


#define HEAP_SIZE 10000     /* adjust as necessary */

static char heap[HEAP_SIZE];
static char *freep = heap;


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


void free_all()
{
  freep = heap;
}
