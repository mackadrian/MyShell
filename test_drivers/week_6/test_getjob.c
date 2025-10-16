#include "getjob.h"
#include "mystring.h"
#include "myheap.h"
#include "errors.h"
#include "jobs.h"

#include <stdio.h>
#include <string.h>

// FUNCTION DECLARATIONS
void print_job(Job *job);
void test_normal_command();
void test_pipeline_command();
void test_background_command();
void test_redirection_command();
void test_bytes_read_negative();
void test_bytes_read_zero();
void test_bytes_read_overflow();
void test_get_job_from_stdin();

// MAIN
int main(void)
{
    test_normal_command();
    test_pipeline_command();
    test_background_command();
    test_redirection_command();
    test_bytes_read_negative();
    test_bytes_read_zero();
    test_bytes_read_overflow();

    printf("Integration test: get_job() reading from stdin\n");
    printf("Feed input via stdin (Ctrl+D to end if typing manually)\n");
    test_get_job_from_stdin();

    return 0;
}

// FUNCTION DEFINITIONS

/* ---
Function Name: print_job
Purpose:
    Prints a Job structure, including stages, arguments, and input/output redirection.
Input:
    job - pointer to a Job structure
Output:
    Prints the parsed job to stdout
--- */
void print_job(Job *job)
{
    printf("Number of stages: %d\n", job->num_stages);
    printf("Background flag: %d\n", job->background);
    if (job->infile_path) printf("Input file: %s\n", job->infile_path);
    if (job->outfile_path) printf("Output file: %s\n", job->outfile_path);

    for (int i = 0; i < job->num_stages; i++) {
        Command *cmd = &job->pipeline[i];
        printf("  Stage %d argc: %d\n", i, cmd->argc);
        for (int j = 0; j < cmd->argc; j++) {
            printf("    argv[%d]: %s\n", j, cmd->argv[j]);
        }
    }
    printf("-------------------------------------------------\n");
}

/* ---
Function Name: test_normal_command
Purpose:
    Tests a simple single-stage command
--- */
void test_normal_command()
{
    Job job;
    char command[] = "ls -l";

    set_job(&job);
    parse_stage(&job.pipeline[job.num_stages], command, &job);
    job.num_stages++;

    printf("Test: %s\n", command);
    print_job(&job);
}

/* ---
Function Name: test_pipeline_command
Purpose:
    Tests a multi-stage pipeline command
--- */
void test_pipeline_command()
{
    Job job;
    char stage1[] = "cat file.txt";
    char stage2[] = "grep foo";
    char stage3[] = "sort";

    set_job(&job);
    parse_stage(&job.pipeline[job.num_stages], stage1, &job); job.num_stages++;
    parse_stage(&job.pipeline[job.num_stages], stage2, &job); job.num_stages++;
    parse_stage(&job.pipeline[job.num_stages], stage3, &job); job.num_stages++;

    printf("Test: cat file.txt | grep foo | sort\n");
    print_job(&job);
}

/* ---
Function Name: test_background_command
Purpose:
    Tests a command with background execution
--- */
void test_background_command()
{
    Job job;
    char command[] = "echo hello world &";

    set_job(&job);
    /* simulate '&' handling */
    int len = mystrlen(command);
    if (command[len - 1] == '&') {
        job.background = 1;
        command[len - 1] = '\0';
    }

    parse_stage(&job.pipeline[job.num_stages], command, &job);
    job.num_stages++;

    printf("Test: echo hello world &\n");
    print_job(&job);
}

/* ---
Function Name: test_redirection_command
Purpose:
    Tests input and output redirection
--- */
void test_redirection_command()
{
    Job job;
    char stage1[] = "sort < unsorted.txt";
    char stage2[] = "uniq > result.txt";

    set_job(&job);
    parse_stage(&job.pipeline[job.num_stages], stage1, &job); job.num_stages++;
    parse_stage(&job.pipeline[job.num_stages], stage2, &job); job.num_stages++;

    printf("Test: sort < unsorted.txt | uniq > result.txt\n");
    print_job(&job);
}

/* ---
Function Name: test_bytes_read_negative
Purpose:
    Simulates bytes_read < 0 (read error)
--- */
void test_bytes_read_negative()
{
    printf("Test: bytes_read < 0\n");
    int status = check_read_status(-1);
    printf("check_read_status returned %d\n", status);
    printf("-------------------------------------------------\n");
}

/* ---
Function Name: test_bytes_read_zero
Purpose:
    Simulates bytes_read == 0 (EOF)
--- */
void test_bytes_read_zero()
{
    printf("Test: bytes_read == 0\n");
    int status = check_read_status(0);
    printf("check_read_status returned %d\n", status);
    printf("-------------------------------------------------\n");
}

/* ---
Function Name: test_bytes_read_overflow
Purpose:
    Simulates bytes_read > MAX_ARGS
--- */
void test_bytes_read_overflow()
{
    printf("Test: bytes_read > MAX_ARGS\n");
    int status = check_read_status(MAX_ARGS + 100);
    printf("check_read_status returned %d\n", status);
    printf("-------------------------------------------------\n");
}

/* ---
Function Name: test_get_job_from_stdin
Purpose:
    Tests get_job() with actual stdin input (can feed file via < or pipe)
--- */
void test_get_job_from_stdin()
{
    Job job;
    set_job(&job);

    get_job(&job);  /* reads from stdin */

    print_job(&job);
}
