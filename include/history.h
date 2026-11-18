#ifndef HISTORY_H
#define HISTORY_H

#include <stddef.h>

/**
 * Add a command line to history.
 * @param line Command line to store (will be copied)
 */
void history_add(const char *line);

/**
 * Print history with line numbers.
 */
void history_print(void);

#endif /* HISTORY_H */