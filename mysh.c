#include <unistd.h> // for read() and write() functions.
#include <string.h> //for strcomp

int main() {
  char string_buffer[256];
  int bytes_read;

  write(1, "$ ", 2);
  bytes_read = read(0, string_buffer, 256);
  string_buffer[bytes_read-1] = '\0';
  
  while (strcmp(string_buffer, "exit") != 0) {

    write (1, string_buffer, 256);
    write (1, "\n", 1);
    
    write(1, "$ ", 2);
    bytes_read = read(0, string_buffer, 256);
    string_buffer[bytes_read-1] = '\0';  
    
  }

  write(1, "quitting program\n", 17);
  
  return 0;
}
