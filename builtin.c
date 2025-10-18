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
    for (int i = INITIAL_INDEX; envp[i]; i++) {
        int j = INITIAL_INDEX;
        while (envp[i][j] && j < name_len && envp[i][j] == name[j]) j++;
        if (j == name_len && envp[i][j] == ENV_ASSIGN_CHAR) {
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
    if (!dir) dir = get_env_value(HOME_ENV_NAME, envp);
    if (!dir || chdir(dir) < 0) {
        write(STDERR_FILENO, CD_ERROR_MSG, CD_ERROR_MSG_LEN);
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
    int val = 0, i = INITIAL_INDEX, sign = 1;
    if (s[INITIAL_INDEX] == NEGATIVE_SIGN) { 
        sign = -1; 
        i++; 
    }
    while (s[i] != NULL_CHAR) { 
        val = val * DECIMAL_BASE + (s[i] - ZERO_CHAR); 
        i++; 
    }
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

    int i = INITIAL_INDEX;
    while (argv[1][i] && argv[1][i] != ENV_ASSIGN_CHAR) i++;
    if (!argv[1][i]) return;

    argv[1][i] = ENV_TERMINATOR_NULL;
    char *var = argv[1];
    char *val = argv[1] + i + 1;

    for (int e = INITIAL_INDEX; envp[e]; e++) {
        int j = INITIAL_INDEX;
        while (envp[e][j] && var[j] && envp[e][j] == var[j]) j++;
        if (envp[e][j] == ENV_ASSIGN_CHAR && var[j] == ENV_TERMINATOR_NULL) {
            int len = mystrlen(var) + mystrlen(val) + ENV_STRING_EXTRA;
            char *new_entry = alloc(len);
            if (!new_entry) return;
            mystrcpy(new_entry, var);
            mystrcat(new_entry, "=");
            mystrcat(new_entry, val);
            envp[e] = new_entry;
            return;
        }
    }

    for (int e = INITIAL_INDEX; envp[e]; e++) {
        if (!envp[e + 1]) {
            int len = mystrlen(var) + mystrlen(val) + ENV_STRING_EXTRA;
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
    int i = INITIAL_INDEX, start;
    if (n == 0) { 
        buf[INITIAL_INDEX] = ZERO_CHAR; 
        buf[1] = NULL_CHAR; 
        return; 
    }

    int neg = 0;
    if (n < 0) { 
        neg = 1; 
        n = -n; 
    }
    while (n > 0) { 
        buf[i++] = (n % DECIMAL_BASE) + ZERO_CHAR; 
        n /= DECIMAL_BASE; 
    }
    if (neg) buf[i++] = NEGATIVE_SIGN;
    buf[i] = NULL_CHAR;

    for (start = INITIAL_INDEX; start < i / 2; start++) {
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
    for (int i = INITIAL_INDEX; argv[i]; i++) {
        if (argv[i][INITIAL_INDEX] == '$') {
            if (mystrcmp(argv[i], VAR_EXIT_STATUS) == 0) {
                char buf[INT_BUFFER_LEN];
                int_to_str(last_exit_status, buf);
                mystrcpy(argv[i], buf);
            } else {
                char *val = get_env_value(argv[i] + 1, envp);
                if (val) mystrcpy(argv[i], val);
                else argv[i][INITIAL_INDEX] = NULL_CHAR;
            }
        }
    }
}

/* ---
Function Name: handle_jobs

Purpose:
  Displays a list of active jobs currently stored in the global
  jobs array. Each entry shows the job number, state, and command.

Input:
  argv - argument vector from user input (unused).

Output:
  Writes the job list to standard output.

--- */
void handle_jobs(char **argv)
{
    (void)argv;  /* avoid unused warning */

    for (int jobIndex = 0; jobIndex < num_jobs; jobIndex++)
    {
        if (jobs[jobIndex].num_stages > 0)
        {
	  const char *state;

	  if (jobs[jobIndex].background) {
	    state = STATUS_RUNNING_TEXT;
	  } else {
	    state = STATUS_DONE_TEXT;
	  }

            write(STDOUT_FILENO, "[", 1);

            char idBuffer[JOB_DISPLAY_WIDTH];
            myitoa(jobIndex + JOB_ID_OFFSET, idBuffer);
            write(STDOUT_FILENO, idBuffer, mystrlen(idBuffer));

            write(STDOUT_FILENO, "] ", 2);
            write(STDOUT_FILENO, state, mystrlen(state));
            write(STDOUT_FILENO, TERMINAL_TAB_CHAR, 1);

            /* print command name for first stage */
            write(STDOUT_FILENO, jobs[jobIndex].pipeline[0].argv[0],
                  mystrlen(jobs[jobIndex].pipeline[0].argv[0]));
            write(STDOUT_FILENO, JOB_NEWLINE_CHAR, 1);
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
    if (num_jobs == NO_JOBS) return;
    Job *job = &jobs[num_jobs - 1];
    if (job->pgid <= INVALID_PGID) return;

    /* Move job to foreground */
    tcsetpgrp(STDIN_FILENO, job->pgid);

    /* Resume stopped job */
    kill(-job->pgid, SIGCONT);
    job->background = 0;

    int status;
    while (waitpid(-job->pgid, &status, WUNTRACED) > 0) {
        if (WIFSTOPPED(status)) {
            write(STDOUT_FILENO, "[FG stopped]\n", 13);
            return;
        }
        if (WIFSIGNALED(status) || WIFEXITED(status))
            break;
    }

    /* Restore terminal control to shell */
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
    if (num_jobs == NO_JOBS) return;
    Job *job = &jobs[num_jobs - 1];
    if (job->pgid <= INVALID_PGID) return;

    /* Resume stopped job in background */
    kill(-job->pgid, SIGCONT);
    job->background = 1;

    write(STDOUT_FILENO, "[", 1);
    char buf[8];
    myitoa(num_jobs, buf);
    write(STDOUT_FILENO, buf, mystrlen(buf));
    write(STDOUT_FILENO, "] Running\t", 10);
    write(STDOUT_FILENO, job->pipeline[0].argv[0],
          mystrlen(job->pipeline[0].argv[0]));
    write(STDOUT_FILENO, "\n", 1);
}
