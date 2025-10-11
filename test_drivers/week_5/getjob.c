#include "getjob.h"
#include "mystring.h"
#include "myheap.h"
#include <unistd.h>    // fork, pipe, dup2, execve, read, write, _exit
#include <sys/wait.h>  // waitpid

/* ---
Function Name: get_job

Purpose: 
  Reads an entire job command line from user input, parses it into one 
  or more pipeline stages, and stores the results in the provided Job 
  structure. The function identifies background execution requests 
  (via '&') and splits the command line by pipes ('|') to create 
  multiple Command stages for pipelined execution. Each stage is 
  tokenized separately using parse_stage().

Input:
  job - pointer to a Job structure that will store the parsed command
        pipeline, background flag, input/output paths, and stage count.

Output:
  Populates the Job structure with parsed command stages, background 
  execution flag, and resets file redirection paths. The number of 
  stages is stored in job->num_stages.
--- */
void get_job(Job *job)
{
    job->num_stages = 0;
    job->background = 0;
    job->infile_path = NULL;
    job->outfile_path = NULL;

    char *command_buffer = alloc(1024);
    
    write(STD_OUT, SHELL, mystrlen(SHELL));
    
    int bytes_read = read(STD_IN, command_buffer, 1024);
    if (bytes_read <= 0) return;
    command_buffer[bytes_read - 1] = '\0';

    // check background
    int len = mystrlen(command_buffer);
    if (command_buffer[len-1] == '&') {
        job->background = 1;
        command_buffer[len-1] = '\0';
    }

    // manual split by pipe '|'
    int i = 0, stage_start = 0;
    while (1) {
        if (command_buffer[i] == '|' || command_buffer[i] == '\0') {
            command_buffer[i] = '\0';
            parse_stage(&job->pipeline[job->num_stages], &command_buffer[stage_start]);
            job->num_stages++;
            if (command_buffer[i] == '\0') break;
            stage_start = i + 1;
        }
        i++;
    }

    free_all();
}

/* ---
Function Name: parse_stage

Purpose: 
  Tokenizes a single stage of a pipeline command string into individual 
  arguments and stores them in the provided Command structure. It splits 
  the stage string based on whitespace and appends each token to the 
  argument vector (argv) of the Command. The argument count (argc) is 
  updated accordingly.

Input:
  cmd       - pointer to a Command structure to store the parsed arguments.
  stage_str - pointer to a null-terminated string representing one stage 
              of a pipeline (a single command and its arguments).

Output:
  Populates the Command structure with parsed arguments and sets the 
  argument count (argc). The argv array is null-terminated.
--- */

void parse_stage(Command *cmd, char *stage_str)
{
    cmd->argc = 0;
    int i = 0, start = 0;
    while (stage_str[i] != '\0') {
        // skip spaces
        while (stage_str[i] == ' ' || stage_str[i] == '\t') i++;
        if (stage_str[i] == '\0') break;

        start = i;
        while (stage_str[i] != ' ' && stage_str[i] != '\t' && stage_str[i] != '\0') i++;

        stage_str[i] = '\0'; // terminate token
        cmd->argv[cmd->argc++] = &stage_str[start];
        i++;
    }
    cmd->argv[cmd->argc] = NULL;
}
