#include "getjob.h"
#include "mystring.h"
#include "myheap.h"
#include "errors.h"
#include "signal.h"

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
    set_job(job);

    char *command_buffer = alloc(MAX_ARGS);
    if (!command_buffer) return;

    /* Install signal handler for Ctrl+C */
    signal(SIGINT, handle_signal);

    /* Prompt and read command line */
    write(STDOUT_FILENO, SHELL, mystrlen(SHELL));
    int bytes_read = read(STDIN_FILENO, command_buffer, MAX_ARGS);
    if (bytes_read <= 0) return;

    trim_newline(command_buffer, bytes_read);
    normalize_newlines(command_buffer);

    int start = skip_leading_whitespace(command_buffer);
    if (command_buffer[start] == '\0') return;

    handle_background(job, command_buffer);
    parse_pipeline(job, command_buffer, start);
}


/* ---
Function Name: normalize_newlines
Purpose:
    Replaces internal '\n' with spaces to avoid breaking parser
Input:
    buffer - null-terminated command string
Output:
    Modifies buffer in place
--- */
static void normalize_newlines(char *buffer)
{
    for (int i = 0; buffer[i] != '\0'; i++) {
        if (buffer[i] == '\n')
            buffer[i] = ' ';
    }
}


/* ---
Function Name: trim_newline
Purpose:
    Removes trailing newline from buffer if present
Input:
    buffer - input buffer
    bytes_read - number of bytes read
Output:
    Modifies buffer in place
--- */
static void trim_newline(char *buffer, int bytes_read)
{
    if (bytes_read <= 0) return;
    if (buffer[bytes_read - 1] == '\n') buffer[bytes_read - 1] = '\0';
    else buffer[bytes_read] = '\0';
}

/* ---
Function Name: skip_leading_whitespace
Purpose:
    Returns the index of the first non-whitespace character
Input:
    buffer - input string
Output:
    index of first non-space/tab character
--- */
static int skip_leading_whitespace(char *buffer)
{
    int i = 0;
    while (buffer[i] == ' ' || buffer[i] == '\t') i++;
    return i;
}

/* ---
Function Name: handle_background
Purpose:
    Detects trailing '&' and sets the Job's background flag
Input:
    job - pointer to Job structure
    buffer - command buffer
Output:
    Modifies job and buffer in place
--- */
static void handle_background(Job *job, char *buffer)
{
    int len = mystrlen(buffer);
    if (len > 0 && buffer[len - 1] == '&') {
        job->background = 1;
        buffer[len - 1] = '\0';
    }
}

/* ---
Function Name: parse_pipeline
Purpose:
    Splits command buffer into pipeline stages separated by '|', trims
    whitespace and newlines, and calls parse_stage for each stage.
    Safely handles empty stages and avoids infinite loops for malformed input.
Input:
    job    - pointer to Job structure to populate
    buffer - command buffer to parse
    start  - starting index of first stage
Output:
    Populates job->pipeline and job->num_stages
--- */
static void parse_pipeline(Job *job, char *buffer, int start)
{
    int stage_start = start;

    for (int i = start;; i++) {
        char c = buffer[i];

        if (c == '|' || c == '\0') {
            buffer[i] = '\0';

            while (buffer[stage_start] == ' ' || buffer[stage_start] == '\t' || buffer[stage_start] == '\n')
                stage_start++;

            if (buffer[stage_start] != '\0') {
                parse_stage(&job->pipeline[job->num_stages], &buffer[stage_start], job);
                job->num_stages++;
            }

            if (c == '\0') break;

            stage_start = i + 1;
        }
    }
}


/* ---
Function Name: parse_stage
Purpose:
    Tokenizes a single stage of a pipeline command into arguments,
    input/output redirection. Uses helper functions to handle each
    type of token.
Input:
    cmd - pointer to Command structure
    stage_str - null-terminated stage string
    job - pointer to Job structure
Output:
    Populates cmd->argv, cmd->argc, job->infile_path, and job->outfile_path
--- */
void parse_stage(Command *cmd, char *stage_str, Job *job)
{
    cmd->argc = 0;
    int i = 0;

    while (stage_str[i] != '\0') {
        while (stage_str[i] == ' ' || stage_str[i] == '\t') i++;
        if (stage_str[i] == '\0') break;

        int start = i;
        while (stage_str[i] != ' ' && stage_str[i] != '\t' && stage_str[i] != '\0') i++;

        /* Copy token to heap */
        int tok_len = i - start;
        char *token = alloc(tok_len + 1);
        for (int j = 0; j < tok_len; j++) token[j] = stage_str[start + j];
        token[tok_len] = '\0';

        if (mystrcmp(token, "<") == 0) {
            parse_input_redirection(job, stage_str, &i);
        } else if (mystrcmp(token, ">") == 0) {
            parse_output_redirection(job, stage_str, &i);
        } else {
            parse_argument(cmd, token);
        }

        if (stage_str[i] != '\0') i++;
    }

    cmd->argv[cmd->argc] = NULL;
}


/* ---
Function Name: parse_argument
Purpose:
    Adds a normal argument token to the Command structure.
Input:
    cmd - pointer to Command structure
    token - argument string
Output:
    Updates cmd->argv and cmd->argc
--- */
static void parse_argument(Command *cmd, char *token)
{
    cmd->argv[cmd->argc++] = token;
}

/* ---
Function Name: parse_input_redirection
Purpose:
    Parses an input redirection token ('< infile') and updates Job infile_path.
Input:
    job - pointer to Job structure
    stage_str - stage string containing redirection
    i - pointer to current index in stage_str; updated after parsing
Output:
    Sets job->infile_path
--- */
static void parse_input_redirection(Job *job, char *stage_str, int *i)
{
    while (stage_str[*i] == ' ' || stage_str[*i] == '\t') (*i)++;
    int start = *i;
    while (stage_str[*i] != ' ' && stage_str[*i] != '\t' && stage_str[*i] != '\0') (*i)++;
    int len = *i - start;
    char *path = alloc(len + 1);
    for (int j = 0; j < len; j++) path[j] = stage_str[start + j];
    path[len] = '\0';
    job->infile_path = path;

    if (stage_str[*i] != '\0') (*i)++;
}

/* ---
Function Name: parse_output_redirection
Purpose:
    Parses an output redirection token ('> outfile') and updates Job outfile_path.
Input:
    job - pointer to Job structure
    stage_str - stage string containing redirection
    i - pointer to current index in stage_str; updated after parsing
Output:
    Sets job->outfile_path
--- */
static void parse_output_redirection(Job *job, char *stage_str, int *i)
{
    while (stage_str[*i] == ' ' || stage_str[*i] == '\t') (*i)++;
    int start = *i;
    while (stage_str[*i] != ' ' && stage_str[*i] != '\t' && stage_str[*i] != '\0') (*i)++;
    int len = *i - start;
    char *path = alloc(len + 1);
    for (int j = 0; j < len; j++) path[j] = stage_str[start + j];
    path[len] = '\0';
    job->outfile_path = path;

    if (stage_str[*i] != '\0') (*i)++;
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

/* --- 
Function Name: check_read_status
Purpose: 
    Checks bytes_read and prints error if negative or exceeds MAX_ARGS
Input:
    bytes_read - result of read()
Output:
    Returns 0 if OK, -1 if error
--- */
int check_read_status(int bytes_read)
{
    int exit_status = 0;

    if (bytes_read < 0) {
        exit_status = -1;
    } else if (bytes_read > MAX_ARGS) {
        print_error(ERR_ARG_EXCD);
        exit_status = -1;
    } else if (bytes_read > 0) {
        free_all();
    }

    return exit_status;
}
