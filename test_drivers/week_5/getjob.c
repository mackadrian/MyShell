#include "getjob.h"
#include "mystring.h"
#include "myheap.h"
#include <unistd.h>    // fork, pipe, dup2, execve, read, write, _exit
#include <sys/wait.h>  // waitpid

/* ---
Function Name: get_job

Purpose: 
  

Input:
  

Output:
  
--- */
void get_job(Job *job)
{
    job->num_stages = 0;
    job->background = 0;
    job->infile_path = NULL;
    job->outfile_path = NULL;

    char *command_buffer = alloc(1024);
    
    write(STD_OUT, SHELL, mystrlen(SHELL));
    
    int bytes_read = read(STD_IN, command_buffer, 1024);
    if (bytes_read <= 0) return;
    command_buffer[bytes_read - 1] = '\0';

    // check background
    int len = mystrlen(command_buffer);
    if (command_buffer[len-1] == '&') {
        job->background = 1;
        command_buffer[len-1] = '\0';
    }

    // manual split by pipe '|'
    int i = 0, stage_start = 0;
    while (1) {
        if (command_buffer[i] == '|' || command_buffer[i] == '\0') {
            command_buffer[i] = '\0';
            parse_stage(&job->pipeline[job->num_stages], &command_buffer[stage_start]);
            job->num_stages++;
            if (command_buffer[i] == '\0') break;
            stage_start = i + 1;
        }
        i++;
    }

    free_all();
}

/* --- manual stage tokenizer --- */
void parse_stage(Command *cmd, char *stage_str)
{
    cmd->argc = 0;
    int i = 0, start = 0;
    while (stage_str[i] != '\0') {
        // skip spaces
        while (stage_str[i] == ' ' || stage_str[i] == '\t') i++;
        if (stage_str[i] == '\0') break;

        start = i;
        while (stage_str[i] != ' ' && stage_str[i] != '\t' && stage_str[i] != '\0') i++;

        stage_str[i] = '\0'; // terminate token
        cmd->argv[cmd->argc++] = &stage_str[start];
        i++;
    }
    cmd->argv[cmd->argc] = NULL;
}
