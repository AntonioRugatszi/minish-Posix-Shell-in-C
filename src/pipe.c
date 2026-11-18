#include "pipe.h"
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

int find_pipe(char **argv) {
    if (!argv) return -1;
    for (int i = 0; argv[i]; i++) {
        if (strcmp(argv[i], "|") == 0) {
            return i;
        }
    }
    return -1;
}

int split_pipe(char **argv, int pipe_idx, cmd_t **left, cmd_t **right) {
    if (!argv || pipe_idx < 0 || !left || !right) return -1;

    /* Create left argv (up to pipe) */
    int left_argc = 0;
    for (int i = 0; i < pipe_idx; i++) {
        if (strcmp(argv[i], "<") == 0 || strcmp(argv[i], ">") == 0 || strcmp(argv[i], ">>") == 0) {
            i++;  /* Skip redirection filename */
        } else {
            left_argc++;
        }
    }

    /* Create right argv (after pipe) */
    int right_argc = 0;
    for (int i = pipe_idx + 1; argv[i]; i++) {
        if (strcmp(argv[i], "<") == 0 || strcmp(argv[i], ">") == 0 || strcmp(argv[i], ">>") == 0) {
            i++;
        } else {
            right_argc++;
        }
    }

    char **left_argv = calloc(left_argc + 1, sizeof(char *));
    char **right_argv = calloc(right_argc + 1, sizeof(char *));
    if (!left_argv || !right_argv) {
        free(left_argv);
        free(right_argv);
        return -1;
    }

    int lidx = 0, ridx = 0;
    for (int i = 0; i < pipe_idx; i++) {
        if (strcmp(argv[i], "<") == 0 || strcmp(argv[i], ">") == 0 || strcmp(argv[i], ">>") == 0) {
            left_argv[lidx++] = argv[i];
            if (argv[i + 1]) {
                left_argv[lidx++] = argv[i + 1];
                i++;
            }
        } else {
            left_argv[lidx++] = argv[i];
        }
    }
    left_argv[lidx] = NULL;

    for (int i = pipe_idx + 1; argv[i]; i++) {
        if (strcmp(argv[i], "<") == 0 || strcmp(argv[i], ">") == 0 || strcmp(argv[i], ">>") == 0) {
            right_argv[ridx++] = argv[i];
            if (argv[i + 1]) {
                right_argv[ridx++] = argv[i + 1];
                i++;
            }
        } else {
            right_argv[ridx++] = argv[i];
        }
    }
    right_argv[ridx] = NULL;

    *left = parse_command(left_argv);
    *right = parse_command(right_argv);

    free(left_argv);
    free(right_argv);

    if (!*left || !*right) {
        free_cmd(*left);
        free_cmd(*right);
        return -1;
    }

    return 0;
}

int execute_pipe(cmd_t *left, cmd_t *right) {
    if (!left || !right) return -1;

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return -1;
    }

    pid_t left_pid = fork();
    if (left_pid == -1) {
        perror("fork left");
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }

    if (left_pid == 0) {
        /* Left child - writes to pipe */
        close(pipefd[0]);
        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            perror("dup2 pipe stdout");
            _exit(1);
        }
        close(pipefd[1]);
        setup_redirection(left);
        execvp(left->argv[0], left->argv);
        perror(left->argv[0]);
        _exit(127);
    }

    pid_t right_pid = fork();
    if (right_pid == -1) {
        perror("fork right");
        close(pipefd[0]);
        close(pipefd[1]);
        /* Wait for left child to avoid zombie */
        waitpid(left_pid, NULL, 0);
        return -1;
    }

    if (right_pid == 0) {
        /* Right child - reads from pipe */
        close(pipefd[1]);
        if (dup2(pipefd[0], STDIN_FILENO) == -1) {
            perror("dup2 pipe stdin");
            _exit(1);
        }
        close(pipefd[0]);
        setup_redirection(right);
        execvp(right->argv[0], right->argv);
        perror(right->argv[0]);
        _exit(127);
    }

    /* Parent - close pipe ends and wait for both children */
    close(pipefd[0]);
    close(pipefd[1]);

    int left_status, right_status;
    waitpid(left_pid, &left_status, 0);
    waitpid(right_pid, &right_status, 0);

    return right_status;
}