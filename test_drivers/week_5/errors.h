#ifndef ERRORS_H
#define ERRORS_H

#define STD_OUT 1
#define STD_ERR 2

enum ErrorCode {
    ERR_CMD_NOT_FOUND = 1,   /* start numbering from 1 */
    ERR_PIPE_FAIL,
    ERR_FORK_FAIL,
    ERR_EXEC_FAIL,
    ERR_FILE_NOT_FOUND,
    ERR_INVALID_INPUT,
    NUM_ERRORS
};


/* Error message table */
static const char *error_messages[NUM_ERRORS] = {
    [ERR_CMD_NOT_FOUND]  = "Error: command not found\n",
    [ERR_PIPE_FAIL]      = "Error: pipe creation failed\n",
    [ERR_FORK_FAIL]      = "Error: fork failed\n",
    [ERR_EXEC_FAIL]      = "Error: execution failed\n",
    [ERR_FILE_NOT_FOUND] = "Error: file not found\n",
    [ERR_INVALID_INPUT]  = "Error: invalid input\n"
};


/* Function prototype */
void print_error(enum ErrorCode code);

#endif
