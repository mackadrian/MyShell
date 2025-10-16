#include "runjob.h"
#include "mystring.h"
#include "myheap.h"
#include "errors.h"
#include "jobs.h"

#include <stdio.h>
#include <string.h>

// FUNCTION DECLARATIONS
void print_job(Job *job);
void set_test_job(Job *job);
void test_normal_command(char* envp[]);
void test_pipeline_command(char* envp[]);
void test_background_command(char* envp[]);
void test_redirection_command(char* envp[]);

// MAIN
int main(int argc, char* argv[], char* envp[])
{
    test_normal_command(envp);
    test_pipeline_command(envp);
    test_background_command(envp);

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

void test_normal_command(char* envp[])
{
    Job job;
    set_test_job(&job);

    job.num_stages = 1;
    job.pipeline[0].argc = 2;
    job.pipeline[0].argv[0] = "ls";
    job.pipeline[0].argv[1] = "-l";
    job.pipeline[0].argv[2] = NULL;

    printf("Test: ls -l\n");
    print_job(&job);

    run_job(&job, envp);
}

void test_pipeline_command(char* envp[])
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

    printf("Test: echo | grep | sort\n");
    print_job(&job);

    run_job(&job, envp);
}

void test_background_command(char* envp[])
{
    Job job;
    set_test_job(&job);

    job.num_stages = 1;
    job.background = 1;
    job.pipeline[0].argc = 2;
    job.pipeline[0].argv[0] = "sleep";
    job.pipeline[0].argv[1] = "2";
    job.pipeline[0].argv[2] = NULL;

    printf("Test: sleep 2 &\n");
    print_job(&job);

    run_job(&job, envp);
}

