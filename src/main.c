#include "shell.h"
#include <readline/readline.h>
#include <readline/history.h>

int main() {
    char* cmdline;
    char** arglist;

    // Enable default Readline tab completion (for files/folders)
    rl_bind_key('\t', rl_complete);
    using_history();

    while ((cmdline = readline(PROMPT)) != NULL) {
        
        // If the user presses Enter without typing anything, skip
        if (strlen(cmdline) == 0) {
            free(cmdline);
            continue;
        }

        // Add the command to Readline's internal history (for Up/Down arrows)
        add_history(cmdline);

        // --- Keep the !n re-execution logic ---
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

        // Add to our custom history array (for the "history" command)
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
