#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "redir.h"

/**
 * Execute an external command (non-builtin).
 * @param cmd Parsed command structure
 * @return Exit status from waitpid (use WIFEXITED/WEXITSTATUS macros)
 */
int execute_external(cmd_t *cmd);

#endif /* EXECUTOR_H */