#include "shell.h"
char* cmd_history[HISTORY_SIZE];
int history_count = 0;

void add_to_history(char* cmdline) {
    // Don't save empty commands
    if (cmdline == NULL || strlen(cmdline) == 0) return;

    // If history is full, shift everything up to make room
    if (history_count >= HISTORY_SIZE) {
        free(cmd_history[0]); // Free the oldest command
        for (int i = 0; i < HISTORY_SIZE - 1; i++) {
            cmd_history[i] = cmd_history[i + 1];
        }
        history_count = HISTORY_SIZE - 1;
    }
    
    // Save a copy of the command
    cmd_history[history_count] = strdup(cmdline);
    history_count++;
}
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
        return NULL; // Handle Ctrl+D
    }
    
    cmdline[pos] = '\0';
    return cmdline;
}

char** tokenize(char* cmdline) {
    // Edge case: empty command line
    if (cmdline == NULL || cmdline[0] == '\0' || cmdline[0] == '\n') {
        return NULL;
    }

    char** arglist = (char**)malloc(sizeof(char*) * (MAXARGS + 1));
    for (int i = 0; i < MAXARGS + 1; i++) {
        arglist[i] = (char*)malloc(sizeof(char) * ARGLEN);
        bzero(arglist[i], ARGLEN);
    }

    char* cp = cmdline;
    char* start;
    int len;
    int argnum = 0;

    while (*cp != '\0' && argnum < MAXARGS) {
        while (*cp == ' ' || *cp == '\t') cp++; // Skip leading whitespace
        
        if (*cp == '\0') break; // Line was only whitespace

        start = cp;
        len = 1;
        while (*++cp != '\0' && !(*cp == ' ' || *cp == '\t')) {
            len++;
        }
        strncpy(arglist[argnum], start, len);
        arglist[argnum][len] = '\0';
        argnum++;
    }

    if (argnum == 0) { // No arguments were parsed
        for(int i = 0; i < MAXARGS + 1; i++) free(arglist[i]);
        free(arglist);
        return NULL;
    }

    arglist[argnum] = NULL;
    return arglist;
}
int handle_builtin(char** arglist) {
    // 1. EXIT
    if (strcmp(arglist[0], "exit") == 0) {
        exit(0); // Terminate the shell gracefully
    }
    
    // 2. CD
    else if (strcmp(arglist[0], "cd") == 0) {
        if (arglist[1] == NULL) {
            chdir(getenv("HOME")); // If just "cd", go to home directory
        } else {
            if (chdir(arglist[1]) != 0) {
                perror("cd failed"); // Handle bad directories
            }
        }
        return 1; // Return 1 to tell main.c "I handled this"
    }
    
    // 3. HELP
    else if (strcmp(arglist[0], "help") == 0) {
        printf("Built-in commands:\n");
        printf("  cd <dir>  - Change directory\n");
        printf("  exit      - Exit the shell\n");
        printf("  help      - Show this help message\n");
        printf("  jobs      - List background jobs\n");
        return 1; 
    }
    
    // 4. JOBS
    else if (strcmp(arglist[0], "jobs") == 0) {
        printf("Job control not yet implemented.\n");
        return 1; 
    }

         // 5. HISTORY
    else if (strcmp(arglist[0], "history") == 0) {
        for (int i = 0; i < history_count; i++) {
            printf("%d: %s\n", i + 1, cmd_history[i]);
        }
        return 1;
    }
    // If we reach here, it's not a built-in command
    return 0;
}
