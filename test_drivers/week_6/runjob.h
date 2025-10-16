#ifndef RUNJOB_H
#define RUNJOB_H

#include "jobs.h"
#include "mysh.h"

char* resolve_command_path(const char *cmd, char *envp[]);
void run_job (Job *job, char* envp[]);

static void build_fullpath(char *buf, const char *dir, const char *cmd);
static void print_background_pid(Job *job, int pid);
static void create_pipes(int pipefd[MAX_PIPELINE_LEN-1][2], int num_stages);
static void setup_redirection(int stage_index, int num_stages, Job *job, int pipefd[MAX_PIPELINE_LEN-1][2]);
static int fork_and_execute_stage(int stage_index, Job *job, char* envp[], int pipefd[MAX_PIPELINE_LEN-1][2]);
static void handle_background_job(Job *job, int pid);

#endif
