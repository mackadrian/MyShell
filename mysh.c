#include <unistd.h> // for read() and write() functions.
#include <string.h>

int main() {
  char string_buffer[256];

  write(1,"$ ", 2);

  while (strcmp(string_buffer, "exit") != 1) {
    if (strcmp(string_buffer, "exit") == 0){
      // exit program
      }
  else {
    write (1, string_buffer, 256);
    write (1, "\n", 1);

    
    }
  };

  return 0;
};
