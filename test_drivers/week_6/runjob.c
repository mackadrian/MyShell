#include "runjob.h"
#include "mystring.h"
#include "myheap.h"
#include "errors.h"

#include <unistd.h>    // fork, pipe, dup2, execve, read, write, _exit
#include <sys/wait.h>  // waitpid
#include <sys/stat.h>  // stat
#include <fcntl.h>     // open, creat
#include <stdlib.h>    // malloc, free
#include <signal.h>    // SIGINT, SIGTSTP

// Helper: concatenate directory + "/" + cmd into buf
static void build_fullpath(char *buf, const char *dir, const char *cmd) {
    int i = 0;
    while (dir[i]) { buf[i] = dir[i]; i++; }
    buf[i++] = '/';
    int j = 0;
    while (cmd[j]) buf[i++] = cmd[j++];
    buf[i] = '\0';
}

// System-call only PATH search
char* resolve_command_path(const char *cmd, char *envp[]) {
    if (!cmd || cmd[0] == '\0') return NULL;

    // If cmd contains '/', treat as literal path
    for (int k = 0; cmd[k]; k++) {
        if (cmd[k] == '/') {
            struct stat st;
            if (stat(cmd, &st) == 0 && (st.st_mode & S_IXUSR)) {
                int len = 0; while (cmd[len]) len++;
                char *copy = (char*)malloc(len+1);
                if (!copy) return NULL;
                for (int i = 0; i <= len; i++) copy[i] = cmd[i];
                return copy;
            } else {
                return NULL;
            }
        }
    }

    // Get PATH from environment
    char *path_env = NULL;
    for (int i = 0; envp[i]; i++) {
        if (envp[i][0]=='P' && envp[i][1]=='A' && envp[i][2]=='T' &&
            envp[i][3]=='H' && envp[i][4]=='=')
        {
            path_env = envp[i]+5;
            break;
        }
    }
    if (!path_env) path_env = "/usr/local/bin:/usr/bin:/bin";

    char path_copy[1024];
    int len = 0;
    while (path_env[len] && len < 1023) { path_copy[len] = path_env[len]; len++; }
    path_copy[len] = '\0';

    char *start = path_copy;
    for (int i = 0; i <= len; i++) {
        if (path_copy[i] == ':' || path_copy[i] == '\0') {
            path_copy[i] = '\0';
            char fullpath[512];
            build_fullpath(fullpath, start, cmd);

            struct stat st;
            if (stat(fullpath, &st) == 0 && (st.st_mode & S_IXUSR)) {
                int plen = 0; while (fullpath[plen]) plen++;
                char *result = (char*)malloc(plen+1);
                if (!result) return NULL;
                for (int j = 0; j <= plen; j++) result[j] = fullpath[j];
                return result;
            }

            start = &path_copy[i+1];
        }
    }

    return NULL;  // not found
}

/* ---
Function Name: create_pipes
Purpose:
    Creates the necessary pipes for a pipeline of commands.
Input:
    pipefd - 2D array to hold pipe file descriptors
    num_stages - number of pipeline stages
Output:
    Initializes pipefd. Prints error if pipe creation fails.
--- */
static void create_pipes(int pipefd[MAX_PIPELINE_LEN-1][2], int num_stages)
{
    for (int i = 0; i < num_stages - 1; i++) {
        if (pipe(pipefd[i]) < 0)
            print_error(ERR_PIPE_FAIL);
    }
}

/* ---
Function Name: setup_redirection
Purpose:
    Sets up input/output redirection for a pipeline stage.
Input:
    stage_index - index of the current pipeline stage
    num_stages - total number of stages
    job - pointer to Job structure
    pipefd - 2D array of pipe file descriptors
Output:
    Duplicates file descriptors to STDIN_FILENO and STDOUT_FILENO as needed.
--- */
static void setup_redirection(int stage_index, int num_stages, Job *job, int pipefd[MAX_PIPELINE_LEN-1][2])
{
    if (stage_index == 0 && job->infile_path) {
        int fd = open(job->infile_path, O_RDONLY);
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    if (stage_index == num_stages - 1 && job->outfile_path) {
        int fd = creat(job->outfile_path, 0644);
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }

    if (stage_index > 0) dup2(pipefd[stage_index-1][0], STDIN_FILENO);
    if (stage_index < num_stages - 1) dup2(pipefd[stage_index][1], STDOUT_FILENO);

    // close all pipes
    for (int i = 0; i < num_stages - 1; i++) {
        close(pipefd[i][0]);
        close(pipefd[i][1]);
    }
}

/* ---
Function Name: fork_and_execute_stage
Purpose:
    Forks a child process for a single pipeline stage and executes the command.
Input:
    stage_index - index of the current stage
    job - pointer to Job structure
    envp - environment variables
    pipefd - 2D array of pipe file descriptors
Output:
    Executes the command in the child. Returns child's pid to parent.
--- */
static int fork_and_execute_stage(int stage_index, Job *job, char* envp[], int pipefd[MAX_PIPELINE_LEN-1][2])
{
    int pid = fork();
    if (pid < 0) { print_error(ERR_FORK_FAIL); return -1; }

    if (pid == 0) { // child
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);

        setup_redirection(stage_index, job->num_stages, job, pipefd);

        char *fullpath = resolve_command_path(job->pipeline[stage_index].argv[0], envp);
        if (!fullpath) {
            const char msg[] = ": command not found\n";
            write(STDERR_FILENO, job->pipeline[stage_index].argv[0], mystrlen(job->pipeline[stage_index].argv[0]));
            write(STDERR_FILENO, msg, sizeof(msg) - 1);
            _exit(1);
        }

        execve(fullpath, job->pipeline[stage_index].argv, envp);
        free(fullpath);
        _exit(1);
    }

    return pid; // parent
}

/* ---
Function Name: handle_background_job
Purpose:
    Prints background job info for the first stage if needed.
Input:
    job - pointer to Job structure
    pid - process ID of the first stage
Output:
    Prints "[background pid ...]" message to STDOUT_FILENO.
--- */
static void handle_background_job(Job *job, int pid)
{
    if (!job->background) return;

    char msg[128], pid_str[16];
    int n = 0, temp_pid = pid;
    if (temp_pid == 0) pid_str[n++] = '0';
    else while (temp_pid > 0 && n < 16) { pid_str[n++] = (temp_pid % 10) + '0'; temp_pid /= 10; }
    for (int j = 0; j < n; j++) pid_str[j] = pid_str[n - j - 1];
    pid_str[n] = '\0';

    mystrcpy(msg, "[background pid ");
    mystrcat(msg, pid_str);
    mystrcat(msg, "] ");
    mystrcat(msg, job->pipeline[0].argv[0]);
    mystrcat(msg, "\n");

    write(STDOUT_FILENO, msg, mystrlen(msg));
}

/* ---
Function Name: print_background_pid
Purpose:
    Prints the background job information for the first stage of a pipeline.
Input:
    job - pointer to Job structure
    pid - process ID of the first stage
Output:
    Writes "[background pid ...] command" to STDOUT_FILENO
--- */
static void print_background_pid(Job *job, int pid)
{
    char msg[128], pid_str[16];
    int n = 0, temp_pid = pid;

    /* convert pid to string */
    if (temp_pid == 0) pid_str[n++] = '0';
    else while (temp_pid > 0 && n < 16) { 
        pid_str[n++] = (temp_pid % 10) + '0'; 
        temp_pid /= 10; 
    }
    for (int j = 0; j < n; j++) pid_str[j] = pid_str[n - j - 1];
    pid_str[n] = '\0';

    /* build and print message */
    mystrcpy(msg, "[background pid ");
    mystrcat(msg, pid_str);
    mystrcat(msg, "] ");
    mystrcat(msg, job->pipeline[0].argv[0]);
    mystrcat(msg, "\n");

    write(STDOUT_FILENO, msg, mystrlen(msg));
}

/* ---
Function Name: run_job
Purpose:
    Executes a Job structure, handling pipelines, redirection, and background jobs.
Input:
    job - pointer to Job structure
    envp - environment variables
Output:
    Executes all stages of the job. Waits for foreground jobs; prints info for background jobs.
--- */
void run_job(Job *job, char* envp[])
{
    int pipefd[MAX_PIPELINE_LEN-1][2];
    int pids[MAX_PIPELINE_LEN];

    create_pipes(pipefd, job->num_stages);

    for (int i = 0; i < job->num_stages; i++) {
        pids[i] = fork_and_execute_stage(i, job, envp, pipefd);
        if (pids[i] < 0) return;
    }

    // close all pipes in parent
    for (int i = 0; i < job->num_stages - 1; i++) {
        close(pipefd[i][0]);
        close(pipefd[i][1]);
    }

    if (job->background) {
    print_background_pid(job, pids[0]);
    } else {
    for (int i = 0; i < job->num_stages; i++) 
        waitpid(pids[i], NULL, 0);
    }
}
