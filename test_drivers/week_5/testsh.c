#include "mystring.h"
#include "jobs.h"
#include "mysh.h"
#include "myheap.h"
#include <unistd.h>
#include <fcntl.h>
#include "runjob.h"
#include "getjob.h"


/* ---
Function Name: main

Purpose: 
  Runs the personal MYSH shell program.

Input:
arc  - number of command-line arguments
argv - array of command-line argument strings
envp - array of environment variable strings

Output: returns 0 upon successful execution
--- */
int main(int argc, char *argv[], char *envp[])
{
  int exitShell = 0;
  Job job;

  get_job(&job);

  while (!exitShell && mystrcmp(job.pipeline[0].argv[0], "exit"))
    {
      run_job(&job);
      get_job(&job);
    }
  
  
  return 0;
}
