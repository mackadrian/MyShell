#ifndef MY_SH_H
#define MY_SH_H

#include "jobs.h"
#include <unistd.h>     /* fork(), execve(), _exit() */
#include <sys/wait.h>   /* waitpid(), WIFEXITED, etc. */

/* SHELL COMMAND CONSTANTS */
#define SHELL                   "mysh$ "
#define CMD_EXIT                "exit"
#define CMD_CD                  "cd"
#define CMD_EXPORT              "export"
#define CMD_JOBS                "jobs"
#define CMD_FG                  "fg"
#define CMD_BG                  "bg"

/* STANDARD FILE DESCRIPTORS */
#define STD_IN                  0
#define STD_OUT                 1
#define STD_ERR                 2

/* GENERAL CONSTANTS */
#define INITIAL_INDEX           0
#define FALSE_VALUE             0
#define TRUE_VALUE              1
#define WAIT_ANY_CHILD          (-1)
#define NULL_PTR                ((char **)0)

/* GLOBAL VARIABLES */
extern int fg_job_status;

static void remove_zombies(void);

#endif
