#ifndef GETJOB_H
#define GETJOB_H

#include "jobs.h"
#include "mysh.h"

void get_job(Job *job);
void set_job(Job *job);
void parse_stage(Command * cmd, char *stage_str, Job *job);
int check_read_status(int bytes_read);


#endif
