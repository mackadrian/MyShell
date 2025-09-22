#include <unistd.h> // for read() and write() functions.
#include <string.h>

int main() {
  char string_buffer[256];
  int bytes_read;

  write(1,"$ ",2);
  bytes_read = read(0, string_buffer, 256);
  string_buffer[bytes_read-1] = '\0';

  while (strcmp(string_buffer, "exit") != 0) {
    if (strcmp(string_buffer, "exit") == 0){
      write(1, "quitting program\n", 17);
    }
      
    else {
      write (1, string_buffer, 256);
      write (1, "\n", 1);

      write(1, "$ ", 2);
      bytes_read = read(0, string_buffer, 256);
      string_buffer[bytes_read-1] = '\0';
    }
    
  }
  
  return 0;
};
