#include "runjob.h"
#include "mystring.h"
#include "myheap.h"
#include "errors.h"
#include "signal.h"

#include <unistd.h>    /* fork, pipe, dup2, execve, read, write, _exit */
#include <sys/wait.h>  /* waitpid */
#include <sys/stat.h>  /* stat */
#include <fcntl.h>     /* open, creat */
#include <errno.h>

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
void run_job(Job *job, char *envp[])
{
    int fg_job_status;
    if (!job || job->num_stages == ZERO_VALUE) return;

    int pipefd[MAX_PIPELINE_LEN - 1][2];
    int pids[MAX_PIPELINE_LEN];
    int fg_job_running = !job->background;

    create_pipes(pipefd, job->num_stages);

    for (int i = ZERO_VALUE; i < job->num_stages; i++) {
        pids[i] = fork_and_execute_stage(i, job, envp, pipefd);
        if (pids[i] < ZERO_VALUE) {
            fg_job_running = ZERO_VALUE;
            free_all();
            return;
        }
    }

    for (int i = ZERO_VALUE; i < job->num_stages - TRUE_VALUE; i++) {
        close(pipefd[i][ZERO_VALUE]);
        close(pipefd[i][TRUE_VALUE]);
    }

    if (job->background) {
        print_background_pid(job, pids[ZERO_VALUE]);
	add_job(job, pids[ZERO_VALUE]);
        fg_job_running = ZERO_VALUE;
    } else {
    int status;

    /* Allow the foreground job to receive Ctrl+Z and Ctrl+C */
    signal(SIGTSTP, SIG_DFL);
    signal(SIGINT, SIG_DFL);

    /* Give the terminal to the job's process group */
    signal(SIGTTOU, SIG_IGN);               // prevent blocking on tcsetpgrp
    tcsetpgrp(STDIN_FILENO, pids[ZERO_VALUE]);

    fg_job_running = TRUE_VALUE;

    /* Wait for the job to finish or stop */
    for (int i = ZERO_VALUE; i < job->num_stages; i++) {
        while (waitpid(pids[i], &status, WUNTRACED) == -1 && errno == EINTR)
            continue;

        if (WIFEXITED(status)) {
            fg_job_status = WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            fg_job_status = EXIT_FAILURE_CODE;
            if (WTERMSIG(status) == SIGINT)
                break;
        } else if (WIFSTOPPED(status)) {
            /* Handle Ctrl+Z stopping the foreground job */
            add_job(job, pids[i]);
            jobs[num_jobs - 1].background = 0;

            write(STDOUT_FILENO, "[", 1);
            char buf[8];
            myitoa(num_jobs, buf);
            write(STDOUT_FILENO, buf, mystrlen(buf));
            write(STDOUT_FILENO, "] Stopped\t", 10);
            write(STDOUT_FILENO, job->pipeline[0].argv[0],
                  mystrlen(job->pipeline[0].argv[0]));
            write(STDOUT_FILENO, "\n", 1);
	    fsync(STDOUT_FILENO);
            break;
        }
    }

    /* Return terminal control to the shell */
    tcsetpgrp(STDIN_FILENO, getpgrp());
    signal(SIGTTOU, SIG_DFL);

    /* Shell should ignore Ctrl+Z again */
    signal(SIGTSTP, SIG_IGN);
    signal(SIGINT, handle_signal);

    fg_job_running = ZERO_VALUE;
}

    free_all();
}



/* ---
Function Name: add_job

Purpose:
  Adds a background job to the global jobs table for later reference by builtins
  such as 'jobs', 'fg', and 'bg'.

Input:
  job  - pointer to Job structure representing the launched job
  pid  - process ID of the first process in the job pipeline

Output:
  None. Updates global jobs[] and num_jobs.
--- */
void add_job(Job *job, int pid)
{
    if (num_jobs >= MAX_JOBS)
        return;

    jobs[num_jobs] = *job;         /* copy job info */
    jobs[num_jobs].pgid = pid;     /* store process group ID */
    num_jobs++;
}

/* ---
Function Name: build_fullpath

Purpose:
    Concatenates a directory path and a command into a single path buffer.
    
Input:
    buf - output buffer
    dir - directory path
    cmd - command name
    
Output:
    Stores the full path in buf.
--- */
static void build_fullpath(char *buf, const char *dir, const char *cmd)
{
    int i = ZERO_VALUE;
    while (dir[i]) { buf[i] = dir[i]; i++; }
    buf[i++] = PATH_SEPARATOR;
    int j = ZERO_VALUE;
    while (cmd[j]) buf[i++] = cmd[j++];
    buf[i] = NULL_CHAR;
}


/* --- 
Function Name: resolve_command_path

Purpose:
  Resolves the full path of a command using the PATH environment variable.
  
Input:
  cmd - command name
  envp - environment variables
  
Output:
  Returns a heap-allocated string containing the command’s full path if found,
  or NULL if not found.
--- */
char* resolve_command_path(const char *cmd, char *envp[])
{
    if (!cmd || cmd[ZERO_VALUE] == NULL_CHAR) return NULL;

    for (int k = ZERO_VALUE; cmd[k]; k++) {
        if (cmd[k] == PATH_SEPARATOR) {
            struct stat st;
            if (stat(cmd, &st) == ZERO_VALUE && (st.st_mode & S_IXUSR)) {
                int len = ZERO_VALUE; while (cmd[len]) len++;
                char *copy = alloc(len + TRUE_VALUE);
                if (!copy) return NULL;
                for (int i = ZERO_VALUE; i <= len; i++) copy[i] = cmd[i];
                return copy;
            } else {
                return NULL;
            }
        }
    }

    char *path_env = NULL;
    for (int i = ZERO_VALUE; envp[i]; i++) {
        if (envp[i][ZERO_VALUE]=='P' && envp[i][1]=='A' &&
            envp[i][2]=='T' && envp[i][3]=='H' && envp[i][4]=='=') {
            path_env = envp[i] + PATH_PREFIX_LEN;
            break;
        }
    }
    if (!path_env) path_env = "/usr/local/bin:/usr/bin:/bin";

    char path_copy[MAX_PATH_LEN];
    int len = ZERO_VALUE;
    while (path_env[len] && len < MAX_PATH_LEN - TRUE_VALUE) {
        path_copy[len] = path_env[len];
        len++;
    }
    path_copy[len] = NULL_CHAR;

    char *start = path_copy;
    for (int i = ZERO_VALUE; i <= len; i++) {
        if (path_copy[i] == PATH_DELIMITER || path_copy[i] == NULL_CHAR) {
            path_copy[i] = NULL_CHAR;
            char fullpath[FULLPATH_LEN];
            build_fullpath(fullpath, start, cmd);

            struct stat st;
            if (stat(fullpath, &st) == ZERO_VALUE && (st.st_mode & S_IXUSR)) {
                int plen = ZERO_VALUE; while (fullpath[plen]) plen++;
                char *result = alloc(plen + TRUE_VALUE);
                if (!result) return NULL;
                for (int j = ZERO_VALUE; j <= plen; j++) result[j] = fullpath[j];
                return result;
            }

            start = &path_copy[i + TRUE_VALUE];
        }
    }

    return NULL;
}


/* ---
Function Name: create_pipes

Purpose:
    Creates pipes for inter-process communication between pipeline stages.
    
Input:
    pipefd - 2D array to hold file descriptors
    num_stages - number of stages in pipeline
    
Output:
    Initializes pipefd. Prints error if pipe creation fails.
--- */
static void create_pipes(int pipefd[MAX_PIPELINE_LEN - 1][2], int num_stages)
{
    for (int i = ZERO_VALUE; i < num_stages - TRUE_VALUE; i++) {
        if (pipe(pipefd[i]) < ZERO_VALUE)
            print_error(ERR_PIPE_FAIL);
    }
}

/* ---
Function Name: setup_redirection

Purpose:
    Handles I/O redirection for each stage in the pipeline.
    
Input:
    stage_index - index of the current stage
    num_stages - total number of stages
    job - pointer to Job structure
    pipefd - 2D array of pipe file descriptors
    
Output:
    Sets up file descriptors appropriately for input/output.
--- */
static void setup_redirection(int stage_index, int num_stages, Job *job,
                              int pipefd[MAX_PIPELINE_LEN - 1][2])
{
    if (stage_index == ZERO_VALUE && job->infile_path) {
        int fd = open(job->infile_path, O_RDONLY);
        if (fd < ZERO_VALUE) _exit(EXIT_FAILURE_CODE);
        dup2(fd, STDIN_FILENO);
        close(fd);
    }

    if (stage_index == num_stages - TRUE_VALUE && job->outfile_path) {
        int fd = creat(job->outfile_path, FILE_PERMISSIONS);
        if (fd < ZERO_VALUE) _exit(EXIT_FAILURE_CODE);
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }

    if (stage_index > ZERO_VALUE)
        dup2(pipefd[stage_index - TRUE_VALUE][ZERO_VALUE], STDIN_FILENO);
    if (stage_index < num_stages - TRUE_VALUE)
        dup2(pipefd[stage_index][TRUE_VALUE], STDOUT_FILENO);

    for (int i = ZERO_VALUE; i < num_stages - TRUE_VALUE; i++) {
        if (i != stage_index - TRUE_VALUE) close(pipefd[i][ZERO_VALUE]);
        if (i != stage_index) close(pipefd[i][TRUE_VALUE]);
    }
}

/* ---
Function Name: fork_and_execute_stage

Purpose:
    Forks and executes a command stage in the pipeline.
    
Input:
    stage_index - index of current stage
    job - pointer to Job structure
    envp - environment variables
    pipefd - 2D array of pipe file descriptors
    
Output:
    Executes the command in a child and returns its PID.
--- */
static int fork_and_execute_stage(int stage_index, Job *job, char *envp[],
                                  int pipefd[MAX_PIPELINE_LEN - 1][2])
{
    if (!job->pipeline[stage_index].argv[ZERO_VALUE]) return ERROR_CODE;

    int pid = fork();
    if (pid < ZERO_VALUE) {
        print_error(ERR_FORK_FAIL);
        return ERROR_CODE;
    }

    if (pid == ZERO_VALUE) {
        setpgid(ZERO_VALUE, ZERO_VALUE);
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);

        setup_redirection(stage_index, job->num_stages, job, pipefd);

        char *fullpath = resolve_command_path(job->pipeline[stage_index].argv[ZERO_VALUE], envp);
        if (!fullpath) {
            write(STDERR_FILENO, job->pipeline[stage_index].argv[ZERO_VALUE],
                  mystrlen(job->pipeline[stage_index].argv[ZERO_VALUE]));
            write(STDERR_FILENO, error_messages[ERR_CMD_NOT_FOUND],
                  mystrlen(error_messages[ERR_CMD_NOT_FOUND]));
            _exit(EXIT_FAILURE_CODE);
        }

        execve(fullpath, job->pipeline[stage_index].argv, envp);
        write(STDERR_FILENO, error_messages[ERR_EXEC_FAIL],
              mystrlen(error_messages[ERR_EXEC_FAIL]));
        _exit(EXIT_FAILURE_CODE);
    }

    return pid;
}

/* ---
Function Name: print_background_pid

Purpose:
    Displays background job process ID.
    
Input:
    job - pointer to Job structure
    pid - process ID of first command in pipeline
    
Output:
    Prints background job info to STDOUT.
--- */
static void print_background_pid(Job *job, int pid)
{
    char msg[MAX_MSG_LEN], pid_str[PID_STR_LEN];
    int n = ZERO_VALUE, temp_pid = pid;

    if (temp_pid == ZERO_VALUE)
        pid_str[n++] = '0';
    else while (temp_pid > ZERO_VALUE && n < PID_STR_LEN) {
        pid_str[n++] = (temp_pid % DECIMAL_BASE) + '0';
        temp_pid /= DECIMAL_BASE;
    }

    for (int j = ZERO_VALUE; j < n; j++)
        pid_str[j] = pid_str[n - j - TRUE_VALUE];
    pid_str[n] = NULL_CHAR;

    mystrcpy(msg, BG_MSG_PREFIX);
    mystrcat(msg, pid_str);
    mystrcat(msg, BG_MSG_SUFFIX);
    mystrcat(msg, job->pipeline[ZERO_VALUE].argv[ZERO_VALUE]);
    mystrcat(msg, NEWLINE_STR);

    write(STDOUT_FILENO, msg, mystrlen(msg));
}
