#ifndef SIGNAL_H
#define SIGNAL_H

/**
 * Set up signal handlers for the shell.
 * Installs SIGINT handler that sets a global flag.
 */
void setup_signals(void);

/**
 * Global flag set when SIGINT is received.
 * Using int instead of sig_atomic_t for C99 compatibility.
 */
extern volatile int g_sigint_received;

#endif /* SIGNAL_H */