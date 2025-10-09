#include "mystring.h"
#include "jobs.h"
#include "mysh.h"
#include "myheap.h"
#include <unistd.h>

/* ---
Function Name: main

Purpose: 
  Runs the personal MYSH shell program.

Input:
arc  - number of command-line arguments
argv - array of command-line argument strings
envp - array of environment variable strings

Output: returns 0 upon successful execution
--- */
int main(int argc, char *argv[], char *envp[])
{
  int exitShell = 0;
  Command command;

  get_command(&command);

  while (!exitShell && mystrcmp(command.argv[0], "exit"))
    {
      /* TO DO: process command line */
      run_command(&command);
      get_command(&command);
      /* TO DO: prompt for and read command line */
    }
  
  
  return 0;
}


/* ---
Function Name: get_command

Purpose: 
  Prompts the terminal for user input, reads the input,
  tokenizes it into arguments, and stores the results
  in the provided Command structure.

Input:
  command - pointer to a Command structure to store the parsed input

Output:
  Populates the Command structure with argument count and argument vector.
--- */
void get_command(Command *command)
{
  char *tokens[MAX_ARGS + 1];
  char *command_buffer= alloc(MAX_ARGS + 1);
  int num_tokens;
  int bytes_read;

  char *shell_line = "$ ";

  write(STD_OUT, shell_line, mystrlen(shell_line));
  bytes_read = read(STD_IN, command_buffer, MAX_ARGS);
  
  if (bytes_read < 0) {
    write(STD_ERR, "Error: cannot read\n", 19);
} else if (bytes_read >= MAX_ARGS) {
    write(STD_ERR, "Error: exceeded maximum arguments\n", 35);
}

  command_buffer[bytes_read - 1] = '\0';
  num_tokens = tokenize(command_buffer, tokens, MAX_ARGS);


  // append to Command structure
  for (int i = 0; i < num_tokens && tokens[i] != NULL; i++)
    {
      command->argv[i] = tokens[i];
    }
  
  command->argc = num_tokens;
  
  free_all();
}

/* ---
Function Name: run_command

Purpose: 
  

Input:
  

Output:
  
--- */
int run_command(Command *command) {
  int pid = fork();

  if (pid == -1) {
    // fork failed                                                                                        
    return -1;
  } else if (pid == 0) {
    // child process                                                                                      
    char fullpath[256];
    mystrcpy(fullpath, "/usr/bin/");
    mystrcat(fullpath, command->argv[0]);

    execve(fullpath, command->argv, NULL);
    _exit(1);
    
    // old                                                                                                
    //execve(command->argv[0], command->argv, NULL);                                                      
    //_exit(1);                                                                                           
  } else {
    // parent process                                                                                     
    int status;
    if (waitpid(pid, &status, 0) < 0) {
        return -1; // waitpid failed                                                                      
      }

      if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
      } else {
        return -1;
      }
  }
}

/* ---
Function Name: get_job

Purpose: 
  

Input:
  

Output:
  
--- */

void get_job(Job *job)
{
  // TO DO
}


/* ---
Function Name: run_job

Purpose: 
  

Input:
  

Output:
  
--- */

void run_job(Job *job)
{
  #include "mystring.h"
#include "jobs.h"
#include "myheap.h"
#include <unistd.h>    // fork, pipe, dup2, execve, read, write, _exit
#include <sys/wait.h>  // waitpid

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

/* --- get_job without standard library --- */
void get_job(Job *job)
{
    job->num_stages = 0;
    job->background = 0;
    job->infile_path = NULL;
    job->outfile_path = NULL;

    char *command_buffer = alloc(1024);
    write(STD_OUT, "$ ", 2);
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

/* --- run_job without standard library --- */
void run_job(Job *job)
{
    int num_stages = job->num_stages;
    int pipefd[MAX_PIPELINE_LEN-1][2];

    for (int i=0; i<num_stages-1; i++)
        if (pipe(pipefd[i]) < 0) write(STD_ERR, "pipe failed\n", 12);

    for (int i=0; i<num_stages; i++) {
        int pid = fork();
        if (pid == 0) {
            // input redirection
            if (i == 0 && job->infile_path) {
                int fd = open(job->infile_path, 0); // 0 = read-only
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            // output redirection
            if (i == num_stages-1 && job->outfile_path) {
                int fd = creat(job->outfile_path, 0644); // create file
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            // connect pipes
            if (i > 0) dup2(pipefd[i-1][0], STDIN_FILENO);
            if (i < num_stages-1) dup2(pipefd[i][1], STDOUT_FILENO);

            // close all pipes
            for (int j=0; j<num_stages-1; j++) {
                close(pipefd[j][0]);
                close(pipefd[j][1]);
            }

            char fullpath[256];
            mystrcpy(fullpath, "/usr/bin/");
            mystrcat(fullpath, job->pipeline[i].argv[0]);
            execve(fullpath, job->pipeline[i].argv, NULL);
            _exit(1);
        }
    }

    // close pipes in parent
    for (int i=0; i<num_stages-1; i++) {
        close(pipefd[i][0]);
        close(pipefd[i][1]);
    }

    // wait for children if foreground
    if (!job->background)
        for (int i=0; i<num_stages; i++) wait(NULL);
}

}


/* ---
Function Name: tokenize

Purpose: 
  Splits a command-line buffer into individual tokens (arguments),
  using whitespace (spaces and tabs) as delimiters.

Input:
  buffer     - The input string to tokenize; will be modified in-place.
  tokens     - Array to store pointers to the individual tokens found in buffer.
  max_tokens - Maximum number of tokens to store in the tokens array.

Output:
  Returns the number of tokens parsed and stored in the tokens array.
--- */
int tokenize(char *buffer, char *tokens[], int max_tokens)
{
  int token_count = 0;
  int i = 0;

  while(buffer[i] != '\0' && token_count < max_tokens) {
    
    // ignore whitespaces
    while (buffer[i] == ' ' || buffer[i] == '\t') {
      i++;
    }
    
    if (buffer[i] == '\0') break;
    
    tokens[token_count++] = &buffer[i];
    
    // find end of token
    while (buffer[i] != ' ' && buffer[i] != '\t' && buffer[i] != '\0') {
      i++;
    }

     // null-terminate token if not end of string
    if (buffer[i] != '\0') {
      buffer[i] = '\0';
      i++;
    }
  }
  
  return token_count; 
}

