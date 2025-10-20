#ifndef GETJOB_H
#define GETJOB_H

#include "jobs.h"
#include "mysh.h"

/* CHARACTER CONSTANTS */
#define SPACE_CHAR              ' '
#define TAB_CHAR                '\t'
#define NEWLINE_CHAR            '\n'
#define PIPE_CHAR               '|'
#define BACKGROUND_CHAR         '&'
#define INPUT_REDIRECT_CHAR     '<'
#define OUTPUT_REDIRECT_CHAR    '>'
#define NULL_CHAR               '\0'

/* TOKEN CONSTANTS */
#define TOKEN_INPUT             "<"
#define TOKEN_OUTPUT            ">"

/* NUMERIC CONSTANTS */
#define ZERO_VALUE              0
#define TRUE_VALUE              1
#define ERROR_CODE              -1
#define READ_BYTE_COUNT         1

/* FUNCTION DECLARATIONS */
void get_job(Job *job);
void set_job(Job *job);
int check_read_status(int bytes_read);
void parse_stage(Command *cmd, char *stage_str, Job *job);

/* STATIC HELPER FUNCTIONS */
static void parse_argument(Command *cmd, char *token);
static void parse_input_redirection(Job *job, char *stage_str, int *i);
static void parse_output_redirection(Job *job, char *stage_str, int *i);
static void handle_background(Job *job, char *buffer);
static void parse_pipeline(Job *job, char *buffer, int start);
static void normalize_newlines(char *buffer);
static void trim_newline(char *buffer, int bytes_read);
static int skip_leading_whitespace(char *buffer);
static int read_line_from_stdin(char *buffer, int maxlen);

#endif
