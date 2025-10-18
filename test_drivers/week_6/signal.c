#include "signal.h"
#include "jobs.h"
#include "mystring.h"

#include <signal.h>
#include <unistd.h>   // write()
#include <sys/wait.h> // waitpid()

volatile sig_atomic_t fg_job_running = 0;

/* extern globals for background jobs */
extern Job jobs[MAX_JOBS];
extern int num_jobs;

/* forward declaration */
void sigchld_handler(int sig);

/* ---
Function Name: handle_signal

Purpose:
  Handles shell signal events such as SIGINT (Ctrl+C).
  If a foreground job is running, sends SIGINT to terminate it;
  otherwise just prints a newline to maintain clean prompt output.
--- */
void handle_signal(int sig)
{
    if (sig == SIGINT)
    {
        write(STDOUT_FILENO, "\n", 1);
    }
}

/* ---
Function Name: sigchld_handler

Purpose:
  Handles the SIGCHLD signal when background processes change state.
  This function reaps all finished child processes to prevent zombies
  and updates the job list accordingly.
--- */
void sigchld_handler(int sig)
{
    (void)sig; // unused

    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        for (int i = 0; i < num_jobs; i++)
        {
            if (jobs[i].pgid == pid)
            {
                // mark as completed
                jobs[i].background = 0;

                write(STDOUT_FILENO, "[Job Done] ", 11);
                write(STDOUT_FILENO, jobs[i].pipeline[0].argv[0],
                      mystrlen(jobs[i].pipeline[0].argv[0]));
                write(STDOUT_FILENO, "\n", 1);
                break;
            }
        }
    }
}

/* ---
Function Name: initialize_signal_handler

Purpose:
  Installs all custom signal handlers:
   - SIGINT (Ctrl+C)
   - SIGTSTP (Ctrl+Z)
   - SIGCHLD (background job notifications)
--- */
void initialize_signal_handler()
{
    struct sigaction sa_int;
    sa_int.sa_handler = handle_signal;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);

    struct sigaction sa_chld;
    sa_chld.sa_handler = sigchld_handler;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa_chld, NULL);

    signal(SIGTSTP, SIG_IGN); // ignore Ctrl+Z
}
