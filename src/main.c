#include "shell.h"
#include <readline/readline.h>
#include <readline/history.h>

void execute_pipe(char** arglist) {
    int pipefd[2];
    if (pipe(pipefd) == -1) { perror("pipe failed"); return; }

    int i = 0;
    while (arglist[i] != NULL && strcmp(arglist[i], "|") != 0) i++;
    arglist[i] = NULL; 

    char** left_cmd = arglist;
    char** right_cmd = &arglist[i + 1];

    int cpid1 = fork();
    if (cpid1 == 0) {
        dup2(pipefd[1], STDOUT_FILENO); close(pipefd[0]); close(pipefd[1]);
        execvp(left_cmd[0], left_cmd); exit(1);
    }
    int cpid2 = fork();
    if (cpid2 == 0) {
        dup2(pipefd[0], STDIN_FILENO); close(pipefd[1]); close(pipefd[0]);
        execvp(right_cmd[0], right_cmd); exit(1);
    }
    close(pipefd[0]); close(pipefd[1]);
    waitpid(cpid1, NULL, 0); waitpid(cpid2, NULL, 0);
}

void process_command(char* cmdline) {
    int is_background = 0;
    int len = strlen(cmdline);
    while (len > 0 && (cmdline[len-1] == ' ' || cmdline[len-1] == '\t')) len--;
    if (len > 0 && cmdline[len-1] == '&') { is_background = 1; cmdline[len-1] = '\0'; }
    if (strlen(cmdline) == 0) return;

    // Check for variable assignment (e.g. MSG=Hello)
    char* eq = strchr(cmdline, '=');
    if (eq != NULL) {
        *eq = '\0'; 
        char* name = cmdline;
        char* value = eq + 1;
        int valid = 1;
        for (int i = 0; name[i] != '\0'; i++) {
            if (name[i] == ' ' || name[i] == '\t') { valid = 0; break; }
        }
        if (valid) {
            set_variable(name, value);
            return;
        }
        *eq = '='; 
    }

    char** arglist = tokenize(cmdline);
    if (arglist == NULL) return;

    expand_variables(arglist);

    if (handle_builtin(arglist) == 0) {
        if (is_background) {
            int cpid = fork();
            if (cpid == 0) { setpgid(0, 0); execvp(arglist[0], arglist); perror("Command not found"); exit(1); }
            printf("[Background] PID: %d\n", cpid);
        } else {
            int has_pipe = 0;
            for (int i = 0; arglist[i] != NULL; i++) {
                if (strcmp(arglist[i], "|") == 0) { has_pipe = 1; break; }
            }
            if (has_pipe) execute_pipe(arglist);
            else execute(arglist);
        }
    }
    for (int i = 0; arglist[i] != NULL; i++) free(arglist[i]);
    free(arglist);
}

int execute_and_get_status(char* cmdline) {
    char** arglist = tokenize(cmdline);
    if (arglist == NULL) return 1;
    int status = 1;
    if (handle_builtin(arglist) == 0) {
        int cpid = fork();
        if (cpid == 0) { execvp(arglist[0], arglist); exit(1); } 
        else {
            waitpid(cpid, &status, 0);
            if (WIFEXITED(status)) status = WEXITSTATUS(status);
        }
    }
    for (int i = 0; arglist[i] != NULL; i++) free(arglist[i]);
    free(arglist);
    return status;
}

void handle_if_block(char* first_line) {
    char* then_block[10];
    char* else_block[10];
    int then_count = 0, else_count = 0;
    int in_else = 0;
    char buffer[MAX_LEN];

    while (1) {
        printf("> "); fflush(stdout);
        if (fgets(buffer, MAX_LEN, stdin) == NULL) break;
        buffer[strcspn(buffer, "\n")] = 0;
        if (strlen(buffer) == 0) continue;

        char keyword[20];
        sscanf(buffer, "%s", keyword);

        if (strcmp(keyword, "fi") == 0) break;
        if (strcmp(keyword, "then") == 0) continue;
        if (strcmp(keyword, "else") == 0) { in_else = 1; continue; }

        if (in_else) else_block[else_count++] = strdup(buffer);
        else then_block[then_count++] = strdup(buffer);
    }

    char* if_cmd = first_line;
    while (*if_cmd == ' ' || *if_cmd == '\t') if_cmd++; 
    if_cmd += 2; 
    while (*if_cmd == ' ' || *if_cmd == '\t') if_cmd++; 

    int status = execute_and_get_status(if_cmd);

    if (status == 0) {
        for (int i = 0; i < then_count; i++) { process_command(then_block[i]); free(then_block[i]); }
    } else {
        for (int i = 0; i < else_count; i++) { process_command(else_block[i]); free(else_block[i]); }
    }
}

int main() {
    char* cmdline;
    int status;

    rl_bind_key('\t', rl_complete);
    using_history();

    while (1) {
        while (waitpid(-1, &status, WNOHANG) > 0) {}

        cmdline = readline(PROMPT);
        if (cmdline == NULL) break;
        if (strlen(cmdline) == 0) { free(cmdline); continue; }

        add_history(cmdline);

        if (cmdline[0] == '!') {
            int n = atoi(&cmdline[1]);
            if (n > 0 && n <= history_count) {
                free(cmdline); cmdline = strdup(cmd_history[n - 1]);
                printf("%s\n", cmdline); 
            } else {
                printf("Error: History index out of bounds.\n"); free(cmdline); continue; 
            }
        }
        add_to_history(cmdline);

        char* temp = cmdline;
        while (*temp == ' ' || *temp == '\t') temp++; 
        
        if (strncmp(temp, "if ", 3) == 0) {
            handle_if_block(cmdline);
        } else {
            char* saveptr;
            char* cmd = strtok_r(cmdline, ";", &saveptr);
            while (cmd != NULL) {
                while (*cmd == ' ' || *cmd == '\t') cmd++;
                if (strlen(cmd) > 0) process_command(cmd);
                cmd = strtok_r(NULL, ";", &saveptr);
            }
        }
        free(cmdline);
    }
    printf("\nShell exited.\n");
    return 0;
}
