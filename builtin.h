#ifndef BUILTIN_H
#define BUILTIN_H

#include "jobs.h"

// Global Variable
extern int last_exit_status;
extern Job jobs[MAX_JOBS];
extern int num_jobs;

// General Constants
#define INITIAL_INDEX           0
#define ZERO_VALUE              0
#define TRUE                    1
#define FALSE                   0
#define NULL_CHAR               '\0'

// Parsing / Numeric Conversion
#define DECIMAL_BASE            10
#define HALF                    2
#define NEGATIVE_SIGN           '-'
#define POSITIVE_SIGN           1
#define ZERO_CHAR               '0'
#define NEGATIVE_SIGN_MUL       -1
#define INT_BUFFER_LEN          16

// Environment Handling
#define HOME_ENV_NAME           "HOME"
#define ENV_ASSIGN_CHAR         '='
#define ASSIGN_EQUAL            "="
#define ENV_STRING_EXTRA        2
#define VAR_EXIT_STATUS         "$?"
#define DEF_EXIT_STATUS         0
#define TOKEN_$                 '$'
#define STRINGS_MATCH           0
#define ENV_TERMINATOR_NULL     '\0'

// Job Control
#define NO_JOBS                 0
#define INVALID_PGID            0
#define JOB_OFFSET_INDEX        1
#define JOB_DISPLAY_WIDTH       8
#define JOB_ID_OFFSET           1

// Output Formatting
#define TERMINAL_TAB_CHAR       "\t"
#define JOB_NEWLINE_CHAR        "\n"
#define JOB_STRING_END          '\0'
#define STATUS_RUNNING_TEXT     "Running"
#define STATUS_DONE_TEXT        "Done"
#define STATUS_STOPPED_TEXT     "Stopped"

// Error Messages
#define CD_ERROR_MSG            "cd: failed\n"
#define CD_ERROR_MSG_LEN        11


// Function Declarations
void handle_cd(char **argv, char *envp[]);
void handle_exit(char **argv);
void handle_export(char **argv, char *envp[]);
void expand_variables(char **argv, char *envp[]);
void handle_jobs(char **argv);
void builtin_fg(char **argv);
void builtin_bg(char **argv);

int myatoi(const char *s);
void int_to_str(int n, char *buf);

#endif
