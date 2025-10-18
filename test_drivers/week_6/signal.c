#include "signal.h"

#include <signal.h> 
#include <unistd.h> // write
#include <sys/wait.h>


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

    // SIGCHLD handler
    struct sigaction sa_chld;
    sa_chld.sa_handler = sigchld_handler;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa_chld, NULL);
    
    signal(SIGTSTP, SIG_IGN);       /* ignore Ctrl+Z */
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
    // Reap all terminated children
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0) {
        // Optionally handle job status updates here
    }
}
