#include "executor.h"
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int execute_external(cmd_t *cmd) {
    if (!cmd || !cmd->argv || !cmd->argv[0]) return -1;

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        /* Child process */
        setup_redirection(cmd);
        execvp(cmd->argv[0], cmd->argv);
        perror(cmd->argv[0]);
        _exit(127);
    }

    /* Parent process */
    int status;
    pid_t waited;
    do {
        waited = waitpid(pid, &status, 0);
    } while (waited == -1 && errno == EINTR);

    if (waited == -1) {
        perror("waitpid");
        return -1;
    }

    return status;
}