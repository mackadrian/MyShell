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
    int exitShell = 0;

    initialize_signal_handler();
    get_job(&job);

    while (!exitShell) {
        remove_zombies();
        /* ignore empty input */
        if (job.num_stages == 0) {
            get_job(&job);
            continue;
        }

        if (mystrcmp(job.pipeline[0].argv[0], "exit") == 0) {
            exitShell = 1;
            break;
        }

        run_job(&job, envp);
        free_all();
        get_job(&job);
    }

    return 0;
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
static void remove_zombies()
{
  int status;
  while (waitpid(-1, &status, WNOHANG) > 0)
    {}
}

/* ---
Function Name: initialize_signal_handler

Purpose:
  Installs the shell's signal handlers. Handles Ctrl+C (SIGINT)
  with handle_signal, and ignores Ctrl+Z (SIGTSTP) to prevent
  background suspension of the shell.

Input:
  None

Output:
  None
--- */
void initialize_signal_handler()
{
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);  // handle ^C

    signal(SIGTSTP, SIG_IGN);      // ignore ^Z
}  
