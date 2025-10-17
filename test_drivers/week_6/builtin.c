#include "builtin.h"
#include "mystring.h"
#include "myheap.h"
#include "jobs.h"

#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>

Job jobs[MAX_JOBS];
int num_jobs = 0;
int last_exit_status = 0;
int shell_pgid = 0;
struct termios shell_tmodes;

void handle_cd(char **argv) {
    const char *dir = argv[1];
    if (!dir) {
        dir = getenv("HOME");
    }
    if (chdir(dir) < 0) {
        write(STDERR_FILENO, "cd: failed\n", 11);
    }
}

int myatoi(const char *s) {
    int val = 0, i = 0, sign = 1;
    if (s[0] == '-') { sign = -1; i++; }
    while (s[i]) { val = val * 10 + (s[i] - '0'); i++; }
    return val * sign;
}

void handle_exit(char **argv) {
    int status = 0;
    if (argv[1]) status = myatoi(argv[1]);
    free_all();
    _exit(status);
}

void handle_export(char **argv) {
    if (!argv[1]) return;
    int i = 0;
    while (argv[1][i] && argv[1][i] != '=') i++;
    if (!argv[1][i]) return;

    argv[1][i] = '\0';
    char *var = argv[1];
    char *val = argv[1] + i + 1;

    setenv(var, val, 1);
}

void int_to_str(int n, char *buf) {
    int i = 0, start;
    if (n == 0) { buf[0] = '0'; buf[1] = '\0'; return; }

    int neg = 0;
    if (n < 0) { neg = 1; n = -n; }
    while (n > 0) { buf[i++] = (n % 10) + '0'; n /= 10; }
    if (neg) buf[i++] = '-';
    buf[i] = '\0';

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

void builtin_fg(char **argv) {
    if (num_jobs == 0) return;
    Job *job = &jobs[num_jobs - 1]; // pick most recent job
    if (job->pgid <= 0) return;

    tcsetpgrp(STDIN_FILENO, job->pgid); // give terminal
    kill(-job->pgid, SIGCONT);          // continue job
    int status;
    waitpid(-job->pgid, &status, WUNTRACED);
    tcsetpgrp(STDIN_FILENO, shell_pgid);
}

void builtin_bg(char **argv) {
    if (num_jobs == 0) return;
    Job *job = &jobs[num_jobs - 1]; // pick most recent stopped job
    if (job->pgid <= 0) return;
    kill(-job->pgid, SIGCONT);      // resume in background
}
