#include "getjob.h"
#include "mystring.h"
#include "myheap.h"
#include "errors.h"
#include "jobs.h"

#include <stdio.h>

//FUNCTION DECLARATIONS
void print_job(Job *job);
void parse_pipeline(Job *job, char *command_line);
void test1();
void test2();
void test3();
void test4();


int main() {
    test1();
    test2();
    test3();
    test4();
    test5();
    return 0;
}

/* Helper to print a Job structure */
void print_job(Job *job) {
    printf("Number of stages: %d\n", job->num_stages);
    printf("Background flag: %d\n", job->background);

    for (int i = 0; i < job->num_stages; i++) {
        Command *cmd = &job->pipeline[i];
        printf("  Stage %d argc: %d\n", i, cmd->argc);
        for (int j = 0; j < cmd->argc; j++) {
            printf("    argv[%d]: %s\n", j, cmd->argv[j]);
        }
    }
    printf("-------------------------------------------------\n");
}

/* Helper to split a command line by pipes and call parse_stage for each stage */
void parse_pipeline(Job *job, char *command_line) {
    set_job(job);

    /* Check for background execution */
    int len = mystrlen(command_line);
    if (len > 0 && command_line[len - 1] == '&') {
        job->background = 1;
        command_line[len - 1] = '\0';
    }

    int stage_start = 0;
    for (int i = 0;; i++) {
        char c = command_line[i];
        if (c == '|' || c == '\0') {
            command_line[i] = '\0';

            /* Skip leading whitespace */
            while (command_line[stage_start] == ' ' || command_line[stage_start] == '\t')
                stage_start++;

            if (command_line[stage_start] != '\0') {
                parse_stage(&job->pipeline[job->num_stages],
                            &command_line[stage_start],
                            job);
                job->num_stages++;
            }

            if (c == '\0')
                break;

            stage_start = i + 1;
        }
    }
}


void test1() {
    Job job;
    char command[] = "ls -l";
    printf("Test 1: '%s'\n", command);
    parse_pipeline(&job, command);
    print_job(&job);
}

void test2() {
    Job job;
    char command[] = "cat file.txt | grep foo | sort";
    printf("Test 2: '%s'\n", command);
    parse_pipeline(&job, command);
    print_job(&job);
}

void test3() {
    Job job;
    char command[] = "echo hello world &";
    printf("Test 3: '%s'\n", command);
    parse_pipeline(&job, command);
    print_job(&job);
}

void test4() {
    Job job;
    char command[] = "gcc main.c -o main > output.txt";
    printf("Test 4: '%s'\n", command);
    parse_pipeline(&job, command);
    print_job(&job);
}

void test5() {
    Job job;
    char command[] = "sort < unsorted.txt | uniq | wc -l";
    printf("Test 5: '%s'\n", command);
    parse_pipeline(&job, command);
    print_job(&job);
}
