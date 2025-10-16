#include "errors.h"
#include "mystring.h"
#include <unistd.h>

/* ---
Function Name: print_error
Purpose:
    Prints a descriptive error message corresponding to the provided ErrorCode.
Input:
    code - an enum value representing the specific error to print
Output:
    Writes the error message to standard error (STD_ERR). If the code
    is invalid or has no associated message, nothing is printed.
--- */
void print_error(enum ErrorCode code) {
    if (code > 0 && code < NUM_ERRORS && error_messages[code]) {
        write(STD_ERR, error_messages[code], mystrlen(error_messages[code]));
    }
}
