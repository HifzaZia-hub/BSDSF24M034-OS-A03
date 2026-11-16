#include "shell.h"

int main() {
    char* cmdline;
    char** arglist;

    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {
        if ((arglist = tokenize(cmdline)) != NULL) {
            
            // NEW: Check if it's a built-in command first
            // handle_builtin returns 1 if it handled it, 0 if it's not a built-in.
            if (handle_builtin(arglist) == 0) {
                execute(arglist); // Only fork/exec if it's NOT a built-in
            }

            // Free the memory allocated by tokenize()
            for (int i = 0; arglist[i] != NULL; i++) {
                free(arglist[i]);
            }
            free(arglist);
        }
        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}
