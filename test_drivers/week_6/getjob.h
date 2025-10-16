#ifndef GETJOB_H
#define GETJOB_H

#include "jobs.h"
#include "mysh.h"

void get_job(Job *job);
void set_job(Job *job);
void parse_stage(Command * cmd, char *stage_str, Job *job);
int check_read_status(int bytes_read);

static void normalize_newlines(char *buffer);
static void trim_newline(char *buffer, int bytes_read);
static int skip_leading_whitespace(char *buffer);
static void handle_background(Job *job, char *buffer);
static void parse_pipeline(Job *job, char *buffer, int start);

#endif
