#include <signal.h>
#include "minish_signal.h"
#include <stdio.h>
#include <unistd.h>

volatile int g_sigint_received = 0;

static void sigint_handler(int sig) {
    (void)sig;
    g_sigint_received = 1;
    /* Write newline to clean up prompt after ^C */
    write(STDOUT_FILENO, "\n", 1);
}

void setup_signals(void) {
    /* Use standard C signal() function */
    signal(SIGINT, sigint_handler);

    /* Ignore SIGTSTP (Ctrl+Z) and SIGQUIT (Ctrl+\\) */
    signal(SIGTSTP, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
}