#ifndef RUNJOB_H
#define RUNJOB_H

#include "jobs.h"
#include "mysh.h"

void build_fullpath(char *buf, const char *dir, const char *cmd);
char* resolve_command_path(const char *cmd, char *envp[]);
void run_job (Job *job, char* envp[]);

#endif
