#ifndef RUNJOB_H
#define RUNJOB_H

#include "jobs.h"
#include "mysh.h"

// Numeric / Boolean Constants
#define INDEX_OFFSET            1
#define ZERO_VALUE              0
#define TRUE_VALUE              1
#define ERROR_CODE              -1
#define EXIT_FAILURE_CODE       1
#define EXIT_SUCCESS_CODE       0

// Character / String Constants
#define PATH_SEPARATOR          '/'
#define PATH_DELIMITER          ':'
#define NULL_CHAR               '\0'
#define PATH_PREFIX_LEN         5
#define DECIMAL_BASE            10
#define ZERO_CHAR               '0'

// Sizes / Lengths
#define MAX_PATH_LEN            1024
#define FULLPATH_LEN            512
#define MAX_MSG_LEN             128
#define PID_STR_LEN             16
#define MSG_BUFFER              8

// File / I/O
#define FILE_PERMISSIONS        0644

// Message Formatting
#define MSG_SPACE               " "
#define MSG_BG_PREFIX            "["
#define MSG_BG_SUFFIX            "] "
#define MSG_FG_PREFIX           "["
#define MSG_FG_SUFFIX           "] stopped \t"
#define NEWLINE_STR             "\n"

char* resolve_command_path(const char *cmd, char *envp[]);
void run_job (Job *job, char* envp[]);
void add_job(Job *job, int pid);

extern Job jobs[MAX_JOBS];
extern int num_jobs;

static int check_executable(const char *path);
static char* copy_string_heap(const char *src);
static void itoa_custom(int value, char *buf, int buflen);
static void construct_background_msg(char *msg, Job *job, int pid, int job_no);
static void build_fullpath(char *buf, const char *dir, const char *cmd);
static void print_background_pid(Job *job, int pid);
static void create_pipes(int pipefd[MAX_PIPELINE_LEN-1][2], int num_stages);
static void setup_redirection(int stage_index, int num_stages, Job *job, int pipefd[MAX_PIPELINE_LEN-1][2]);
static int fork_and_execute_stage(int stage_index, Job *job, char* envp[], int pipefd[MAX_PIPELINE_LEN-1][2]);
static void handle_background_job(Job *job, int pid);
static int execute_all_stages(Job *job, char *envp[], int pipefd[MAX_PIPELINE_LEN - 1][2], int *pids);
static void close_all_pipes(int pipefd[MAX_PIPELINE_LEN - 1][2], int num_stages);
static void handle_background_job(Job *job, int pid);
static void handle_foreground_job(Job *job, int *pids);

#endif
