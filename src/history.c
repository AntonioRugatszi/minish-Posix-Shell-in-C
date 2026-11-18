#include "history.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define HISTORY_SIZE 100

static char *history_buffer[HISTORY_SIZE] = {0};
static int history_count = 0;
static int history_next = 0;

void history_add(const char *line) {
    if (!line || !*line) return;

    /* Free oldest entry if buffer is full */
    if (history_buffer[history_next]) {
        free(history_buffer[history_next]);
    }

    history_buffer[history_next] = strdup(line);
    if (!history_buffer[history_next]) return;

    history_next = (history_next + 1) % HISTORY_SIZE;
    if (history_count < HISTORY_SIZE) {
        history_count++;
    }
}

void history_print(void) {
    int start = (history_next - history_count + HISTORY_SIZE) % HISTORY_SIZE;
    for (int i = 0; i < history_count; i++) {
        int idx = (start + i) % HISTORY_SIZE;
        if (history_buffer[idx]) {
            printf("%5d  %s\n", i + 1, history_buffer[idx]);
        }
    }
}