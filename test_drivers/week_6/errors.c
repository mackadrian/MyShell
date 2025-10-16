#include "errors.h"
#include "mystring.h"
#include <unistd.h>

void print_error(enum ErrorCode code) {
    if (code > 0 && code < NUM_ERRORS && error_messages[code]) {
        write(STD_ERR, error_messages[code], mystrlen(error_messages[code]));
    }
}
