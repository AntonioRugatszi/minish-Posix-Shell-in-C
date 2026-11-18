#ifndef BUILTINS_H
#define BUILTINS_H

#include <stddef.h>

/**
 * Check if a command is a built-in.
 * @param cmd Command name
 * @return 1 if built-in, 0 otherwise
 */
int is_builtin(const char *cmd);

/**
 * Execute a built-in command.
 * @param argv Argument vector (argv[0] is command name)
 * @param argc Number of arguments
 * @return 0 on success, -1 on error, 42 for exit command
 */
int run_builtin(char **argv, int argc);

#endif /* BUILTINS_H */