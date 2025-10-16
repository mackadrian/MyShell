#include "getjob.h"
#include "mystring.h"
#include "myheap.h"
#include "errors.h"
#include "jobs.h"

#include <stdio.h>   /* only for testing output */

int main(void)
{
    Job job;

    printf("Enter command for get_job test: ");
    get_job(&job);

    printf("Number of stages: %d\n", job.num_stages);
    printf("Background flag: %d\n", job.background);

    for (int i = 0; i < job.num_stages; i++) {
        Command *cmd = &job.pipeline[i];
        printf("Stage %d argc: %d\n", i, cmd->argc);
        for (int j = 0; j < cmd->argc; j++) {
            printf("argv[%d]: %s\n", j, cmd->argv[j]);
        }
    }



    
    return 0;
}
