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

void run_job(Job *job, char* envp[])
{
    int num_stages = job->num_stages;
    int pipefd[MAX_PIPELINE_LEN-1][2];
    int pids[MAX_PIPELINE_LEN];
    int pid;

    // create pipes
    for (int i=0; i<num_stages-1; i++)
        if (pipe(pipefd[i]) < 0)
            print_error(ERR_PIPE_FAIL);

    // fork child processes
    for (int i=0; i<num_stages; i++) {
        pid = fork();
        if (pid < 0) { print_error(ERR_FORK_FAIL); return; }

        if (pid == 0) { // child
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);

            if (i == 0 && job->infile_path) {
                int fd = open(job->infile_path, 0);
                dup2(fd, STDIN_FILENO);
                close(fd);
            }
            if (i == num_stages-1 && job->outfile_path) {
                int fd = creat(job->outfile_path, 0644);
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            if (i > 0) dup2(pipefd[i-1][0], STDIN_FILENO);
            if (i < num_stages-1) dup2(pipefd[i][1], STDOUT_FILENO);

            // close all pipes
            for (int j=0; j<num_stages-1; j++) {
                close(pipefd[j][0]);
                close(pipefd[j][1]);
            }

            // execute command
            char *fullpath = resolve_command_path(job->pipeline[i].argv[0], envp);
            if (!fullpath) {
                const char msg1[] = ": command not found\n";
                //write(STDERR_FILENO, msg1, sizeof(msg1)-1);
                write(STDERR_FILENO, job->pipeline[i].argv[0], mystrlen(job->pipeline[i].argv[0]));
                write(STDERR_FILENO, msg1, sizeof(msg1)-1);
                _exit(1);
            }
            execve(fullpath, job->pipeline[i].argv, envp);
            free(fullpath);
            _exit(1);
        } else { // parent
            pids[i] = pid;
        }
    }

    // close pipes in parent
    for (int i=0; i<num_stages-1; i++) {
        close(pipefd[i][0]);
        close(pipefd[i][1]);
    }

    // handle background/foreground
    if (job->background) {
        char msg[128], pid_str[16];
        int n = 0;
        int temp_pid = pids[0];
        char digits[16];
        if (temp_pid == 0) digits[n++] = '0';
        else while (temp_pid > 0 && n < 16) { digits[n++] = (temp_pid % 10) + '0'; temp_pid /= 10; }
        for (int j = 0; j < n; j++) pid_str[j] = digits[n - j - 1];
        pid_str[n] = '\0';
        mystrcpy(msg, "[background pid ");
        mystrcat(msg, pid_str);
        mystrcat(msg, "] ");
        mystrcat(msg, job->pipeline[0].argv[0]);
        mystrcat(msg, "\n");
        write(STDOUT_FILENO, msg, mystrlen(msg));
    } else {
        for (int i=0; i<num_stages; i++)
            waitpid(pids[i], NULL, 0);
    }
}

