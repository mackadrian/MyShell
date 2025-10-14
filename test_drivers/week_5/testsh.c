#include "mystring.h"
#include "jobs.h"
#include "mysh.h"
#include "myheap.h"
#include "runjob.h"
#include "getjob.h"

#include <unistd.h>
#include <fcntl.h>

#include <stdio.h> // for testing

/* ---
Function Name: main

Purpose: 
 Tests the personal MYSH shell program.

 **The testing of this MYSH shell program is ONLY applicable to
 Project 1: Week 5 Design and Implementation Guidance!

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

    printf("Running get_job()... \n");
    get_job(&job);

    while (!exitShell) {
        /* skip empty input */
        if (job.num_stages == 0) {
            /* just prompt again */
            get_job(&job);
            continue;
        }

        printf("\nPrint out appended Job structure... \n");
        printf("Press ANY KEY to continue... \n");
        getchar();
        print_job(&job);
        printf("\n");

        /* check if user typed 'exit' */
        if (mystrcmp(job.pipeline[0].argv[0], "exit") == 0)
            break;

        run_job(&job);
        free_all();

        printf("\n");
        printf("Printing out appended Job structure inside SHELL LOOP...\n");
        printf("Press ANY KEY to continue...\n");
        getchar();
        print_job(&job);
        printf("\n");

        /* get next command */
        get_job(&job);
    }

    return 0;
}


/* ---
Function Name: print_job

Purpose: 
  Prints all information contained in a Job structure for debugging 
  purposes. This includes the number of stages, input/output file paths, 
  background execution flag, and all commands in the pipeline with their 
  respective arguments.

Input:
  job - pointer to a Job structure whose contents will be printed.

Output:
  Prints the Job's number of stages, infile/outfile paths, background flag, 
  and the arguments of each Command in the pipeline to standard output.
--- */
void print_job(Job *job)
{
    printf("# of stages: %u\n", job->num_stages);
    printf("Input file: %s\n", job->infile_path ? job->infile_path : "none");
    printf("Output file: %s\n", job->outfile_path ? job->outfile_path : "none");
    printf("Background: %s\n", job->background ? "yes" : "no");

    for (unsigned int i = 0; i < job->num_stages; i++) {
        Command *cmd = &job->pipeline[i];
        printf("Stage %u:", i);
        for (unsigned int j = 0; j < cmd->argc; j++) {
            printf(" \"%s\"", cmd->argv[j]);
        }
        printf("\n");
    }
}
