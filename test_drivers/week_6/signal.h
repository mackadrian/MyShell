#ifndef SIGNAL_H
#define SIGNAL_H

#include <signal.h>

#define SIGNAL_PROMPT "\nmysh$ "
#define SIGNAL_PROMPT_LEN 8

extern volatile sig_atomic_t fg_job_running;

void handle_signal(int sig);

#endif
