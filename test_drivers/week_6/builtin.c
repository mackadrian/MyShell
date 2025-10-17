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

/* ---
Function Name: get_env_value
Purpose:
    Retrieves the value of an environment variable manually from envp.
Input:
    name - variable name
    envp - environment variable array
Output:
    Pointer to value string, or NULL if not found.
--- */
static char* get_env_value(const char *name, char *envp[]) {
    int name_len = mystrlen(name);
    for (int i = 0; envp[i]; i++) {
        int j = 0;
        while (envp[i][j] && j < name_len && envp[i][j] == name[j]) j++;
        if (j == name_len && envp[i][j] == '=') {
            return envp[i] + j + 1;
        }
    }
    return NULL;
}


/* ---
Function Name: handle_cd
Purpose:
    Implements the 'cd' builtin command.
Input:
    argv - argument list
    envp - environment variables
Output:
    Changes current directory, prints error on failure.
--- */
void handle_cd(char **argv, char *envp[]) {
    const char *dir = argv[1];
    if (!dir) {
        dir = get_env_value("HOME", envp);
    }
    if (!dir || chdir(dir) < 0) {
        write(STDERR_FILENO, "cd: failed\n", 11);
    }
}


/* ---
Function Name: myatoi
Purpose:
    Converts a string to an integer.
Input:
    s - input string
Output:
    Integer representation of the string.
--- */
int myatoi(const char *s) {
    int val = 0, i = 0, sign = 1;
    if (s[0] == '-') { sign = -1; i++; }
    while (s[i]) { val = val * 10 + (s[i] - '0'); i++; }
    return val * sign;
}

/* ---
Function Name: handle_exit
Purpose:
    Implements the 'exit' builtin command.
Input:
    argv - argument list
Output:
    Exits the shell process with the provided status.
--- */
void handle_exit(char **argv) {
    int status = 0;
    if (argv[1]) status = myatoi(argv[1]);
    free_all();
    _exit(status);
}

/* ---
Function Name: handle_export
Purpose:
    Implements 'export' command using manual envp updates.
Input:
    argv - argument list ("VAR=value")
    envp - environment variable array
Output:
    Adds or updates the environment variable.
--- */
void handle_export(char **argv, char *envp[]) {
    if (!argv[1]) return;

    int i = 0;
    while (argv[1][i] && argv[1][i] != '=') i++;
    if (!argv[1][i]) return;

    argv[1][i] = '\0';
    char *var = argv[1];
    char *val = argv[1] + i + 1;

    // Replace if already exists
    for (int e = 0; envp[e]; e++) {
        int j = 0;
        while (envp[e][j] && var[j] && envp[e][j] == var[j]) j++;
        if (envp[e][j] == '=' && var[j] == '\0') {
            int len = mystrlen(var) + mystrlen(val) + 2;
            char *new_entry = alloc(len);
            if (!new_entry) return;
            mystrcpy(new_entry, var);
            mystrcat(new_entry, "=");
            mystrcat(new_entry, val);
            envp[e] = new_entry;
            return;
        }
    }

    // Append new variable if not found
    for (int e = 0; envp[e]; e++) {
        if (!envp[e + 1]) {
            int len = mystrlen(var) + mystrlen(val) + 2;
            char *new_entry = alloc(len);
            if (!new_entry) return;
            mystrcpy(new_entry, var);
            mystrcat(new_entry, "=");
            mystrcat(new_entry, val);
            envp[e + 1] = new_entry;
            envp[e + 2] = NULL;
            return;
        }
    }
}

/* ---
Function Name: int_to_str
Purpose:
    Converts an integer to a string.
Input:
    n - integer
    buf - output buffer
Output:
    String representation of n stored in buf.
--- */
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

/* ---
Function Name: expand_variables
Purpose:
    Expands $VAR and $? in argument list.
Input:
    argv - argument list
    envp - environment variables
Output:
    Modifies argv in place with expanded values.
--- */
void expand_variables(char **argv, char *envp[]) {
    for (int i = 0; argv[i]; i++) {
        if (argv[i][0] == '$') {
            if (mystrcmp(argv[i], "$?") == 0) {
                char buf[16];
                int_to_str(last_exit_status, buf);
                mystrcpy(argv[i], buf);
            } else {
                char *val = get_env_value(argv[i] + 1, envp);
                if (val) mystrcpy(argv[i], val);
                else argv[i][0] = '\0';
            }
        }
    }
}

/* ---
Function Name: builtin_fg
Purpose:
    Brings the most recent background job to the foreground.
Input:
    argv - unused argument list
Output:
    Transfers terminal control and waits for job completion.
--- */
void builtin_fg(char **argv) {
    if (num_jobs == 0) return;
    Job *job = &jobs[num_jobs - 1];
    if (job->pgid <= 0) return;

    tcsetpgrp(STDIN_FILENO, job->pgid);
    kill(-job->pgid, SIGCONT);
    int status;
    waitpid(-job->pgid, &status, WUNTRACED);
    tcsetpgrp(STDIN_FILENO, shell_pgid);
}

/* ---
Function Name: builtin_bg
Purpose:
    Resumes the most recent stopped job in the background.
Input:
    argv - unused argument list
Output:
    Sends SIGCONT to the job’s process group.
--- */
void builtin_bg(char **argv) {
    if (num_jobs == 0) return;
    Job *job = &jobs[num_jobs - 1];
    if (job->pgid <= 0) return;
    kill(-job->pgid, SIGCONT);
}
