#include "mystring.h"
#include "jobs.h"
#include "mysh.h"
#include "myheap.h"
#include <unistd.h>
#include <stdio.h> // for testing

/* ---
Function Name: main

Purpose: 
 Tests the personal MYSH shell program.

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



  printf("Running get_command()... \n");
  get_command(&command);

  printf("Print out appended Command structure... \n");
  printf("Press ENTER to continue... \n");
  getchar();


  printf("command->argc = %d arguments\n", command.argc);
  for(int i = 0; i < command.argc; i++) {
    printf("command->argv[%d] = %s\n", i, command.argv[i]); 
  }


  
  while (!exitShell && mystrcmp(command.argv[0], "exit"))
    {
     
    
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
  
---
int run_command(Command *command) {
  int pid = fork();

  if (pid == -1) {
    // fork failed
    return -1;
  } else if (pid == 0) {
    // child process
    execve(command->argv[0], command->argv, NULL);
    _exit(1);
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
  } */

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

