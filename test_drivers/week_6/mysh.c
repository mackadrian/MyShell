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

	expand_variables(job.pipeline[0].argv, envp);

	if (mystrcmp(job.pipeline[0].argv[0], "exit") == 0) {
            int status = 0;
            if (job.pipeline[0].argv[1])
                status = myatoi(job.pipeline[0].argv[1]);
            free_all();
            _exit(status);
        }
        if (mystrcmp(job.pipeline[0].argv[0], "cd") == 0) {
	  handle_cd(job.pipeline[0].argv, envp);
            get_job(&job);
            continue;
        }
        if (mystrcmp(job.pipeline[0].argv[0], "export") == 0) {
	  handle_export(job.pipeline[0].argv, envp);
            get_job(&job);
            continue;
        }
        if (mystrcmp(job.pipeline[0].argv[0], "fg") == 0) {
            builtin_fg(0);  // for simplicity, pick first job
            get_job(&job);
            continue;
        }
        if (mystrcmp(job.pipeline[0].argv[0], "bg") == 0) {
            builtin_bg(0);  // for simplicity, pick first job
            get_job(&job);
            continue;
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
  while (waitpid(-1, &status, WNOHANG) > 0) {}
}

static void sigchld_handler(int sig) {
  int status;
  int pid;
  while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0) {}
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
