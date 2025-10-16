#include "signal.h"

#include <signal.h> 
#include <unistd.h> // write

void handle_signal(int sig)
{
  if (sig == SIGINT) {
    write(STDOUT_FILENO, SIGNAL_PROMPT, SIGNAL_PROMPT_LEN);
  }
}
