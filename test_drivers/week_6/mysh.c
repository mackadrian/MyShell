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

int fg_job_status = 0;

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

    initialize_signal_handler();
    get_job(&job);

    while (!exitShell) {
        remove_zombies();

        /* Ignore empty input lines*/
        if (job.num_stages == FALSE_VALUE) {
            get_job(&job);
            continue;
        }

        expand_variables(job.pipeline[FALSE_VALUE].argv, envp);
        /* Built-in Exit */
        if (mystrcmp(job.pipeline[FALSE_VALUE].argv[FALSE_VALUE], "exit") == FALSE_VALUE) {
            int status = FALSE_VALUE;
            if (job.pipeline[FALSE_VALUE].argv[TRUE_VALUE])
                status = myatoi(job.pipeline[FALSE_VALUE].argv[TRUE_VALUE]);
            free_all();
            _exit(status);
        }
        /* Built-in cd */
        if (mystrcmp(job.pipeline[FALSE_VALUE].argv[FALSE_VALUE], "cd") == FALSE_VALUE) {
            handle_cd(job.pipeline[FALSE_VALUE].argv, envp);
            get_job(&job);
            continue;
        }
        /* Built-in export */
        if (mystrcmp(job.pipeline[FALSE_VALUE].argv[FALSE_VALUE], "export") == FALSE_VALUE) {
            handle_export(job.pipeline[FALSE_VALUE].argv, envp);
            get_job(&job);
            continue;
        }
        /* Built-in fg */
        if (mystrcmp(job.pipeline[FALSE_VALUE].argv[FALSE_VALUE], "fg") == FALSE_VALUE) {
            builtin_fg(NULL_PTR);
            get_job(&job);
            continue;
        }
        /* Built-in bg */
        if (mystrcmp(job.pipeline[FALSE_VALUE].argv[FALSE_VALUE], "bg") == FALSE_VALUE) {
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

/* ---
Function Name: sigchld_handler

Purpose:
  Handles the SIGCHLD signal, which is sent to the parent process
  whenever a child process changes state (exits, stops, or continues).
  Ensures that all child processes are properly reaped without blocking
  the shell.

Input:
  sig - integer representing the caught signal (expected: SIGCHLD)

Output:
  Cleans up child processes using non-blocking waitpid().
--- */
static void sigchld_handler(int sig)
{
    int status;
    int pid;
    while ((pid = waitpid(WAIT_ANY_CHILD, &status, WNOHANG | WUNTRACED | WCONTINUED)) > FALSE_VALUE) {}
}
