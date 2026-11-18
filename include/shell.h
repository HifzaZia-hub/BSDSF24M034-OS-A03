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

// History
extern char* cmd_history[HISTORY_SIZE];
extern int history_count;

// Variables Linked List
typedef struct Variable {
    char *name;
    char *value;
    struct Variable *next;
} Variable;

extern Variable *var_list;

// Function prototypes
char* read_cmd(char* prompt, FILE* fp);
char** tokenize(char* cmdline);
int execute(char** arglist);
int handle_builtin(char** arglist);
void add_to_history(char* cmdline);

// Variable functions
void set_variable(char* name, char* value);
char* get_variable(char* name);
void expand_variables(char** arglist);

#endif // SHELL_H
