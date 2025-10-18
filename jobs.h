#ifndef JOBS_H
#define JOBS_H

#define MAX_ARGS 1024
#define MAX_PIPELINE_LEN 10
#define MAX_JOBS 16

typedef struct
{
  char *argv[MAX_ARGS+1];
  unsigned int argc;
} Command;

typedef struct
{
  Command pipeline[MAX_PIPELINE_LEN];
  unsigned int num_stages;
  char *outfile_path;
  char *infile_path;
  int background;
  int pgid;
} Job;

#endif
