#include "shell.h"
#include <fcntl.h>

int execute(char* arglist[]) {
    int status;
    int cpid = fork();

    switch (cpid) {
        case -1:
            perror("fork failed");
            exit(1);
            
        case 0: { 
            int i = 0;
            while (arglist[i] != NULL) {
                if (strcmp(arglist[i], ">") == 0) {
                    if (arglist[i+1] == NULL) {
                        fprintf(stderr, "Syntax error: expected filename after >\n");
                        exit(1);
                    }
                    int fd = open(arglist[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd < 0) {
                        perror("open failed");
                        exit(1);
                    }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                    arglist[i] = NULL; 
                } 
                else if (strcmp(arglist[i], "<") == 0) {
                    if (arglist[i+1] == NULL) {
                        fprintf(stderr, "Syntax error: expected filename after <\n");
                        exit(1);
                    }
                    int fd = open(arglist[i+1], O_RDONLY);
                    if (fd < 0) {
                        perror("open failed");
                        exit(1);
                    }
                    dup2(fd, STDIN_FILENO); 
                    close(fd);
                    arglist[i] = NULL; 
                }
                i++;
            }
            execvp(arglist[0], arglist);
            perror("Command not found");
            exit(1);
        }
            
        default: 
            waitpid(cpid, &status, 0);
            return 0;
    }
}
