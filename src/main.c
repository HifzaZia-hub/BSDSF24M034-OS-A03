#include "shell.h"

int main() {
    char* cmdline;
    char** arglist;

    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {
        
        // --- NEW: Handle !n (Re-execution) ---
        if (cmdline[0] == '!') {
            int n = atoi(&cmdline[1]); // Convert the number after ! to an int
            if (n > 0 && n <= history_count) {
                // Get the command from history and replace cmdline
                free(cmdline);
                cmdline = strdup(cmd_history[n - 1]);
                printf("%s\n", cmdline); // Print the command so user sees it
            } else {
                printf("Error: History index out of bounds.\n");
                free(cmdline);
                continue; // Skip execution, go back to prompt
            }
        }

        // Add the (possibly updated) command to history
        add_to_history(cmdline);

        if ((arglist = tokenize(cmdline)) != NULL) {
            if (handle_builtin(arglist) == 0) {
                execute(arglist);
            }

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
