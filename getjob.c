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
    set_job(job);
    write(STDOUT_FILENO, SHELL, mystrlen(SHELL));

    char *command_buffer = alloc(MAX_ARGS);
    if (!command_buffer) return;

    int bytes_read = read_line_from_stdin(command_buffer, MAX_ARGS);
    if (bytes_read <= ZERO_VALUE) return;

    normalize_newlines(command_buffer);
    int start = skip_leading_whitespace(command_buffer);
    if (command_buffer[start] == NULL_CHAR) return;

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
    for (int i = ZERO_VALUE; buffer[i] != NULL_CHAR; i++) {
        if (buffer[i] == NEWLINE_CHAR)
            buffer[i] = SPACE_CHAR;
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
    if (bytes_read <= ZERO_VALUE) return;
    if (buffer[bytes_read - TRUE_VALUE] == NEWLINE_CHAR)
        buffer[bytes_read - TRUE_VALUE] = NULL_CHAR;
    else
        buffer[bytes_read] = NULL_CHAR;
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
    int i = ZERO_VALUE;
    while (buffer[i] == SPACE_CHAR || buffer[i] == TAB_CHAR) i++;
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
    if (len > ZERO_VALUE && buffer[len - TRUE_VALUE] == BACKGROUND_CHAR) {
        job->background = TRUE_VALUE;
        buffer[len - TRUE_VALUE] = NULL_CHAR;
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

        if (c == PIPE_CHAR || c == NULL_CHAR) {
            buffer[i] = NULL_CHAR;

            while (buffer[stage_start] == SPACE_CHAR ||
                   buffer[stage_start] == TAB_CHAR ||
                   buffer[stage_start] == NEWLINE_CHAR)
                stage_start++;

            if (buffer[stage_start] != NULL_CHAR) {
                parse_stage(&job->pipeline[job->num_stages], &buffer[stage_start], job);
                job->num_stages++;
            }

            if (c == NULL_CHAR) break;
            stage_start = i + TRUE_VALUE;
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
    cmd->argc = ZERO_VALUE;
    int i = ZERO_VALUE;

    while (stage_str[i] != NULL_CHAR) {
        while (stage_str[i] == SPACE_CHAR || stage_str[i] == TAB_CHAR) i++;
        if (stage_str[i] == NULL_CHAR) break;

        int start = i;
        while (stage_str[i] != SPACE_CHAR && stage_str[i] != TAB_CHAR && stage_str[i] != NULL_CHAR) i++;

        int tok_len = i - start;
        char *token = alloc(tok_len + TRUE_VALUE);
        for (int j = ZERO_VALUE; j < tok_len; j++) token[j] = stage_str[start + j];
        token[tok_len] = NULL_CHAR;

        if (mystrcmp(token, TOKEN_INPUT) == ZERO_VALUE) {
            parse_input_redirection(job, stage_str, &i);
        } else if (mystrcmp(token, TOKEN_OUTPUT) == ZERO_VALUE) {
            parse_output_redirection(job, stage_str, &i);
        } else {
            parse_argument(cmd, token);
        }

        if (stage_str[i] != NULL_CHAR) i++;
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
    while (stage_str[*i] == SPACE_CHAR || stage_str[*i] == TAB_CHAR) (*i)++;
    int start = *i;
    while (stage_str[*i] != SPACE_CHAR && stage_str[*i] != TAB_CHAR && stage_str[*i] != NULL_CHAR) (*i)++;
    int len = *i - start;
    char *path = alloc(len + TRUE_VALUE);
    for (int j = ZERO_VALUE; j < len; j++) path[j] = stage_str[start + j];
    path[len] = NULL_CHAR;
    job->infile_path = path;

    if (stage_str[*i] != NULL_CHAR) (*i)++;
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
    while (stage_str[*i] == SPACE_CHAR || stage_str[*i] == TAB_CHAR) (*i)++;
    int start = *i;
    while (stage_str[*i] != SPACE_CHAR && stage_str[*i] != TAB_CHAR && stage_str[*i] != NULL_CHAR) (*i)++;
    int len = *i - start;
    char *path = alloc(len + TRUE_VALUE);
    for (int j = ZERO_VALUE; j < len; j++) path[j] = stage_str[start + j];
    path[len] = NULL_CHAR;
    job->outfile_path = path;

    if (stage_str[*i] != NULL_CHAR) (*i)++;
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
    job->num_stages = ZERO_VALUE;
    job->background = ZERO_VALUE;
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
    int exit_status = ZERO_VALUE;

    if (bytes_read < ZERO_VALUE) {
        exit_status = ERROR_CODE;
    } else if (bytes_read > MAX_ARGS) {
        print_error(ERR_ARG_EXCD);
        exit_status = ERROR_CODE;
    } else if (bytes_read > ZERO_VALUE) {
        free_all();
    }

    return exit_status;
}
/* ---
Function Name: read_line_from_stdin

Purpose:
    Reads one line from stdin (up to newline or EOF) using system calls only.
    
Input:
    buffer - destination buffer
    maxlen - maximum bytes to read (including null terminator)
    
Output:
    Returns number of bytes read (excluding null terminator), 
    0 on EOF, or -1 on error.
--- */
static int read_line_from_stdin(char *buffer, int maxlen)
{
    int total = ZERO_VALUE;
    char c;

    while (total < maxlen - TRUE_VALUE) {
        int n = read(STDIN_FILENO, &c, READ_BYTE_COUNT);

        if (n == ZERO_VALUE) {
            break;
        } else if (n < ZERO_VALUE) {
            return ERROR_CODE;
        } else if (c == NEWLINE_CHAR) {
            break;
        }

        buffer[total++] = c;
    }

    buffer[total] = NULL_CHAR;
    return total;
}
