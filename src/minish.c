#define _GNU_SOURCE
#include "parser.h"
#include "builtins.h"
#include "history.h"
#include "executor.h"
#include "pipe.h"
#include "minish_signal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define PROMPT "minish> "

static void trim_newline(char *line) {
    if (!line) return;
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n') {
        line[len - 1] = '\0';
    }
}

static int is_whitespace_only(const char *line) {
    if (!line) return 1;
    while (*line) {
        if (!isspace((unsigned char)*line)) return 0;
        line++;
    }
    return 1;
}

int main(void) {
    setup_signals();

    char *line = NULL;
    size_t linecap = 0;

    while (1) {
        g_sigint_received = 0;

        /* Print prompt */
        fprintf(stdout, "%s", PROMPT);
        fflush(stdout);

        /* Read line */
        ssize_t linelen = getline(&line, &linecap, stdin);
        if (linelen == -1) {
            /* EOF (Ctrl+D) */
            fprintf(stdout, "\n");
            break;
        }

        trim_newline(line);

        /* Skip empty/whitespace lines */
        if (is_whitespace_only(line)) {
            continue;
        }

        /* Add to history */
        history_add(line);

        /* Parse line */
        int argc;
        char **argv = parse_line(line, &argc);
        if (!argv || argc == 0) {
            free_argv(argv);
            continue;
        }

        /* Check for pipe */
        int pipe_idx = find_pipe(argv);
        if (pipe_idx >= 0) {
            /* Handle pipe */
            cmd_t *left = NULL;
            cmd_t *right = NULL;
            if (split_pipe(argv, pipe_idx, &left, &right) == 0) {
                execute_pipe(left, right);
                free_cmd(left);
                free_cmd(right);
            } else {
                fprintf(stderr, "minish: pipe syntax error\n");
            }
            free_argv(argv);
            continue;
        }

        /* No pipe - parse for redirections */
        cmd_t *cmd = parse_command(argv);
        if (!cmd) {
            free_argv(argv);
            continue;
        }

        /* Check for built-in */
        if (is_builtin(cmd->argv[0])) {
            int result = run_builtin(cmd->argv, argc);
            if (result == 42) {  /* exit */
                free_cmd(cmd);
                free_argv(argv);
                break;
            }
        } else {
            /* External command */
            execute_external(cmd);
        }

        free_cmd(cmd);
        free_argv(argv);
    }

    free(line);
    return 0;
}