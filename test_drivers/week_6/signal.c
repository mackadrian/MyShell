#include "signal.h"

#include <signal.h> 
#include <unistd.h> // write

volatile sig_atomic_t fg_job_running = 0;

void handle_signal(int sig)
{
  if (sig == SIGINT) {
    if (fg_job_running) {
      write(STDOUT_FILENO, "\n", 1);
    } else {
      write(STDOUT_FILENO, SIGNAL_PROMPT, SIGNAL_PROMPT_LEN);
    }
  }
}
