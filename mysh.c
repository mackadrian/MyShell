#include "mystring.h"
#include "jobs.h"
#include "myheap.h"
#include "getjob.h"
#include "runjob.h"  
#include "signal.h"
#include "mysh.h"
#include "builtin.h"

#include <stdlib.h>
#include <unistd.h>

int fg_job_status = FALSE_VALUE;

/* ---
Function Name: main

Purpose: 
  Runs the personal MYSH shell program.

Input:
arc  - number of command-line arguments
argv - array of command-line argument strings
envp - array of environment variable strings

Returns EXIT_SUCCESS upon normal shell termination.
--- */
int main(int argc, char *argv[], char *envp[])
{
    Job job;
    int exitShell = FALSE_VALUE;

    int shell_pgid = getpid();
    setpgid(shell_pgid, shell_pgid);
    tcsetpgrp(STDIN_FILENO, shell_pgid);
    
    initialize_signal_handler();
    get_job(&job);

    while (!exitShell) {
        remove_zombies();

        /* Ignore empty input lines*/
        if (job.num_stages == FALSE_VALUE) {
            get_job(&job);
            continue;
        }

        expand_variables(job.pipeline[INITIAL_INDEX].argv, envp);
        /* Built-in Exit */
        if (mystrcmp(job.pipeline[INITIAL_INDEX].argv[INITIAL_INDEX], CMD_EXIT) == FALSE_VALUE) {
            int status = FALSE_VALUE;
            if (job.pipeline[INITIAL_INDEX].argv[INITIAL_INDEX + 1])
                status = myatoi(job.pipeline[INITIAL_INDEX].argv[INITIAL_INDEX + 1]);
            free_all();
            _exit(status);
        }
        /* Built-in cd */
        if (mystrcmp(job.pipeline[INITIAL_INDEX].argv[INITIAL_INDEX], CMD_CD) == FALSE_VALUE) {
            handle_cd(job.pipeline[INITIAL_INDEX].argv, envp);
            get_job(&job);
            continue;
        }
        /* Built-in export */
        if (mystrcmp(job.pipeline[INITIAL_INDEX].argv[INITIAL_INDEX], CMD_EXPORT) == FALSE_VALUE) {
            handle_export(job.pipeline[INITIAL_INDEX].argv, envp);
            get_job(&job);
            continue;
        }
	/* Built-in jobs*/
	if (mystrcmp(job.pipeline[INITIAL_INDEX].argv[INITIAL_INDEX], CMD_JOBS) == FALSE_VALUE) {
	  handle_jobs(job.pipeline[INITIAL_INDEX].argv);
	  get_job(&job);
	  continue;
	}
        /* Built-in fg */
        if (mystrcmp(job.pipeline[INITIAL_INDEX].argv[INITIAL_INDEX], CMD_FG) == FALSE_VALUE) {
            builtin_fg(NULL_PTR);
            get_job(&job);
            continue;
        }
        /* Built-in bg */
        if (mystrcmp(job.pipeline[INITIAL_INDEX].argv[INITIAL_INDEX], CMD_BG) == FALSE_VALUE) {
            builtin_bg(NULL_PTR);
            get_job(&job);
            continue;
        }

        run_job(&job, envp);
        free_all();
        get_job(&job);
    }

    return EXIT_SUCCESS;
}


/* ---
Function Name: remove_zombies

Purpose:
  Reaps any terminated child processes to prevent accumulation of
  zombie processes. Uses a non-blocking waitpid() call to continuously
  clean up background jobs that have completed.

Input:
  None

Output:
  Reclaims resources of finished child processes.
--- */
static void remove_zombies(void)
{
    int status;
    while (waitpid(WAIT_ANY_CHILD, &status, WNOHANG) > FALSE_VALUE) {}
}
