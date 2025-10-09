#ifndef JOBS_H
#define JOBS_H

#define MAX_ARGS 256
#define MAX_PIPELINE_LEN 2

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
} Job;

#endif
