#include "runjob.h"
#include "mystring.h"
#include "myheap.h"
#include "errors.h"
#include "jobs.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

/* Dummy globals for unit testing */
Job jobs[MAX_JOBS];
int num_jobs = 0;

// FUNCTION DECLARATIONS
static void print_job(Job *job);
static void set_test_job(Job *job);
static void build_cmdline(Job *job, char *cmdline, size_t size);

static void test_normal_command(char *envp[]);
static void test_pipeline_command(char *envp[]);
static void test_background_command(char *envp[]);
static void test_redirection_command(char *envp[]);
static void test_invalid_command(char *envp[]);
static void test_missing_file_command(char *envp[]);
static void test_sigint_handling(char *envp[]);

static void prepare_input_file(const char *filename);
static void check_output_file(const char *filename);
static void test_cat_input_redirection(char *envp[]);
static void test_sort_pipe_uniq_with_redirection(char *envp[]);
static void test_output_redirection_only(char *envp[]);
static void test_combined_redirection_and_pipeline(char *envp[]);
static void test_cat_frankenstein(char *envp[]);
static void test_less_frankenstein(char *envp[]);

// MAIN
int main(int argc, char *argv[], char *envp[])
{
    printf("=== Standard RunJob Tests ===\n");
    test_normal_command(envp);
    test_pipeline_command(envp);
    test_background_command(envp);

    printf("\n=== Redirection Tests ===\n");
    test_redirection_command(envp);
    test_cat_input_redirection(envp);
    test_sort_pipe_uniq_with_redirection(envp);
    test_output_redirection_only(envp);
    test_combined_redirection_and_pipeline(envp);
    test_cat_frankenstein(envp);

    printf("\n=== Error Handling Tests ===\n");
    test_invalid_command(envp);
    test_missing_file_command(envp);

    printf("\n=== Signal Handling Tests ===\n");
    test_sigint_handling(envp);

    printf("\n=== Interactive Pager Test ===\n");
    test_less_frankenstein(envp);
    

    return 0;
}

// FUNCTION DEFINITIONS
/* ---
Function Name: print_job
Purpose:
    Displays the details of a Job structure for debugging and verification.
    Prints all stages, their arguments, and any input/output redirection
    associated with the job.
Input:
    job - pointer to a Job structure containing pipeline and redirection data
Output:
    Prints the job configuration (number of stages, background flag,
    redirection paths, and command arguments) to stdout.
--- */
static void print_job(Job *job)
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
Function Name: set_test_job
Purpose:
    Initializes a Job structure for testing by resetting all fields to 
    default values. Clears previous command arguments, redirection paths,
    and stage data to ensure a clean starting state.
Input:
    job - pointer to a Job structure to reset
Output:
    Job structure fields (num_stages, background flag, pipeline arguments,
    redirection paths) are set to initial values.
--- */
static void set_test_job(Job *job)
{
    job->num_stages = 0;
    job->background = 0;
    job->infile_path = NULL;
    job->outfile_path = NULL;
    for (int i = 0; i < MAX_PIPELINE_LEN; i++) {
        job->pipeline[i].argc = 0;
        for (int j = 0; j < MAX_ARGS; j++) job->pipeline[i].argv[j] = NULL;
    }
}

/* ---
Function Name: build_cmdline
Purpose:
    Constructs a readable command-line string from a Job structure by 
    concatenating all stage arguments and pipe ('|') separators. If the 
    background flag is set, an '&' is appended to the end of the command.
Input:
    job     - pointer to a Job structure containing command data
    cmdline - destination buffer for the constructed command line
    size    - total size of the destination buffer
Output:
    Fills the cmdline buffer with a string representation of the job
    (e.g., "cat file.txt | grep foo | sort &").
--- */
static void build_cmdline(Job *job, char *cmdline, size_t size)
{
    cmdline[0] = '\0';
    for (int i = 0; i < job->num_stages; i++) {
        for (int j = 0; j < job->pipeline[i].argc; j++) {
            strncat(cmdline, job->pipeline[i].argv[j], size - strlen(cmdline) - 1);
            strncat(cmdline, " ", size - strlen(cmdline) - 1);
        }
        if (i < job->num_stages - 1) strncat(cmdline, "| ", size - strlen(cmdline) - 1);
    }
    if (job->background) strncat(cmdline, "&", size - strlen(cmdline) - 1);
}

/* ---
Function Name: test_normal_command
Purpose:
    Tests execution of a simple, single-stage command (non-pipeline) using run_job().
    Verifies that a standard command like 'ls -l' runs correctly without input/output
    redirection or background execution.
Input:
    envp - environment variable array passed to run_job()
Output:
    Prints the constructed command line, job structure details, and the command output.
    Expected result: directory listing from 'ls -l'.
--- */
static void test_normal_command(char *envp[])
{
    Job job;
    set_test_job(&job);

    job.num_stages = 1;
    job.pipeline[0].argc = 2;
    job.pipeline[0].argv[0] = "ls";
    job.pipeline[0].argv[1] = "-l";
    job.pipeline[0].argv[2] = NULL;

    char cmdline[256];
    build_cmdline(&job, cmdline, sizeof(cmdline));
    printf("Test: %s\n", cmdline);
    print_job(&job);

    run_job(&job, envp);
    printf("-------------------------------------------------\n");
}


/* ---
Function Name: test_pipeline_command
Purpose:
    Tests execution of a multi-stage pipeline (e.g., echo | grep | sort).
Input:
    envp - environment variable array passed to execve().
Output:
    Verifies correct pipe setup and sequential data flow between commands.
--- */
static void test_pipeline_command(char *envp[])
{
    Job job;
    set_test_job(&job);

    job.pipeline[0].argc = 3;
    job.pipeline[0].argv[0] = "echo";
    job.pipeline[0].argv[1] = "-e";
    job.pipeline[0].argv[2] = "foo\\nbar\\nbaz";
    job.pipeline[0].argv[3] = NULL;

    job.pipeline[1].argc = 2;
    job.pipeline[1].argv[0] = "grep";
    job.pipeline[1].argv[1] = "ba";
    job.pipeline[1].argv[2] = NULL;

    job.pipeline[2].argc = 1;
    job.pipeline[2].argv[0] = "sort";
    job.pipeline[2].argv[1] = NULL;

    job.num_stages = 3;

    char cmdline[512];
    build_cmdline(&job, cmdline, sizeof(cmdline));
    printf("Test: %s\n", cmdline);
    print_job(&job);

    run_job(&job, envp);
    printf("-------------------------------------------------\n");
}

/* ---
Function Name: test_background_command
Purpose:
    Tests background execution (e.g., sleep 2 &) and ensures non-blocking behavior.
Input:
    envp - environment variable array passed to execve().
Output:
    Prints job details and verifies job runs in background.
--- */
static void test_background_command(char *envp[])
{
    Job job;
    set_test_job(&job);

    job.num_stages = 1;
    job.background = 1;
    job.pipeline[0].argc = 2;
    job.pipeline[0].argv[0] = "sleep";
    job.pipeline[0].argv[1] = "2";
    job.pipeline[0].argv[2] = NULL;

    char cmdline[128];
    build_cmdline(&job, cmdline, sizeof(cmdline));
    printf("Test: %s\n", cmdline);
    print_job(&job);

    run_job(&job, envp);
    printf("-------------------------------------------------\n");
}

/* ---
Function Name: test_redirection_command
Purpose:
    Tests combined input and output redirection through a simple pipeline.
Input:
    envp - environment variable array passed to execve().
Output:
    Writes to output.txt after reading from input.txt, verifies file redirection.
--- */
static void test_redirection_command(char *envp[])
{
    Job job;
    set_test_job(&job);

    job.num_stages = 2;
    job.infile_path = "input.txt";
    job.outfile_path = "output.txt";

    job.pipeline[0].argc = 1;
    job.pipeline[0].argv[0] = "sort";
    job.pipeline[0].argv[1] = NULL;

    job.pipeline[1].argc = 1;
    job.pipeline[1].argv[0] = "uniq";
    job.pipeline[1].argv[1] = NULL;

    char cmdline[256];
    build_cmdline(&job, cmdline, sizeof(cmdline));
    printf("Test: %s < %s | %s > %s\n",
           job.pipeline[0].argv[0], job.infile_path,
           job.pipeline[1].argv[0], job.outfile_path);
    print_job(&job);

    run_job(&job, envp);
    printf("-------------------------------------------------\n");
}

/* ---
Function Name: test_invalid_command
Purpose:
    Tests handling of an invalid executable (command not found).
Input:
    envp - environment variable array passed to execve().
Output:
    Expects and reports an appropriate error message.
--- */
static void test_invalid_command(char *envp[])
{
    Job job;
    set_test_job(&job);

    job.num_stages = 1;
    job.pipeline[0].argc = 1;
    job.pipeline[0].argv[0] = "nonexistentcmd";
    job.pipeline[0].argv[1] = NULL;

    char cmdline[128];
    build_cmdline(&job, cmdline, sizeof(cmdline));
    printf("Test: %s\n", cmdline);
    print_job(&job);

    run_job(&job, envp);
    printf("Expected error: %s", error_messages[ERR_CMD_NOT_FOUND]);
    printf("-------------------------------------------------\n");
}

/* ---
Function Name: test_missing_file_command
Purpose:
    Tests handling of a missing input file for redirection.
Input:
    envp - environment variable array passed to execve().
Output:
    Expects file not found error and verifies graceful failure.
--- */
static void test_missing_file_command(char *envp[])
{
    Job job;
    set_test_job(&job);

    job.num_stages = 1;
    job.infile_path = "nonexistentfile.txt";
    job.pipeline[0].argc = 1;
    job.pipeline[0].argv[0] = "cat";
    job.pipeline[0].argv[1] = NULL;

    char cmdline[128];
    build_cmdline(&job, cmdline, sizeof(cmdline));
    printf("Test: %s\n", cmdline);
    print_job(&job);

    run_job(&job, envp);
    printf("Expected error: %s", error_messages[ERR_FILE_NOT_FOUND]);
    printf("-------------------------------------------------\n");
}

/* ---
Function Name: test_sigint_handling
Purpose:
    Tests process response to SIGINT (Ctrl+C) while running a long command.
Input:
    envp - environment variable array passed to execve().
Output:
    Observes signal handling and ensures process terminates cleanly.
--- */
static void test_sigint_handling(char *envp[])
{
    Job job;
    set_test_job(&job);

    job.num_stages = 1;
    job.pipeline[0].argc = 2;
    job.pipeline[0].argv[0] = "sleep";
    job.pipeline[0].argv[1] = "5";
    job.pipeline[0].argv[2] = NULL;

    char cmdline[128];
    build_cmdline(&job, cmdline, sizeof(cmdline));
    printf("Test: %s (sending SIGINT after 5 second)\n", cmdline);
    print_job(&job);
    run_job(&job, envp);

    printf("-------------------------------------------------\n");
}

/* ---
Function Name: prepare_input_file
Purpose:
    Creates a test input file with predefined sample data for use in 
    redirection and pipeline tests.
Input:
    filename - name of the file to create and write sample data into.
Output:
    Writes several lines of test text (e.g., fruit names) to the specified file.
--- */
static void prepare_input_file(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) return;
    fprintf(f, "apple\nbanana\napple\ncherry\nbanana\n");
    fclose(f);
}

/* ---
Function Name: check_output_file
Purpose:
    Opens and displays the contents of an output file after command execution.
Input:
    filename - name of the output file to open and read.
Output:
    Prints file contents to stdout; reports if file cannot be found.
--- */
static void check_output_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        printf("Output file %s not found\n", filename);
        return;
    }
 
    printf("Contents of %s:\n", filename);
    char line[128];
    while (fgets(line, sizeof(line), f)) {
        printf("%s", line);
    }
    fclose(f);
    printf("-------------------------------------------------\n");
}

/* ---
Function Name: test_cat_input_redirection
Purpose:
    Tests input redirection with a single-stage 'cat' command.
Input:
    envp - environment variable array passed to execve().
Output:
    Reads and prints contents of input.txt, verifying input redirection.
--- */
static void test_cat_input_redirection(char *envp[]) {
    printf("=== Test: cat < input.txt ===\n");
    prepare_input_file("input.txt");

    Job job;
    set_test_job(&job);

    job.num_stages = 1;
    job.infile_path = "input.txt";
    job.pipeline[0].argc = 1;
    job.pipeline[0].argv[0] = "cat";
    job.pipeline[0].argv[1] = NULL;

    run_job(&job, envp);
    printf("Expected output: contents of input.txt\n");
    printf("-------------------------------------------------\n");
}

/* ---
Function Name: test_sort_pipe_uniq_with_redirection
Purpose:
    Tests full pipeline with both input and output redirection:
        sort < input.txt | uniq > output.txt
Input:
    envp - environment variable array passed to execve().
Output:
    Creates sorted, unique lines in output.txt for verification.
--- */
static void test_sort_pipe_uniq_with_redirection(char *envp[]) {
    printf("=== Test: sort < input.txt | uniq > output.txt ===\n");
    prepare_input_file("input.txt");

    Job job;
    set_test_job(&job);

    job.num_stages = 2;
    job.infile_path = "input.txt";
    job.outfile_path = "output.txt";

    job.pipeline[0].argc = 1;
    job.pipeline[0].argv[0] = "sort";
    job.pipeline[0].argv[1] = NULL;

    job.pipeline[1].argc = 1;
    job.pipeline[1].argv[0] = "uniq";
    job.pipeline[1].argv[1] = NULL;

    run_job(&job, envp);
    check_output_file("output.txt");
}

/* ---
Function Name: test_output_redirection_only
Purpose:
    Tests output redirection for a simple echo command.
Input:
    envp - environment variable array passed to execve().
Output:
    Writes “hello world” to output.txt and verifies correctness.
--- */
static void test_output_redirection_only(char *envp[]) {
    printf("=== Test: echo hello world > output.txt ===\n");

    Job job;
    set_test_job(&job);

    job.num_stages = 1;
    job.outfile_path = "output.txt";

    job.pipeline[0].argc = 3;
    job.pipeline[0].argv[0] = "echo";
    job.pipeline[0].argv[1] = "hello";
    job.pipeline[0].argv[2] = "world";
    job.pipeline[0].argv[3] = NULL;

    run_job(&job, envp);
    check_output_file("output.txt");
}

/* ---
Function Name: test_combined_redirection_and_pipeline
Purpose:
    Tests complex case: input redirection, pipeline, and output redirection.
Input:
    envp - environment variable array passed to execve().
Output:
    Produces filtered and sorted output written to output.txt.
--- */
static void test_combined_redirection_and_pipeline(char *envp[]) {
    printf("=== Test: cat < input.txt | grep a | sort > output.txt ===\n");
    prepare_input_file("input.txt");

    Job job;
    set_test_job(&job);

    job.num_stages = 3;
    job.infile_path = "input.txt";
    job.outfile_path = "output.txt";

    job.pipeline[0].argc = 1;
    job.pipeline[0].argv[0] = "cat";
    job.pipeline[0].argv[1] = NULL;

    job.pipeline[1].argc = 2;
    job.pipeline[1].argv[0] = "grep";
    job.pipeline[1].argv[1] = "a";
    job.pipeline[1].argv[2] = NULL;

    job.pipeline[2].argc = 1;
    job.pipeline[2].argv[0] = "sort";
    job.pipeline[2].argv[1] = NULL;

    run_job(&job, envp);
    check_output_file("output.txt");
}

/* ---
Function Name: test_cat_frankenstein
Purpose:
    Tests reading large file via input redirection (cat < frankenstein.txt).
Input:
    envp - environment variable array passed to execve().
Output:
    Streams file content to stdout to verify system-call based execution.
--- */
static void test_cat_frankenstein(char *envp[])
{
    Job job;
    set_test_job(&job);

    job.num_stages = 1;
    job.pipeline[0].argc = 1;
    job.pipeline[0].argv[0] = "cat";
    job.pipeline[0].argv[1] = NULL;

    job.infile_path = "frankenstein.txt";

    printf("=== Test: cat < frankenstein.txt ===\n");
    print_job(&job);

    run_job(&job, envp);
    printf("=== End of test ===\n\n");
}

/* ---
Function Name: test_less_frankenstein
Purpose:
    Tests reading large file through interactive pager (less frankenstein.txt).
Input:
    envp - environment variable array passed to execve().
Output:
    Streams file content to stdout to verify system-call based execution.
--- */
static void test_less_frankenstein(char *envp[])
{
    Job job;
    set_test_job(&job);

    job.num_stages = 1;
    job.pipeline[0].argc = 1;
    job.pipeline[0].argv[0] = "less";
    job.pipeline[0].argv[1] = NULL;

    job.infile_path = "frankenstein.txt";

    printf("=== Test: less frankenstein.txt ===\n");
    print_job(&job);

    run_job(&job, envp);
    printf("=== End of test ===\n\n");
}
