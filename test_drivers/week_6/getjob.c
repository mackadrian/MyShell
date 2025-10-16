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
  structure. It identifies background execution requests (via '&') and 
  splits the command line by pipes ('|') to create multiple Command 
  stages. Each stage is tokenized separately using parse_stage().

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
    write(STDOUT_FILENO, SHELL, mystrlen(SHELL));

    int bytes_read = read(STDIN_FILENO, command_buffer, MAX_ARGS);
    int rid = check_read_status(bytes_read);

    /* remove trailing newline if present */
    if (command_buffer[bytes_read - 1] == '\n') {
        command_buffer[bytes_read - 1] = '\0';
    } else {
        command_buffer[bytes_read] = '\0';
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
                parse_stage(&job->pipeline[job->num_stages],
                            &command_buffer[stage_start],
                            job);
                job->num_stages++;
            }

            if (c == '\0')
                break;

            stage_start = i + 1;
        }
    }
}


/* ---
Function Name: parse_stage

Purpose: 
  Tokenizes a single stage of a pipeline command string into individual 
  arguments and stores them in the provided Command structure. It also 
  detects input/output redirection symbols ('<' and '>') and updates 
  the Job structure accordingly. Only normal command arguments are 
  added to argv.

Input:
  cmd       - pointer to a Command structure to store the parsed arguments.
  stage_str - pointer to a null-terminated string representing one stage 
              of a pipeline (a single command and its arguments).
  job       - pointer to the Job structure to set infile_path/outfile_path.

Output:
  Populates the Command structure with parsed arguments and sets the 
  argument count (argc). Updates job->infile_path and job->outfile_path.
  The argv array is null-terminated.
--- */
void parse_stage(Command *cmd, char *stage_str, Job *job)
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

        char *token = &stage_str[start];
        if (stage_str[i] != '\0')
            stage_str[i++] = '\0';

        /* handle input redirection */
        if (mystrcmp(token, "<") == 0) {
            while (stage_str[i] == ' ' || stage_str[i] == '\t')
                i++;
            start = i;
            while (stage_str[i] != ' ' && stage_str[i] != '\t' && stage_str[i] != '\0')
                i++;
            if (stage_str[i] != '\0') stage_str[i++] = '\0';
            job->infile_path = &stage_str[start];
        }
        /* handle output redirection */
        else if (mystrcmp(token, ">") == 0) {
            while (stage_str[i] == ' ' || stage_str[i] == '\t')
                i++;
            start = i;
            while (stage_str[i] != ' ' && stage_str[i] != '\t' && stage_str[i] != '\0')
                i++;
            if (stage_str[i] != '\0') stage_str[i++] = '\0';
            job->outfile_path = &stage_str[start];
        }
        /* normal argument */
        else {
            cmd->argv[cmd->argc++] = token;
        }
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


int check_read_status(int bytes_read)
{
  int exit_status = 0;

  if (bytes_read < 0) {
    print_error(ERR_EXEC_FAIL);
    exit_status = -1;
  } else if (bytes_read > MAX_ARGS) {
    print_error(ARG_EXCD_FAIL);
    exit_status = -1;
  } else {
    free_all();
  }

  return exit_status;
}
