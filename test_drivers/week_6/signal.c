#include "signal.h"

#include <signal.h> 
#include <unistd.h> // write

volatile sig_atomic_t fg_job_running = 0;

void handle_signal(int sig)
{
    if (sig == SIGINT) {
        if (fg_job_running) {
            write(STDOUT_FILENO, NEWLINE_STR, NEWLINE_LEN);
        } else {
            write(STDOUT_FILENO, SIGNAL_PROMPT, SIGNAL_PROMPT_LEN);
        }
    }
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
    sa.sa_flags = NO_FLAGS;
    sigaction(SIGINT, &sa, NULL);   /* handle Ctrl+C */
    signal(SIGTSTP, SIG_IGN);       /* ignore Ctrl+Z */
}
