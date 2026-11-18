#ifndef REDIR_H
#define REDIR_H

#include <stddef.h>

/**
 * Parsed command structure with redirection info.
 */
typedef struct {
    char **argv;      /* Command and arguments (NULL-terminated) */
    char *infile;     /* Input redirection file (<) */
    char *outfile;    /* Output redirection file (>) */
    int append;       /* 1 for >>, 0 for > */
} cmd_t;

/**
 * Parse argv array for redirection operators.
 * Modifies argv in place (removes redirection tokens).
 * @param argv Argument vector from parse_line
 * @return Allocated cmd_t structure (must be freed with free_cmd)
 */
cmd_t *parse_command(char **argv);

/**
 * Free a cmd_t structure.
 * @param cmd Command structure to free
 */
void free_cmd(cmd_t *cmd);

/**
 * Set up redirection in child process before exec.
 * @param cmd Command with redirection info
 * On error, calls perror and _exit(1)
 */
void setup_redirection(cmd_t *cmd);

#endif /* REDIR_H */