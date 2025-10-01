#ifndef MY_SH_H
#define MY_SH_H

#include "jobs.h"

/*CONSTANTS*/
#define STD_IN 0
#define STD_OUT 1
#define STD_ERR 2

/*FUNCTION DECLARATIONS*/
void get_command(Command *command);
void run_command(Command *command);


#endif
