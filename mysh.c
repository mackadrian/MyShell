#include <stdlib.h>
#include <unistd.h>
#include "mystring.h"
#include "jobs.h"
#include "myheap.h"
#include "getjob.h"
#include "runjob.h"  
#include "signal.h"

void remove_zombies()
{
  int status;
  while (waitpid(-1, &status, WNOHANG) > 0)
    {}
}

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

    // Install signal handlers
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, NULL); // handle ^C
    signal(SIGTSTP, SIG_IGN); // ignore ^Z
    
    /* initialize first input */
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

        run_job(&job);
        free_all();
        get_job(&job);
    }

    return 0;
}
