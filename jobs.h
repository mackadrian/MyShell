#ifndef JOBS_H
#define JOBS_H

#define MAX_ARGS 256

typedef struct
{
  char *argv[MAX_ARGS+1];
  unsigned int argc;
} Command;

#endif
