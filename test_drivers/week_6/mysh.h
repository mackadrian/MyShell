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

static void remove_zombies();
static void initialize_signal_handler();
#endif
