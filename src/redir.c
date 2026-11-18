#include "redir.h"
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

cmd_t *parse_command(char **argv) {
    if (!argv) return NULL;

    cmd_t *cmd = calloc(1, sizeof(cmd_t));
    if (!cmd) return NULL;

    /* Count non-redirection arguments */
    int argc = 0;
    for (int i = 0; argv[i]; i++) {
        if (strcmp(argv[i], "<") == 0 || strcmp(argv[i], ">") == 0 || strcmp(argv[i], ">>") == 0) {
            /* Redirection operator - skip for argc count */
        } else {
            argc++;
        }
    }

    cmd->argv = calloc(argc + 1, sizeof(char *));
    if (!cmd->argv) {
        free(cmd);
        return NULL;
    }

    int arg_idx = 0;
    for (int i = 0; argv[i]; i++) {
        if (strcmp(argv[i], "<") == 0) {
            if (argv[i + 1]) {
                cmd->infile = argv[i + 1];
                i++;  /* Skip filename */
            }
        } else if (strcmp(argv[i], ">") == 0) {
            if (argv[i + 1]) {
                cmd->outfile = argv[i + 1];
                cmd->append = 0;
                i++;
            }
        } else if (strcmp(argv[i], ">>") == 0) {
            if (argv[i + 1]) {
                cmd->outfile = argv[i + 1];
                cmd->append = 1;
                i++;
            }
        } else {
            cmd->argv[arg_idx++] = argv[i];
        }
    }
    cmd->argv[arg_idx] = NULL;

    return cmd;
}

void free_cmd(cmd_t *cmd) {
    if (!cmd) return;
    if (cmd->argv) {
        /* Note: argv strings are owned by the original parse_line, don't free them */
        free(cmd->argv);
    }
    /* Note: infile/outfile point to argv strings, don't free them */
    free(cmd);
}

void setup_redirection(cmd_t *cmd) {
    if (!cmd) return;

    if (cmd->infile) {
        int fd = open(cmd->infile, O_RDONLY);
        if (fd == -1) {
            perror(cmd->infile);
            _exit(1);
        }
        if (dup2(fd, STDIN_FILENO) == -1) {
            perror("dup2 stdin");
            close(fd);
            _exit(1);
        }
        close(fd);
    }

    if (cmd->outfile) {
        int flags = O_WRONLY | O_CREAT | (cmd->append ? O_APPEND : O_TRUNC);
        int fd = open(cmd->outfile, flags, 0644);
        if (fd == -1) {
            perror(cmd->outfile);
            _exit(1);
        }
        if (dup2(fd, STDOUT_FILENO) == -1) {
            perror("dup2 stdout");
            close(fd);
            _exit(1);
        }
        close(fd);
    }
}