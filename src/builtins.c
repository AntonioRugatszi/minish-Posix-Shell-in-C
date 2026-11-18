#include "builtins.h"
#include "history.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

static const char *builtin_names[] = {"cd", "exit", "history", NULL};

int is_builtin(const char *cmd) {
    if (!cmd) return 0;
    for (int i = 0; builtin_names[i]; i++) {
        if (strcmp(cmd, builtin_names[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int run_builtin(char **argv, int argc) {
    if (!argv || argc == 0) return -1;

    if (strcmp(argv[0], "cd") == 0) {
        const char *path = argv[1] ? argv[1] : getenv("HOME");
        if (!path) {
            fprintf(stderr, "cd: HOME not set\n");
            return -1;
        }
        if (chdir(path) != 0) {
            perror("cd");
            return -1;
        }
        return 0;
    }

    if (strcmp(argv[0], "exit") == 0) {
        return 42;  /* Special exit code */
    }

    if (strcmp(argv[0], "history") == 0) {
        history_print();
        return 0;
    }

    return -1;
}