#include "runjob.h"
#include "mystring.h"
#include "myheap.h"
#include "errors.h"

#include <unistd.h>    // fork, pipe, dup2, execve, read, write, _exit
#include <sys/wait.h>  // waitpid


/* ---
Function Name: run_job

Purpose: 
  Executes a parsed Job structure by creating child processes for each 
  stage in the pipeline. It handles inter-stage piping, input/output 
  redirection, and background execution. Each child process replaces 
  its image with the specified command using execve(). The parent process 
  waits for all child processes to complete if the job is running in 
  the foreground.

Input:
  job - pointer to a Job structure containing the pipeline of commands, 
        number of stages, input/output file paths, and background flag.

Output:
  Executes the commands specified in the Job structure. For foreground 
  jobs, the function waits for all stages to complete. For background 
  jobs, it returns immediately after spawning the child processes.
--- */
void run_job(Job *job)
{
    int num_stages = job->num_stages;
    int pipefd[MAX_PIPELINE_LEN-1][2];

    for (int i=0; i<num_stages-1; i++)
      if (pipe(pipefd[i]) < 0) print_error(ERR_PIPE_FAIL);

    for (int i=0; i<num_stages; i++) {
        int pid = fork();
        if (pid == 0) {
            // input redirection
            if (i == 0 && job->infile_path) {
                int fd = open(job->infile_path, 0); // 0 = read-only
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            // output redirection
            if (i == num_stages-1 && job->outfile_path) {
                int fd = creat(job->outfile_path, 0644); // create file
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            // connect pipes
            if (i > 0) dup2(pipefd[i-1][0], STDIN_FILENO);
            if (i < num_stages-1) dup2(pipefd[i][1], STDOUT_FILENO);

            // close all pipes
            for (int j=0; j<num_stages-1; j++) {
                close(pipefd[j][0]);
                close(pipefd[j][1]);
            }

            char fullpath[256];
            mystrcpy(fullpath, "/usr/bin/");
            mystrcat(fullpath, job->pipeline[i].argv[0]);
            execve(fullpath, job->pipeline[i].argv, NULL);
            _exit(1);
        }
    }

    // close pipes in parent
    for (int i=0; i<num_stages-1; i++) {
        close(pipefd[i][0]);
        close(pipefd[i][1]);
    }

    // wait for children if foreground
    if (!job->background)
        for (int i=0; i<num_stages; i++) wait(NULL);
}

