#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INITIAL_ARGV_CAPACITY 8

char **parse_line(const char *line, int *argc) {
    if (!line || !argc) {
        if (argc) *argc = 0;
        return NULL;
    }

    size_t capacity = INITIAL_ARGV_CAPACITY;
    char **argv = calloc(capacity + 1, sizeof(char *));
    if (!argv) {
        *argc = 0;
        return NULL;
    }

    int count = 0;
    const char *p = line;

    while (*p) {
        /* Skip whitespace */
        while (*p && isspace((unsigned char)*p)) {
            p++;
        }
        if (!*p) break;

        /* Find end of token */
        const char *start = p;
        while (*p && !isspace((unsigned char)*p)) {
            p++;
        }

        size_t len = p - start;
        char *token = malloc(len + 1);
        if (!token) {
            /* Cleanup on allocation failure */
            for (int i = 0; i < count; i++) {
                free(argv[i]);
            }
            free(argv);
            *argc = 0;
            return NULL;
        }
        memcpy(token, start, len);
        token[len] = '\0';

        argv[count++] = token;

        /* Resize if needed */
        if (count >= (int)capacity) {
            capacity *= 2;
            char **new_argv = realloc(argv, (capacity + 1) * sizeof(char *));
            if (!new_argv) {
                for (int i = 0; i < count; i++) {
                    free(argv[i]);
                }
                free(argv);
                *argc = 0;
                return NULL;
            }
            argv = new_argv;
        }
    }

    argv[count] = NULL;
    *argc = count;
    return argv;
}

void free_argv(char **argv) {
    if (!argv) return;
    for (int i = 0; argv[i]; i++) {
        free(argv[i]);
    }
    free(argv);
}