#include "mystring.h"
#include "jobs.h"
#include "mysh.h"
#include "myheap.h"
#include "stdlib.h" // to make use of exit() system call

/* ---
Function Name: main

Purpose: 
  Runs the personal MYSH shell program.

Input:
arc -
argv -
envp -


Output:
--- */
int main(int argc, char *argv[], char *envp[])
{
  int exitShell = 0;
  Command command;

  /* TO DO: prompt for and read command line */
  get_command(&command);
  
  while (!exitShell)
  {
      /* TO DO: process command line */
  
      
      /* TO DO: prompt for and read command line */

  }
  
  
  return 0;
}


/* ---
Function Name: get_command

Purpose: 
  Prompts the terminal for input.

Input:
  command -

Output:
--- */
void get_command(Command *command)
{
  char *tokens[MAX_ARGS + 1];
  char *command_buffer = alloc(MAX_ARGS);
  int num_tokens;
  int bytes_read;
  
  

  write(STD_OUT, "$ ", 2);
  bytes_read = read(STD_IN, command_buffer, MAX_ARGS);
  
  if (bytes_read < 0) {
    write(STD_ERR, "Error: cannot read\n", 19);
    exit(1);
  } else if (bytes_read >= MAX_ARGS) {
    write(STD_ERR, "Error: exceeded maximum arguments\n", 35);
    exit (1);
  }
  
  command_buffer[bytes_read - 1] = '\0';
  num_tokens = tokenize(command_buffer, tokens, MAX_ARGS);
  free_all();


  //test print
  printf("Number of tokens: %d\n", num_tokens);
  for (int i = 0; i < num_tokens; i++){
    printf("Token[%d] %s\n", i, tokens[i]);
  }
  
  
				   
      
  
}


/* ---
Function Name: tokenize

Purpose: 

Input:
  command -

Output:
--- */
int tokenize(char *buffer, char *tokens[], int max_tokens)
{
  int token_count = 0;
  int i = 0;

  while(buffer[i] != '\0' && token_count < max_tokens) {
    
    // ignore whitespaces
    while (buffer[i] == ' ' || buffer[i] == '\t') i++;
    if (buffer[i] == '\0') break;

    tokens[token_count++] = &buffer[i];
    
    // find end of token
    while (buffer[i] != ' ' && buffer[i] != '\t' && buffer[i] != '\0') i++;
    
    if (buffer[i] != '\0') {
      buffer[i] = '\0';
      i++;
    }
  }

  return token_count;
}
      
