#ifndef RUNJOB_H
#define RUNJOB_H

#include "jobs.h"
#include "mysh.h"

#define ZERO_VALUE              0
#define TRUE_VALUE              1
#define ERROR_CODE              -1
#define EXIT_FAILURE_CODE       1
#define EXIT_SUCCESS_CODE       0

#define PATH_SEPARATOR          '/'
#define PATH_DELIMITER          ':'
#define NULL_CHAR               '\0'
#define PATH_PREFIX_LEN         5
#define DECIMAL_BASE            10

#define MAX_PATH_LEN            1024
#define FULLPATH_LEN            512
#define MAX_MSG_LEN             128
#define PID_STR_LEN             16
#define FILE_PERMISSIONS        0644

#define BG_MSG_PREFIX           "[background pid "
#define BG_MSG_SUFFIX           "] "
#define NEWLINE_STR             "\n"

char* resolve_command_path(const char *cmd, char *envp[]);
void run_job (Job *job, char* envp[]);

static void build_fullpath(char *buf, const char *dir, const char *cmd);
static void print_background_pid(Job *job, int pid);
static void create_pipes(int pipefd[MAX_PIPELINE_LEN-1][2], int num_stages);
static void setup_redirection(int stage_index, int num_stages, Job *job, int pipefd[MAX_PIPELINE_LEN-1][2]);
static int fork_and_execute_stage(int stage_index, Job *job, char* envp[], int pipefd[MAX_PIPELINE_LEN-1][2]);
static void handle_background_job(Job *job, int pid);

#endif
