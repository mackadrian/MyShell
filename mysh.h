#ifndef MY_SH_H
#define MY_SH_H

#include "jobs.h"
#include <unistd.h>     // for fork(), execve(), _exit()
#include <sys/wait.h>   // for waitpid(), WIFEXITED, etc.

/*CONSTANTS*/
#define SHELL "mysh$ "
#define STD_IN 0
#define STD_OUT 1
#define STD_ERR 2

#define FALSE_VALUE             0
#define TRUE_VALUE              1
#define WAIT_ANY_CHILD          (-1)
#define NULL_PTR                ((char **)0)

extern int fg_job_status;

static void remove_zombies();

#endif
