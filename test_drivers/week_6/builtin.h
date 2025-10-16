#ifndef BUILTIN_H
#define BUILTIN_H

extern int last_exit_status;

void handle_cd(char **argv);
void handle_exit(char **argv);
void handle_export(char **argv);
void expand_variables(char **argv);

#endif
