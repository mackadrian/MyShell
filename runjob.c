#include "runjob.h"
#include "mystring.h"
#include "myheap.h"
#include "errors.h"

#include <unistd.h>    // fork, pipe, dup2, execve, read, write, _exit
#include <sys/wait.h>  // waitpid
#include <fcntl.h>     // open, creat

void run_job(Job *job)
{
    int num_stages = job->num_stages;
    int pipefd[MAX_PIPELINE_LEN-1][2];
    int pids[MAX_PIPELINE_LEN];
    int pid;

    /* create pipes */
    for (int i=0; i<num_stages-1; i++)
        if (pipe(pipefd[i]) < 0)
            print_error(ERR_PIPE_FAIL);

    /* fork child processes */
    for (int i=0; i<num_stages; i++) {
        pid = fork();

        if (pid < 0) {
            print_error(ERR_FORK_FAIL);
            return;
        }

        if (pid == 0) {
            /* --- Child process --- */

            /* input redirection */
            if (i == 0 && job->infile_path) {
                int fd = open(job->infile_path, 0); /* 0 = read-only */
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            /* output redirection */
            if (i == num_stages-1 && job->outfile_path) {
                int fd = creat(job->outfile_path, 0644);
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            /* connect pipes */
            if (i > 0)
                dup2(pipefd[i-1][0], STDIN_FILENO);
            if (i < num_stages-1)
                dup2(pipefd[i][1], STDOUT_FILENO);

            /* close all pipes */
            for (int j=0; j<num_stages-1; j++) {
                close(pipefd[j][0]);
                close(pipefd[j][1]);
            }

            /* execute command */
            char fullpath[256];
            mystrcpy(fullpath, "/usr/bin/");
            mystrcat(fullpath, job->pipeline[i].argv[0]);
            execve(fullpath, job->pipeline[i].argv, NULL);
            _exit(1);
        } 
        else {
            /* --- Parent process --- */
            pids[i] = pid;
        }
    }

    /* close pipes in parent */
    for (int i=0; i<num_stages-1; i++) {
        close(pipefd[i][0]);
        close(pipefd[i][1]);
    }

    /* handle background or foreground job */
    if (job->background) {
        char msg[128];
        char pid_str[16];

        /* convert pid to string manually */
        int n = 0;
        int temp_pid = pids[0];
        char digits[16];
        if (temp_pid == 0) {
            digits[n++] = '0';
        } else {
            while (temp_pid > 0 && n < 16) {
                digits[n++] = (temp_pid % 10) + '0';
                temp_pid /= 10;
            }
        }

        /* reverse digits into pid_str */
        for (int j = 0; j < n; j++)
            pid_str[j] = digits[n - j - 1];
        pid_str[n] = '\0';

        /* construct message using mystring functions */
        mystrcpy(msg, "[background pid ");
        mystrcat(msg, pid_str);
        mystrcat(msg, "] ");
        mystrcat(msg, job->pipeline[0].argv[0]);
        mystrcat(msg, "\n");

        /* write directly to stdout (fd = 1) */
        write(STDOUT_FILENO, msg, mystrlen(msg));
    } 
    else {
        for (int i=0; i<num_stages; i++)
            waitpid(pids[i], NULL, 0);
    }
}
