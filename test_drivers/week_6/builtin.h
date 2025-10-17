#ifndef BUILTIN_H
#define BUILTIN_H

extern int last_exit_status;

void handle_cd(char **argv, char *envp[]);
void handle_exit(char **argv);
void handle_export(char **argv, char *envp[]);
void expand_variables(char **argv, char *envp[]);
void builtin_fg(char **argv);
void builtin_bg(char **argv);

int myatoi(const char *s);
void int_to_str(int n, char *buf);

#define CD_ERROR_MSG            "cd: failed\n"
#define CD_ERROR_MSG_LEN        11

#define HOME_ENV_NAME           "HOME"
#define ENV_ASSIGN_CHAR         '='
#define ENV_TERMINATOR_NULL     '\0'

#define DECIMAL_BASE            10
#define INITIAL_INDEX           0
#define NEGATIVE_SIGN           '-'
#define ZERO_CHAR               '0'
#define NULL_CHAR               '\0'

#define INT_BUFFER_LEN          16
#define ENV_STRING_EXTRA        2

#define NO_JOBS                 0
#define INVALID_PGID            0

#define VAR_EXIT_STATUS         "$?"

#endif
