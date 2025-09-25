#include "mystring.h"
#include "jobs.h"

#define MAX_COMMAND_LEN 256
#define STD_OUT 1
#define STD_IN 0

void clear_buffer(char *command_buffer, int bytes_read);


int main(int argc, char *argv[], char *envp[])
{
  int exitShell = 0;
  char command_buffer[MAX_COMMAND_LEN];
  int bytes_read;

  /* TO DO: prompt for and read command line */
  write(STD_OUT, "$ ", 2);
  bytes_read = read(0, command_buffer, 256);
  command_buffer[bytes_read - 1] = '\0';
  
  while (!exitShell)
    {
      /* TO DO: process command line */
      write(STD_OUT, command_buffer, MAX_COMMAND_LEN);
      write(STD_OUT, "\n", 1);

      clear_buffer(command_buffer, bytes_read);
      
      /* TO DO: prompt for and read command line */
      write(STD_OUT, "$ ", 2);
      bytes_read = read(0, command_buffer, MAX_COMMAND_LEN);
      command_buffer[bytes_read - 1] = '\0';

      
    }
  
  
  return 0;
}



void clear_buffer(char *command_buffer, int bytes_read)
{
  for (int i = 0; i < bytes_read; i++)
    {
      command_buffer[i] = '\0';
    }
}
