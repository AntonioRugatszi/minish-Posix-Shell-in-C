#ifndef PIPE_H
#define PIPE_H

#include "redir.h"

/**
 * Execute a piped command (left | right).
 * @param left Left command
 * @param right Right command
 * @return Exit status of right command
 */
int execute_pipe(cmd_t *left, cmd_t *right);

/**
 * Check if argv contains a pipe operator.
 * @param argv Argument vector
 * @return Index of '|' token, or -1 if not found
 */
int find_pipe(char **argv);

/**
 * Split argv at pipe into two cmd_t structures.
 * @param argv Argument vector (will be modified)
 * @param pipe_idx Index of '|' token
 * @param left Output: left command
 * @param right Output: right command
 * @return 0 on success, -1 on error
 */
int split_pipe(char **argv, int pipe_idx, cmd_t **left, cmd_t **right);

#endif /* PIPE_H */