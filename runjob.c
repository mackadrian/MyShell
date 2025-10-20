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
    if (!job || job->num_stages == ZERO_VALUE) return;

    int pipefd[MAX_PIPELINE_LEN - 1][2];
    int pids[MAX_PIPELINE_LEN];

    create_pipes(pipefd, job->num_stages);

    if (!execute_all_stages(job, envp, pipefd, pids)) {
        free_all();
        return;
    }

    close_all_pipes(pipefd, job->num_stages);

    if (job->background) {
        handle_background_job(job, pids[ZERO_VALUE]);
    } else {
        handle_foreground_job(job, pids);
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
Function Name: check_executable

Purpose:
    Checks if a path exists and is executable.
    
Input:
    path - path string to check
    
Output:
    Returns 1 if path exists and is executable, 0 otherwise.
--- */
static int check_executable(const char *path)
{
    struct stat st;
    return (stat(path, &st) == ZERO_VALUE && (st.st_mode & S_IXUSR));
}

/* ---
Function Name: copy_string_heap

Purpose:
    Copies a string to a newly allocated heap buffer.
    
Input:
    src - source string
    
Output:
    Returns pointer to heap copy or NULL if allocation fails.
--- */
static char* copy_string_heap(const char *src)
{
    int len = ZERO_VALUE;
    while (src[len]) len++;
    char *copy = alloc(len + TRUE_VALUE);
    if (!copy) return NULL;
    for (int i = ZERO_VALUE; i <= len; i++)
        copy[i] = src[i];
    return copy;
}

/* ---
Function Name: resolve_command_path

Purpose:
    Resolves a command name to its full executable path using PATH.
    
Input:
    cmd  - command name
    envp - environment variables
    
Output:
    Returns heap-allocated full path if found, NULL otherwise.
--- */
char* resolve_command_path(const char *cmd, char *envp[])
{
    if (!cmd || cmd[ZERO_VALUE] == NULL_CHAR) return NULL;

    /* Absolute or relative path given */
    for (int k = ZERO_VALUE; cmd[k]; k++) {
        if (cmd[k] == PATH_SEPARATOR && check_executable(cmd)) {
            return copy_string_heap(cmd);
        }
    }

    /* Get PATH environment */
    char *path_env = NULL;
    for (int i = ZERO_VALUE; envp[i]; i++) {
        if (envp[i][ZERO_VALUE]=='P' && envp[i][1]=='A' &&
            envp[i][2]=='T' && envp[i][3]=='H' && envp[i][4]=='=') {
            path_env = envp[i] + PATH_PREFIX_LEN;
            break;
        }
    }
    if (!path_env) path_env = "/usr/local/bin:/usr/bin:/bin";

    /* Copy PATH to local buffer */
    char path_copy[MAX_PATH_LEN];
    int len = ZERO_VALUE;
    while (path_env[len] && len < MAX_PATH_LEN - TRUE_VALUE) {
        path_copy[len] = path_env[len];
        len++;
    }
    path_copy[len] = NULL_CHAR;

    /* Iterate over directories in PATH */
    char *start = path_copy;
    for (int i = ZERO_VALUE; i <= len; i++) {
        if (path_copy[i] == PATH_DELIMITER || path_copy[i] == NULL_CHAR) {
            path_copy[i] = NULL_CHAR;

            char fullpath[FULLPATH_LEN];
            build_fullpath(fullpath, start, cmd);
            if (check_executable(fullpath))
                return copy_string_heap(fullpath);

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
Function Name: itoa_custom

Purpose:
    Converts an integer to a string in decimal form.
    
Input:
    value  - integer to convert
    buf    - output buffer
    buflen - maximum buffer length
    
Output:
    Stores the null-terminated string representation of value in buf.
--- */
static void itoa_custom(int value, char *buf, int buflen)
{
    int n = INITIAL_INDEX;
    int temp = value;

    if (temp == INITIAL_INDEX) {
        buf[n++] = ZERO_CHAR;
    } else {
        while (temp > INITIAL_INDEX && n < buflen - TRUE_VALUE) {
            buf[n++] = (temp % DECIMAL_BASE) + ZERO_CHAR;
            temp /= DECIMAL_BASE;
        }
    }

    /* Reverse string */
    for (int i = INITIAL_INDEX; i < n / 2; i++) {
        char tmp = buf[i];
        buf[i] = buf[n - i - 1];
        buf[n - i - 1] = tmp;
    }
    buf[n] = NULL_CHAR;
}

/* ---
Function Name: construct_background_msg

Purpose:
    Constructs the background job message "[job_number] pid command" in msg buffer.
    
Input:
    msg   - output buffer
    job   - pointer to Job structure
    pid   - process ID
    job_no - job number to display (starting at 1)
    
Output:
    msg buffer contains the formatted string.
--- */
static void construct_background_msg(char *msg, Job *job, int pid, int job_no)
{
    char pid_str[PID_STR_LEN];
    char job_num_str[PID_STR_LEN];

    itoa_custom(pid, pid_str, PID_STR_LEN);
    itoa_custom(job_no, job_num_str, PID_STR_LEN);

    mystrcpy(msg, MSG_BG_PREFIX);
    mystrcat(msg, job_num_str);
    mystrcat(msg, MSG_BG_SUFFIX);
    mystrcat(msg, pid_str);
    mystrcat(msg, MSG_SPACE);
    mystrcat(msg, job->pipeline[INITIAL_INDEX].argv[INITIAL_INDEX]);
    mystrcat(msg, NEWLINE_STR);
}

/* ---
Function Name: print_background_pid

Purpose:
    Prints background job info to STDOUT in Bash style.
    
Input:
    job - pointer to Job structure
    pid - process ID of first command in pipeline
    
Output:
    Prints message like "[1] 1234 sleep" to STDOUT.
--- */
static void print_background_pid(Job *job, int pid)
{
    char msg[MAX_MSG_LEN];
    int job_no = num_jobs + INDEX_OFFSET;
    construct_background_msg(msg, job, pid, job_no);
    write(STDOUT_FILENO, msg, mystrlen(msg));
}

/* ---
Function Name: execute_all_stages

Purpose:
    Forks and executes all stages of the job pipeline.

Input:
    job - pointer to Job structure
    envp - environment variables
    pipefd - array of pipe file descriptors

Output:
    Returns 1 on success, 0 on failure. Populates pids array.
--- */
static int execute_all_stages(Job *job, char *envp[], int pipefd[MAX_PIPELINE_LEN - 1][2], int *pids)
{
    for (int i = ZERO_VALUE; i < job->num_stages; i++) {
        pids[i] = fork_and_execute_stage(i, job, envp, pipefd);
        if (pids[i] < ZERO_VALUE)
            return ZERO_VALUE;
    }
    return TRUE_VALUE;
}

/* ---
Function Name: close_all_pipes

Purpose:
    Closes all pipe file descriptors after use.

Input:
    pipefd - 2D array of pipe file descriptors
    num_stages - number of stages in the job

Output:
    Closes all pipe read/write ends.
--- */
static void close_all_pipes(int pipefd[MAX_PIPELINE_LEN - 1][2], int num_stages)
{
    for (int i = ZERO_VALUE; i < num_stages - TRUE_VALUE; i++) {
        close(pipefd[i][ZERO_VALUE]);
        close(pipefd[i][TRUE_VALUE]);
    }
}

/* ---
Function Name: handle_background_job

Purpose:
    Handles background job behavior by printing PID and adding it to the job list.

Input:
    job - pointer to Job structure
    pid - PID of the first process in the pipeline

Output:
    Prints background job info and stores it.
--- */
static void handle_background_job(Job *job, int pid)
{
    print_background_pid(job, pid);
    add_job(job, pid);
}

/* ---
Function Name: handle_foreground_job

Purpose:
    Handles foreground job execution, signal management, and waiting.

Input:
    job - pointer to Job structure
    pids - array of process IDs for the job stages

Output:
    Waits for job completion or suspension. Restores shell control.
--- */
static void handle_foreground_job(Job *job, int *pids)
{
    int status;
    int fg_job_status;

    /* Allow the foreground job to receive Ctrl+Z and Ctrl+C */
    signal(SIGTSTP, SIG_DFL);
    signal(SIGINT, SIG_DFL);

    /* Give the terminal to the job's process group */
    signal(SIGTTOU, SIG_IGN);
    tcsetpgrp(STDIN_FILENO, pids[ZERO_VALUE]);

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

            write(STDOUT_FILENO, MSG_FG_PREFIX, 1);
            char buf[MSG_BUFFER];
            myitoa(num_jobs, buf);
            write(STDOUT_FILENO, buf, mystrlen(buf));
            write(STDOUT_FILENO, MSG_FG_SUFFIX, 10);
            write(STDOUT_FILENO, job->pipeline[0].argv[0],
                  mystrlen(job->pipeline[0].argv[0]));
            write(STDOUT_FILENO, NEWLINE_STR, 1);
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
}
