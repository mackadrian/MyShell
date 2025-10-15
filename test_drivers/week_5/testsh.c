#include "mystring.h"
#include "jobs.h"
#include "mysh.h"
#include "myheap.h"
#include "runjob.h"
#include "getjob.h"

#include <unistd.h>
#include <fcntl.h>

#include <stdio.h> // for testing

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mystring.h"
#include "jobs.h"
#include "myheap.h"
#include "getjob.h"
#include "runjob.h"    /* optional, if you want to simulate running jobs */

/* Forward declarations */
void print_job(Job *job);

/* --- 
Function Name: main
Purpose:
  Drives tests for get_job() and parse_stage() by simulating
  how the MYSH shell parses commands and constructs Job structs.
--- */
int main(int argc, char *argv[], char *envp[])
{
    Job job;
    int exitShell = 0;

    printf("=== MYSH get_job() TEST DRIVER ===\n");
    printf("Type any command to test parsing (use 'exit' to quit)\n\n");

    /* initialize first input */
    get_job(&job);

    while (!exitShell) {
        /* ignore empty input */
        if (job.num_stages == 0) {
            get_job(&job);
            continue;
        }

        printf("\n--- Parsed Job Structure ---\n");
        print_job(&job);

        /* if user types exit, stop loop */
        if (mystrcmp(job.pipeline[0].argv[0], "exit") == 0) {
            exitShell = 1;
            break;
        }

        /* simulate running and freeing */
        run_job(&job);
        free_all();

        /* read next command */
        get_job(&job);
    }

    printf("\nExiting MYSH test driver.\n");
    return 0;
}

/* --- 
Function Name: print_job
Purpose:
  Prints all fields from Job and Command structs for verification.
--- */
void print_job(Job *job)
{
    printf("Number of stages: %u\n", job->num_stages);
    printf("Background: %s\n", job->background ? "yes" : "no");
    printf("Input file: %s\n", job->infile_path ? job->infile_path : "none");
    printf("Output file: %s\n", job->outfile_path ? job->outfile_path : "none");

    for (unsigned int i = 0; i < job->num_stages; i++) {
        Command *cmd = &job->pipeline[i];
        printf("Stage %u:", i);
        for (unsigned int j = 0; j < cmd->argc; j++) {
            printf(" \"%s\"", cmd->argv[j]);
        }
        printf("\n");
    }
}
