#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

/**
 * Parse a command line string into an argv array.
 * @param line Input command line (will not be modified)
 * @param argc Output: number of arguments
 * @return NULL-terminated array of strings (must be freed with free_argv)
 */
char **parse_line(const char *line, int *argc);

/**
 * Free an argv array returned by parse_line.
 * @param argv Array to free
 */
void free_argv(char **argv);

#endif /* PARSER_H */