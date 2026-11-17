#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>

#define MAX_LEN 512
#define MAXARGS 30
#define ARGLEN 64
#define PROMPT "FCIT> "
#define HISTORY_SIZE 20

// Global history variables
extern char* cmd_history[HISTORY_SIZE];
extern int history_count;

// Function prototypes
char* read_cmd(char* prompt, FILE* fp);
char** tokenize(char* cmdline);
int execute(char** arglist);
int handle_builtin(char** arglist);
void add_to_history(char* cmdline);

#endif // SHELL_H
