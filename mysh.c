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
  
  //while (!exitShell)
  //  {
      /* TO DO: process command line */
      
      /* TO DO: prompt for and read command line */
      
  //  }
  
  
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
  int bytes_read;
  int command_buffer[MAX_ARGS];

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
  

  
    
}
