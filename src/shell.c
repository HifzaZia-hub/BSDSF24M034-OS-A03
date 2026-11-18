#include "shell.h"

char* cmd_history[HISTORY_SIZE];
int history_count = 0;

Variable *var_list = NULL;

char* read_cmd(char* prompt, FILE* fp) {
    printf("%s", prompt);
    char* cmdline = (char*) malloc(sizeof(char) * MAX_LEN);
    int c, pos = 0;

    while ((c = getc(fp)) != EOF) {
        if (c == '\n') break;
        cmdline[pos++] = c;
    }

    if (c == EOF && pos == 0) {
        free(cmdline);
        return NULL;
    }

    cmdline[pos] = '\0';
    return cmdline;
}

char** tokenize(char* cmdline) {
    if (cmdline == NULL || cmdline[0] == '\0') return NULL;

    char** arglist = (char**)malloc(sizeof(char*) * (MAXARGS + 1));
    for (int i = 0; i < MAXARGS + 1; i++) {
        arglist[i] = (char*)malloc(sizeof(char) * ARGLEN);
        bzero(arglist[i], ARGLEN);
    }

    char* cp = cmdline;
    int argnum = 0;

    while (*cp != '\0' && argnum < MAXARGS) {
        while (*cp == ' ' || *cp == '\t') cp++;
        if (*cp == '\0') break;

        int len = 0;
        
        if (*cp == '<' || *cp == '>' || *cp == '|') {
            arglist[argnum][0] = *cp;
            arglist[argnum][1] = '\0';
            cp++;
        } 
        else {
            while (*cp != '\0' && *cp != ' ' && *cp != '\t' && *cp != '<' && *cp != '>' && *cp != '|') {
                arglist[argnum][len++] = *cp++;
            }
            arglist[argnum][len] = '\0';
        }
        argnum++;
    }

    if (argnum == 0) {
        for(int i = 0; i < MAXARGS + 1; i++) free(arglist[i]);
        free(arglist);
        return NULL;
    }

    arglist[argnum] = NULL;
    return arglist;
}

void add_to_history(char* cmdline) {
    if (cmdline == NULL || strlen(cmdline) == 0) return;

    if (history_count >= HISTORY_SIZE) {
        free(cmd_history[0]); 
        for (int i = 0; i < HISTORY_SIZE - 1; i++) {
            cmd_history[i] = cmd_history[i + 1];
        }
        history_count = HISTORY_SIZE - 1;
    }
    
    cmd_history[history_count] = strdup(cmdline);
    history_count++;
}

void set_variable(char* name, char* value) {
    Variable *curr = var_list;
    while (curr != NULL) {
        if (strcmp(curr->name, name) == 0) {
            free(curr->value);
            curr->value = strdup(value);
            return;
        }
        curr = curr->next;
    }
    Variable *new_var = (Variable*)malloc(sizeof(Variable));
    new_var->name = strdup(name);
    new_var->value = strdup(value);
    new_var->next = var_list;
    var_list = new_var;
}

char* get_variable(char* name) {
    Variable *curr = var_list;
    while (curr != NULL) {
        if (strcmp(curr->name, name) == 0) return curr->value;
        curr = curr->next;
    }
    return NULL;
}

void expand_variables(char** arglist) {
    for (int i = 0; arglist[i] != NULL; i++) {
        if (arglist[i][0] == '$') {
            char* var_name = arglist[i] + 1;
            char* val = get_variable(var_name);
            if (val != NULL) {
                free(arglist[i]);
                arglist[i] = strdup(val);
            }
        }
    }
}

int handle_builtin(char** arglist) {
    if (strcmp(arglist[0], "exit") == 0) {
        exit(0); 
    }
    else if (strcmp(arglist[0], "cd") == 0) {
        if (arglist[1] == NULL) {
            chdir(getenv("HOME")); 
        } else {
            if (chdir(arglist[1]) != 0) perror("cd failed"); 
        }
        return 1; 
    }
    else if (strcmp(arglist[0], "help") == 0) {
        printf("Built-in commands:\n");
        printf("  cd <dir>  - Change directory\n");
        printf("  exit      - Exit the shell\n");
        printf("  help      - Show this help message\n");
        printf("  jobs      - List background jobs\n");
        printf("  history   - Show command history\n");
        printf("  set       - Show all variables\n");
        return 1; 
    }
    else if (strcmp(arglist[0], "jobs") == 0) {
        printf("Job control not yet fully implemented.\n");
        return 1; 
    }
    else if (strcmp(arglist[0], "history") == 0) {
        for (int i = 0; i < history_count; i++) {
            printf("%d: %s\n", i + 1, cmd_history[i]);
        }
        return 1;
    }
    else if (strcmp(arglist[0], "set") == 0) {
        Variable *curr = var_list;
        while (curr != NULL) {
            printf("%s=%s\n", curr->name, curr->value);
            curr = curr->next;
        }
        return 1;
    }
    return 0;
}
