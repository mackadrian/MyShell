#include "signal.h"

#include <signal.h> 
#include <unistd.h> // write

volatile sig_atomic_t fg_job_running = 0;

/* ---
Function Name: handle_signal

Purpose:
  Handles shell signal events. Specifically responds to SIGINT (Ctrl+C).
  If a foreground job is currently running, it writes a newline to 
  maintain proper output formatting. If no job is active, it prints
  the shell prompt again to restore user control.

Input:
  sig - integer signal value (e.g., SIGINT)

Output:
  Writes either a newline or the shell prompt to standard output.
--- */
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
  Installs the shell's signal handlers. Configures SIGINT (Ctrl+C)
  to trigger the handle_signal function and ignores SIGTSTP (Ctrl+Z)
  to prevent accidental suspension of the shell itself.

Input:
  None

Output:
  Registers the handlers for SIGINT and SIGTSTP.
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
