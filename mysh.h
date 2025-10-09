#ifndef MY_SH_H
#define MY_SH_H

#include "jobs.h"
#include <unistd.h>     // for fork(), execve(), _exit()
#include <sys/wait.h>   // for waitpid(), WIFEXITED, etc.

/*CONSTANTS*/
#define STD_IN 0
#define STD_OUT 1
#define STD_ERR 2

/*FUNCTION DECLARATIONS*/
void get_command(Command *command);
int run_command(Command *command);
int tokenize(char *buffer, char *tokens[], int max_tokens);

#endif
