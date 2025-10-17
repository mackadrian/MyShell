#ifndef MY_HEAP_H
#define MY_HEAP_H

#include <unistd.h>

#define HEAP_SIZE 10000     /* adjust as necessary */

char *alloc(unsigned int size);
void free_all();

#endif
