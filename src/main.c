#include "shell.h"
#include <readline/readline.h>
#include <readline/history.h>

// --- Pipes function ---
void execute_pipe(char** arglist) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        return;
    }

    int i = 0;
    while (arglist[i] != NULL && strcmp(arglist[i], "|") != 0) {
        i++;
    }
    arglist[i] = NULL; 

    char** left_cmd = arglist;
    char** right_cmd = &arglist[i + 1];

    int cpid1 = fork();
    if (cpid1 == 0) {
        dup2(pipefd[1], STDOUT_FILENO); 
        close(pipefd[0]); 
        close(pipefd[1]);
        execvp(left_cmd[0], left_cmd);
        exit(1);
    }

    int cpid2 = fork();
    if (cpid2 == 0) {
        dup2(pipefd[0], STDIN_FILENO); 
        close(pipefd[1]); 
        close(pipefd[0]);
        execvp(right_cmd[0], right_cmd);
        exit(1);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(cpid1, NULL, 0);
    waitpid(cpid2, NULL, 0);
}

// --- Function to process a single command (handles &, |, builtins, external) ---
void process_command(char* cmdline) {
    int is_background = 0;
    
    // Check if it ends with '&'
    int len = strlen(cmdline);
    while (len > 0 && (cmdline[len-1] == ' ' || cmdline[len-1] == '\t')) {
        len--;
    }
    if (len > 0 && cmdline[len-1] == '&') {
        is_background = 1;
        cmdline[len-1] = '\0'; // Remove the '&' from the string
    }

    if (strlen(cmdline) == 0) return;

    char** arglist = tokenize(cmdline);
    if (arglist == NULL) return;

    if (handle_builtin(arglist) == 0) {
        if (is_background) {
            // Background execution
            int cpid = fork();
            if (cpid == 0) {
                setpgid(0, 0); // Create a new process group for background job
                execvp(arglist[0], arglist);
                perror("Command not found");
                exit(1);
            }
            printf("[Background] PID: %d\n", cpid);
        } else {
            // Foreground execution
            int has_pipe = 0;
            for (int i = 0; arglist[i] != NULL; i++) {
                if (strcmp(arglist[i], "|") == 0) {
                    has_pipe = 1;
                    break;
                }
            }
            
            if (has_pipe) {
                execute_pipe(arglist);
            } else {
                execute(arglist);
            }
        }
    }

    for (int i = 0; arglist[i] != NULL; i++) {
        free(arglist[i]);
    }
    free(arglist);
}

int main() {
    char* cmdline;
    int status;

    rl_bind_key('\t', rl_complete);
    using_history();

    while (1) {
        // Reap Zombies (Background Jobs)
        while (waitpid(-1, &status, WNOHANG) > 0) {
            // Child reaped silently.
        }

        cmdline = readline(PROMPT);
        if (cmdline == NULL) break; // Ctrl+D
        
        if (strlen(cmdline) == 0) {
            free(cmdline);
            continue;
        }

        add_history(cmdline);

        if (cmdline[0] == '!') {
            int n = atoi(&cmdline[1]);
            if (n > 0 && n <= history_count) {
                free(cmdline);
                cmdline = strdup(cmd_history[n - 1]);
                printf("%s\n", cmdline); 
            } else {
                printf("Error: History index out of bounds.\n");
                free(cmdline);
                continue; 
            }
        }

        add_to_history(cmdline);

        // Handle Semicolon Chaining (;)
        char* saveptr;
        char* cmd = strtok_r(cmdline, ";", &saveptr);
        
        while (cmd != NULL) {
            while (*cmd == ' ' || *cmd == '\t') cmd++;
            if (strlen(cmd) > 0) {
                process_command(cmd);
            }
            cmd = strtok_r(NULL, ";", &saveptr);
        }

        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}
