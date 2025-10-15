#include "getjob.h"
#include "mystring.h"
#include "myheap.h"
#include "errors.h"

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
    /* reset job structure */
    set_job(job);

    char *command_buffer = alloc(MAX_ARGS);
    write(STD_OUT, SHELL, mystrlen(SHELL));

    int bytes_read = read(STD_IN, command_buffer, MAX_ARGS);
    if (bytes_read < 0) {
        print_error(ERR_EXEC_FAIL);
        return;
    }
    if (bytes_read == 0) {
        /* EOF pressed, treat as empty input */
        free_all();
        return;
    }

    /* remove trailing newline if present */
    if (command_buffer[bytes_read - 1] == '\n') {
        command_buffer[bytes_read - 1] = '\0';
    } else {
        command_buffer[bytes_read] = '\0'; /* ensure null termination */
    }

    /* skip leading whitespace */
    int start = 0;
    while (command_buffer[start] == ' ' || command_buffer[start] == '\t')
        start++;

    if (command_buffer[start] == '\0') {
        /* empty input */
        free_all();
        return;
    }

    /* check for background execution (&) */
    int len = mystrlen(command_buffer);
    if (len > 0 && command_buffer[len - 1] == '&') {
        job->background = 1;
        command_buffer[len - 1] = '\0';
    }

    /* parse pipeline stages separated by '|' */
    int stage_start = start;
    for (int i = start;; i++) {
        char c = command_buffer[i];

        if (c == '|' || c == '\0') {
            command_buffer[i] = '\0';

            /* skip leading whitespace in this stage */
            while (command_buffer[stage_start] == ' ' || command_buffer[stage_start] == '\t')
                stage_start++;

            if (command_buffer[stage_start] != '\0') {
                parse_stage(&job->pipeline[job->num_stages], &command_buffer[stage_start]);
                job->num_stages++;
            }

            if (c == '\0')
                break;

            stage_start = i + 1;
        }
    }

    /* NOTE: do NOT call free_all() here — call it after run_job() in your shell loop */
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
    int i = 0;

    while (stage_str[i] != '\0') {
        /* skip whitespace */
        while (stage_str[i] == ' ' || stage_str[i] == '\t')
            i++;

        if (stage_str[i] == '\0')
            break;

        int start = i;

        while (stage_str[i] != ' ' && stage_str[i] != '\t' && stage_str[i] != '\0')
            i++;

        if (stage_str[i] != '\0')
            stage_str[i++] = '\0';

        cmd->argv[cmd->argc++] = &stage_str[start];
    }

    cmd->argv[cmd->argc] = NULL;
}


/* --- 
Function Name: set_job
Purpose: 
    Resets all fields of a Job structure to their default/empty state.
Input:
    job - pointer to a Job structure
Output:
    job fields are reset
--- */
void set_job(Job *job)
{
    job->num_stages = 0;
    job->background = 0;
    job->infile_path = NULL;
    job->outfile_path = NULL;

}
