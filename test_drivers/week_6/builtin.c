#include "builtin.h"
#include "mystring.h"
#include "myheap.h"
#include <unistd.h>
#include <stdlib.h> // only for getenv if allowed

int last_exit_status = 0;

void handle_cd(char **argv) {
    const char *dir = argv[1];
    if (!dir) {
        dir = getenv("HOME");  // you can keep this; getenv is fine
    }
    if (chdir(dir) < 0) {
        write(STDERR_FILENO, "cd: failed\n", 11);
    }
}

int myatoi(const char *s) {
    int val = 0, i = 0;
    int sign = 1;
    if (s[0] == '-') { sign = -1; i++; }
    while (s[i]) { val = val * 10 + (s[i] - '0'); i++; }
    return val * sign;
}

void handle_exit(char **argv) {
    int status = 0;
    if (argv[1]) status = myatoi(argv[1]);
    free_all();
    _exit(status);  // use _exit to avoid stdlib cleanup
}

void handle_export(char **argv) {
    if (!argv[1]) return;

    // split VAR=value manually
    int i = 0;
    while (argv[1][i] && argv[1][i] != '=') i++;
    if (!argv[1][i]) return; // no '='

    argv[1][i] = '\0';
    char *var = argv[1];
    char *val = argv[1] + i + 1;

    // custom setenv replacement
    setenv(var, val, 1);  // if setenv is allowed, otherwise manage envp manually
}

extern int last_exit_status;

void int_to_str(int n, char *buf) {
    int i = 0, start;
    if (n == 0) { buf[0] = '0'; buf[1] = '\0'; return; }

    int neg = 0;
    if (n < 0) { neg = 1; n = -n; }
    while (n > 0) {
        buf[i++] = (n % 10) + '0';
        n /= 10;
    }
    if (neg) buf[i++] = '-';

    buf[i] = '\0';
    // reverse buffer
    for (start = 0; start < i / 2; start++) {
        char tmp = buf[start];
        buf[start] = buf[i - 1 - start];
        buf[i - 1 - start] = tmp;
    }
}

void expand_variables(char **argv) {
    for (int i = 0; argv[i]; i++) {
        if (argv[i][0] == '$') {
            if (mystrcmp(argv[i], "$?") == 0) {
                char buf[16];
                int_to_str(last_exit_status, buf);
                mystrcpy(argv[i], buf);
            } else {
                char *val = getenv(argv[i] + 1);
                if (val) mystrcpy(argv[i], val);
                else argv[i][0] = '\0';
            }
        }
    }
}
