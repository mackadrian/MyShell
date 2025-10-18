#ifndef SIGNAL_H
#define SIGNAL_H

#include_next <signal.h>    /* sig_atomic_t, sigaction, SIGINT, etc. */

#define SIGNAL_PROMPT "\nmysh$ "
#define SIGNAL_PROMPT_LEN 8

#define NEWLINE_STR             "\n"
#define NEWLINE_LEN             1
#define NO_FLAGS                0

extern volatile sig_atomic_t fg_job_running;

void handle_signal(int sig);
void initialize_signal_handler(void);
static void sigchld_handler(int sig);

#endif
