#ifndef SIGNAL_H
#define SIGNAL_H

#include_next <signal.h>    /* sig_atomic_t, sigaction, SIGINT, etc. */
#include "mysh.h"

/* SIGNAL PROMPT CONSTANTS */
#define SIGNAL_PROMPT           "\nmysh$ "
#define SIGNAL_PROMPT_LEN       8

/* GENERAL STRING CONSTANTS */
#define NEWLINE_STR             "\n"
#define MSG_JOB_DONE            "[Job Done] "

/* NUMERIC / CONTROL CONSTANTS */
#define NO_FLAGS                0
#define INITIAL_INDEX           0
#define ZERO_VALUE              0
#define VALID_PID               0

/* GLOBAL VARIABLES */
extern volatile sig_atomic_t fg_job_running;

void handle_signal(int sig);
void initialize_signal_handler(void);
static void sigchld_handler(int sig);

#endif /* SIGNAL_H */
