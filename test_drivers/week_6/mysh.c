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

Output: returns 0 upon successful execution
--- */
int main(int argc, char *argv[], char *envp[])
{
    Job job;
    int exitShell = FALSE_VALUE;

    initialize_signal_handler();
    get_job(&job);

    while (!exitShell) {
        remove_zombies();

        if (job.num_stages == FALSE_VALUE) {
            get_job(&job);
            continue;
        }

        expand_variables(job.pipeline[FALSE_VALUE].argv, envp);

        if (mystrcmp(job.pipeline[FALSE_VALUE].argv[FALSE_VALUE], "exit") == FALSE_VALUE) {
            int status = FALSE_VALUE;
            if (job.pipeline[FALSE_VALUE].argv[TRUE_VALUE])
                status = myatoi(job.pipeline[FALSE_VALUE].argv[TRUE_VALUE]);
            free_all();
            _exit(status);
        }

        if (mystrcmp(job.pipeline[FALSE_VALUE].argv[FALSE_VALUE], "cd") == FALSE_VALUE) {
            handle_cd(job.pipeline[FALSE_VALUE].argv, envp);
            get_job(&job);
            continue;
        }

        if (mystrcmp(job.pipeline[FALSE_VALUE].argv[FALSE_VALUE], "export") == FALSE_VALUE) {
            handle_export(job.pipeline[FALSE_VALUE].argv, envp);
            get_job(&job);
            continue;
        }

        if (mystrcmp(job.pipeline[FALSE_VALUE].argv[FALSE_VALUE], "fg") == FALSE_VALUE) {
            builtin_fg(NULL_PTR);
            get_job(&job);
            continue;
        }

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
  Reaps any terminated child processes to prevent zombie processes
  from accumulating. Uses a non-blocking wait to clean up all children
  that have exited.

Input:
  None

Output:
  None
--- */
static void remove_zombies(void)
{
    int status;
    while (waitpid(WAIT_ANY_CHILD, &status, WNOHANG) > FALSE_VALUE) {}
}

static void sigchld_handler(int sig)
{
    int status;
    int pid;
    while ((pid = waitpid(WAIT_ANY_CHILD, &status, WNOHANG | WUNTRACED | WCONTINUED)) > FALSE_VALUE) {}
}
