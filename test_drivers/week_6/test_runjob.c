#include "runjob.h"
#include "mystring.h"
#include "myheap.h"
#include "errors.h"
#include "jobs.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

// FUNCTION DECLARATIONS
void print_job(Job *job);
void set_test_job(Job *job);

void test_normal_command(char *envp[]);
void test_pipeline_command(char *envp[]);
void test_background_command(char *envp[]);
void test_redirection_command(char *envp[]);

void test_invalid_command(char *envp[]);
void test_missing_file_command(char *envp[]);
void test_sigint_handling(char *envp[]);

// MAIN
int main(int argc, char *argv[], char *envp[])
{
    printf("=== Standard RunJob Tests ===\n");
    test_normal_command(envp);
    test_pipeline_command(envp);
    test_background_command(envp);
    test_redirection_command(envp);

    printf("\n=== Error Handling Tests ===\n");
    test_invalid_command(envp);
    test_missing_file_command(envp);

    printf("\n=== Signal Handling Tests ===\n");
    test_sigint_handling(envp);

    
    return 0;
}

// FUNCTION DEFINITIONS

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

void set_test_job(Job *job)
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

void build_cmdline(Job *job, char *cmdline, size_t size)
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

void test_normal_command(char *envp[])
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

void test_pipeline_command(char *envp[])
{
    Job job;
    set_test_job(&job);

    job.pipeline[0].argc = 3;
    job.pipeline[0].argv[0] = "echo";
    job.pipeline[0].argv[1] = "-e";
    job.pipeline[0].argv[2] = "foo\nbar\nbaz";
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

void test_background_command(char *envp[])
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

void test_redirection_command(char *envp[])
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

void test_invalid_command(char *envp[])
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

void test_missing_file_command(char *envp[])
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

void test_sigint_handling(char *envp[])
{
    Job job;
    set_test_job(&job);

    job.num_stages = 1;
    job.pipeline[0].argc = 2;
    job.pipeline[0].argv[0] = "sleep";
    job.pipeline[0].argv[1] = "50";
    job.pipeline[0].argv[2] = NULL;

    char cmdline[128];
    build_cmdline(&job, cmdline, sizeof(cmdline));
    printf("Test: %s (sending SIGINT after 5 second)\n", cmdline);
    print_job(&job);

    pid_t pid = fork();
    if (pid == 0) {
        /* Child runs the job */
        run_job(&job, envp);
        _exit(0);
    } else {
        /* Parent waits a moment then sends SIGINT */
        sleep(1);
        kill(pid, SIGINT);
        printf("Sent SIGINT to child process %d\n", pid);
        waitpid(pid, NULL, 0);
    }

    printf("-------------------------------------------------\n");
}
