# minish Shell Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a custom POSIX shell in C (minish) with REPL, command parsing, built-in commands (cd, exit, history), external command execution, I/O redirection, piping, and signal handling.

**Architecture:** Single-process shell with fork/exec for external commands. Modular design with separate modules for parsing, built-ins, execution, redirection, piping, and signal handling. Dynamic memory allocation for argument vectors and history buffer.

**Tech Stack:** C99, POSIX system calls (fork, execvp, pipe, dup2, waitpid, signal, chdir, open, close), getline for input reading.

**Spec:** Project specification in system reminder (CLAUDE.md)

## Global Constraints

- C99 standard
- POSIX-compliant system calls only
- No external dependencies beyond standard C library
- Handle EOF (Ctrl+D) gracefully
- SIGINT (Ctrl+C) cancels foreground child without killing shell
- Memory safety: no leaks, proper free() for all malloc/calloc
- Error handling with perror() for system call failures

## Review Focus

1. **Empty input / whitespace only** — shell should re-prompt without error
2. **Command not found** — clear error message, shell continues
3. **Redirection with non-existent file** — perror on open failure, shell continues
4. **Pipe with failed left command** — right command should not hang, proper cleanup
5. **History buffer overflow** — circular buffer or dynamic resize, no crash

---

### Task 1: Project Structure & Build System

**Files:**
- Create: `src/minish.c` (main entry point)
- Create: `src/parser.c` / `include/parser.h`
- Create: `src/builtins.c` / `include/builtins.h`
- Create: `src/executor.c` / `include/executor.h`
- Create: `src/redir.c` / `include/redir.h`
- Create: `src/pipe.c` / `include/pipe.h`
- Create: `src/signal.c` / `include/signal.h`
- Create: `src/history.c` / `include/history.h`
- Create: `Makefile`
- Create: `README.md`

**Interfaces:**
- Consumes: (none - foundation task)
- Produces: Compilation target `minish`, header files with function declarations

- [ ] **Step 1: Create directory structure**
```bash
mkdir -p src include
```

- [ ] **Step 2: Write Makefile**
```makefile
CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -g
SRC = src/minish.c src/parser.c src/builtins.c src/executor.c src/redir.c src/pipe.c src/signal.c src/history.c
OBJ = $(SRC:.c=.o)
TARGET = minish

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
```

- [ ] **Step 3: Create empty header files with include guards**
- [ ] **Step 4: Create empty .c files with includes**
- [ ] **Step 5: Verify build**
Run: `make`
Expected: Builds `minish` executable (does nothing yet)

- [ ] **Step 6: Commit**
```bash
git add . && git commit -m "feat: project structure and build system"
```

---

### Task 2: REPL Loop & Signal Handling

**Files:**
- Modify: `src/minish.c`
- Modify: `src/signal.c` / `include/signal.h`

**Interfaces:**
- Consumes: (none)
- Produces: `void setup_signals(void)` in signal.h, `int main(void)` in minish.c

- [ ] **Step 1: Write signal handler test**
```c
// tests/test_signal.c
#include "signal.h"
#include <signal.h>
#include <assert.h>

void test_sigint_handler() {
    setup_signals();
    // Verify SIGINT handler is installed
    struct sigaction sa;
    sigaction(SIGINT, NULL, &sa);
    assert(sa.sa_handler != SIG_DFL);
    assert(sa.sa_handler != SIG_IGN);
}
```

- [ ] **Step 2: Run test to verify it fails**
Run: `make test_signal && ./test_signal`
Expected: FAIL (functions not defined)

- [ ] **Step 3: Implement `setup_signals(void)` in `src/signal.c`**
- Install SIGINT handler that sets a global flag `g_sigint_received = 1`
- Use `sigaction` with SA_RESTART for system call restart
- Ignore SIGTSTP, SIGQUIT (optional but recommended)

- [ ] **Step 4: Implement REPL skeleton in `src/minish.c`**
- `int main(void)` calls `setup_signals()`
- Loop: print prompt `minish> `, read line with `getline()`
- Handle EOF (NULL return from getline) -> exit loop gracefully
- Trim trailing newline
- Skip empty/whitespace lines
- Free line buffer each iteration

- [ ] **Step 5: Run test to verify signal handler works**
Run: `make && ./minish` (press Ctrl+C, should not exit)
Expected: Shell continues running after Ctrl+C

- [ ] **Step 6: Commit**
```bash
git add . && git commit -m "feat: REPL loop and signal handling"
```

---

### Task 3: Command Parser (Tokenization)

**Files:**
- Modify: `src/parser.c` / `include/parser.h`

**Interfaces:**
- Consumes: (string input from REPL)
- Produces: `char **parse_line(const char *line, int *argc)` - returns malloc'd argv array, sets argc, NULL-terminated. Caller must free with `free_argv(char **argv)`.

- [ ] **Step 1: Write parser test**
```c
// tests/test_parser.c
#include "parser.h"
#include <assert.h>
#include <string.h>

void test_parse_simple() {
    int argc;
    char **argv = parse_line("ls -la", &argc);
    assert(argc == 2);
    assert(strcmp(argv[0], "ls") == 0);
    assert(strcmp(argv[1], "-la") == 0);
    assert(argv[2] == NULL);
    free_argv(argv);
}

void test_parse_whitespace() {
    int argc;
    char **argv = parse_line("  echo   hello  world  ", &argc);
    assert(argc == 3);
    assert(strcmp(argv[0], "echo") == 0);
    assert(strcmp(argv[1], "hello") == 0);
    assert(strcmp(argv[2], "world") == 0);
    free_argv(argv);
}

void test_parse_empty() {
    int argc;
    char **argv = parse_line("", &argc);
    assert(argc == 0);
    assert(argv[0] == NULL);
    free_argv(argv);
}
```

- [ ] **Step 2: Run test to verify it fails**
Run: `make test_parser && ./test_parser`
Expected: FAIL (functions not defined)

- [ ] **Step 3: Implement `parse_line()` in `src/parser.c`**
- Use `strtok_r()` or manual iteration to split on whitespace (space, tab)
- Handle multiple consecutive whitespace
- Allocate argv array dynamically (start with capacity 8, double as needed)
- Return NULL-terminated array
- Implement `free_argv(char **argv)` to free each string and the array

- [ ] **Step 4: Run test to verify it passes**
Run: `make test_parser && ./test_parser`
Expected: PASS

- [ ] **Step 5: Commit**
```bash
git add . && git commit -m "feat: command parser with tokenization"
```

---

### Task 4: Built-in Commands (cd, exit, history)

**Files:**
- Modify: `src/builtins.c` / `include/builtins.h`
- Modify: `src/history.c` / `include/history.h`
- Modify: `src/minish.c` (integrate builtins)

**Interfaces:**
- Consumes: `char **argv`, `int argc` from parser
- Produces: 
  - `int is_builtin(const char *cmd)` - returns 1 if builtin
  - `int run_builtin(char **argv, int argc)` - executes builtin, returns 0 on success, -1 on error, special value for exit
  - `void history_add(const char *line)` - adds to history buffer
  - `void history_print(void)` - prints history with numbers

- [ ] **Step 1: Write builtins test**
```c
// tests/test_builtins.c
#include "builtins.h"
#include "history.h"
#include <assert.h>
#include <unistd.h>

void test_is_builtin() {
    assert(is_builtin("cd") == 1);
    assert(is_builtin("exit") == 1);
    assert(is_builtin("history") == 1);
    assert(is_builtin("ls") == 0);
    assert(is_builtin("") == 0);
}

void test_cd() {
    char *old = getcwd(NULL, 0);
    char *argv[] = {"cd", "/tmp", NULL};
    assert(run_builtin(argv, 2) == 0);
    char *new = getcwd(NULL, 0);
    assert(strcmp(new, "/tmp") == 0);
    chdir(old);
    free(old);
    free(new);
}

void test_cd_error() {
    char *argv[] = {"cd", "/nonexistent", NULL};
    assert(run_builtin(argv, 2) == -1); // perror called
}

void test_exit() {
    char *argv[] = {"exit", NULL};
    assert(run_builtin(argv, 1) == 42); // special exit code
}

void test_history() {
    history_add("ls");
    history_add("cd /tmp");
    history_add("echo hello");
    // history_print() prints to stdout - verify manually or capture
}
```

- [ ] **Step 2: Run test to verify it fails**
Run: `make test_builtins && ./test_builtins`
Expected: FAIL

- [ ] **Step 3: Implement history module in `src/history.c`**
- Fixed-size circular buffer (e.g., 100 entries) or dynamic array
- `history_add()` - stores copy of command line
- `history_print()` - prints numbered list

- [ ] **Step 4: Implement builtins in `src/builtins.c`**
- `is_builtin()` - strcmp against "cd", "exit", "history"
- `run_builtin()`:
  - `cd`: `chdir(argv[1] ? argv[1] : getenv("HOME"))`, perror on failure
  - `exit`: return special value (e.g., 42) to signal shell exit
  - `history`: call `history_print()`, return 0

- [ ] **Step 5: Integrate in `src/minish.c`**
- After parsing, check `is_builtin(argv[0])`
- If builtin, call `run_builtin()`, handle exit code
- If not builtin, proceed to external execution (next task)
- Add command to history before execution (if not empty)

- [ ] **Step 6: Run test to verify it passes**
Run: `make test_builtins && ./test_builtins`
Expected: PASS

- [ ] **Step 7: Manual test REPL**
Run: `make && ./minish`
Test: `cd /tmp`, `pwd`, `history`, `exit`
Expected: All work correctly

- [ ] **Step 8: Commit**
```bash
git add . && git commit -m "feat: built-in commands (cd, exit, history)"
```

---

### Task 5: External Command Execution

**Files:**
- Modify: `src/executor.c` / `include/executor.h`
- Modify: `src/minish.c` (integrate executor)

**Interfaces:**
- Consumes: `char **argv`, `int argc` from parser (non-builtin)
- Produces: `int execute_external(char **argv)` - forks, execs, waits, returns child exit status

- [ ] **Step 1: Write executor test**
```c
// tests/test_executor.c
#include "executor.h"
#include <assert.h>

void test_execute_ls() {
    char *argv[] = {"ls", NULL};
    int status = execute_external(argv);
    assert(WIFEXITED(status));
    assert(WEXITSTATUS(status) == 0);
}

void test_execute_not_found() {
    char *argv[] = {"nonexistentcmd123", NULL};
    int status = execute_external(argv);
    assert(WIFEXITED(status));
    assert(WEXITSTATUS(status) != 0); // typically 127
}
```

- [ ] **Step 2: Run test to verify it fails**
Run: `make test_executor && ./test_executor`
Expected: FAIL

- [ ] **Step 3: Implement `execute_external()` in `src/executor.c`**
- `fork()` - check for error
- Child: `execvp(argv[0], argv)`, on failure `perror("execvp")`, `_exit(127)`
- Parent: `waitpid(pid, &status, 0)`, return status
- Handle EINTR from waitpid (retry on signal)

- [ ] **Step 4: Integrate in `src/minish.c`**
- After builtin check fails, call `execute_external(argv)`
- Print exit status if non-zero (optional)

- [ ] **Step 5: Run test to verify it passes**
Run: `make test_executor && ./test_executor`
Expected: PASS

- [ ] **Step 6: Manual test REPL**
Run: `make && ./minish`
Test: `ls`, `echo hello`, `cat nonexistent`
Expected: External commands execute, errors handled

- [ ] **Step 7: Commit**
```bash
git add . && git commit -m "feat: external command execution with fork/execvp/waitpid"
```

---

### Task 6: Input/Output Redirection

**Files:**
- Modify: `src/redir.c` / `include/redir.h`
- Modify: `src/parser.c` / `include/parser.h` (extend parser)
- Modify: `src/executor.c` (integrate redirection)
- Modify: `src/minish.c` (wire up)

**Interfaces:**
- Consumes: `char **argv` from parser (may contain redirection operators)
- Produces: 
  - `typedef struct { char **argv; char *infile; char *outfile; int append; } cmd_t;`
  - `cmd_t *parse_command(char **argv)` - parses argv for redirections, returns cmd_t
  - `void free_cmd(cmd_t *cmd)`
  - `void setup_redirection(cmd_t *cmd)` - called in child before exec

- [ ] **Step 1: Write redirection test**
```c
// tests/test_redir.c
#include "redir.h"
#include "parser.h"
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

void test_parse_output_redir() {
    int argc;
    char **argv = parse_line("echo hello > out.txt", &argc);
    cmd_t *cmd = parse_command(argv);
    assert(cmd->argv[0] != NULL);
    assert(strcmp(cmd->argv[0], "echo") == 0);
    assert(strcmp(cmd->argv[1], "hello") == 0);
    assert(cmd->argv[2] == NULL);
    assert(strcmp(cmd->outfile, "out.txt") == 0);
    assert(cmd->infile == NULL);
    assert(cmd->append == 0);
    free_cmd(cmd);
    free_argv(argv);
}

void test_parse_input_redir() {
    int argc;
    char **argv = parse_line("cat < in.txt", &argc);
    cmd_t *cmd = parse_command(argv);
    assert(strcmp(cmd->argv[0], "cat") == 0);
    assert(cmd->argv[1] == NULL);
    assert(strcmp(cmd->infile, "in.txt") == 0);
    assert(cmd->outfile == NULL);
    free_cmd(cmd);
    free_argv(argv);
}

void test_parse_append_redir() {
    int argc;
    char **argv = parse_line("echo hi >> log.txt", &argc);
    cmd_t *cmd = parse_command(argv);
    assert(cmd->append == 1);
    free_cmd(cmd);
    free_argv(argv);
}

void test_setup_redirection() {
    // Test in child process context - verify fd duplication
}
```

- [ ] **Step 2: Run test to verify it fails**
Run: `make test_redir && ./test_redir`
Expected: FAIL

- [ ] **Step 3: Extend parser in `src/parser.c`**
- `parse_line()` remains same (tokenizes everything including >, <, >>)
- Add `parse_command()` in redir.c that scans argv for redirection operators
- Remove redirection tokens from argv, populate cmd_t fields
- Support: `> file`, `< file`, `>> file`
- Error if multiple redirections same direction

- [ ] **Step 4: Implement redirection setup in `src/redir.c`**
- `setup_redirection(cmd_t *cmd)`:
  - If infile: `fd = open(infile, O_RDONLY)`, `dup2(fd, STDIN_FILENO)`, `close(fd)`
  - If outfile: `flags = O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC)`, `fd = open(outfile, flags, 0644)`, `dup2(fd, STDOUT_FILENO)`, `close(fd)`
  - Error handling with perror, `_exit(1)` on failure

- [ ] **Step 5: Modify `execute_external()` in `src/executor.c`**
- Accept `cmd_t *cmd` instead of `char **argv`
- In child: call `setup_redirection(cmd)` before `execvp(cmd->argv[0], cmd->argv)`

- [ ] **Step 6: Update `src/minish.c` to use new flow**
- Parse line -> parse_command -> if builtin handle redirection? (builtins don't typically support redirection in simple shells) -> else execute_external

- [ ] **Step 7: Run test to verify it passes**
Run: `make test_redir && ./test_redir`
Expected: PASS

- [ ] **Step 8: Manual test REPL**
Run: `make && ./minish`
Test: `echo hello > /tmp/test.txt`, `cat < /tmp/test.txt`, `echo world >> /tmp/test.txt`
Expected: Redirection works correctly

- [ ] **Step 9: Commit**
```bash
git add . && git commit -m "feat: input/output redirection (<, >, >>)"
```

---

### Task 7: Piping

**Files:**
- Modify: `src/pipe.c` / `include/pipe.h`
- Modify: `src/parser.c` / `include/parser.h` (extend for pipe)
- Modify: `src/executor.c` (add pipe execution)
- Modify: `src/minish.c` (wire up)

**Interfaces:**
- Consumes: Two `cmd_t*` commands (left and right of pipe)
- Produces: `int execute_pipe(cmd_t *left, cmd_t *right)` - creates pipe, forks twice, wires fds, waits for both

- [ ] **Step 1: Write pipe test**
```c
// tests/test_pipe.c
#include "pipe.h"
#include "redir.h"
#include "parser.h"
#include <assert.h>

void test_parse_pipe() {
    int argc;
    char **argv = parse_line("cat file.txt | grep hello", &argc);
    // parse_command should detect | and split into two cmd_t
    // This tests the parser integration
}

void test_execute_pipe() {
    // echo hello | grep hello -> should find match, exit 0
    // echo hello | grep world -> should not match, exit 1
}
```

- [ ] **Step 2: Run test to verify it fails**
Run: `make test_pipe && ./test_pipe`
Expected: FAIL

- [ ] **Step 3: Extend parser for pipe in `src/parser.c` / `src/redir.c`**
- Modify `parse_command()` to detect `|` token
- If found, split argv into left and right parts
- Parse each part separately for redirections
- Return structure indicating pipe with two cmd_t

- [ ] **Step 4: Implement `execute_pipe()` in `src/pipe.c`**
- `pipe(fds)` - check error
- Fork left child:
  - `dup2(fds[1], STDOUT_FILENO)`
  - `close(fds[0])`, `close(fds[1])`
  - `setup_redirection(left)`
  - `execvp(left->argv[0], left->argv)`
- Fork right child:
  - `dup2(fds[0], STDIN_FILENO)`
  - `close(fds[0])`, `close(fds[1])`
  - `setup_redirection(right)`
  - `execvp(right->argv[0], right->argv)`
- Parent: `close(fds[0])`, `close(fds[1])`
- `waitpid()` for both children
- Return right child's exit status

- [ ] **Step 5: Update `src/executor.c` and `src/minish.c`**
- Detect pipe in parsed command
- Call `execute_pipe()` instead of `execute_external()`

- [ ] **Step 6: Run test to verify it passes**
Run: `make test_pipe && ./test_pipe`
Expected: PASS

- [ ] **Step 7: Manual test REPL**
Run: `make && ./minish`
Test: `ls | grep minish`, `echo hello | cat`, `cat /etc/passwd | head -5`
Expected: Piping works correctly

- [ ] **Step 8: Commit**
```bash
git add . && git commit -m "feat: piping support (|)"
```

---

### Task 8: Integration Testing & Edge Cases

**Files:**
- Modify: `src/minish.c` (final polish)
- Create: `tests/integration.sh` or comprehensive test suite

**Interfaces:**
- Consumes: All modules integrated
- Produces: Working minish shell

- [ ] **Step 1: Write integration tests**
```bash
# tests/integration.sh
#!/bin/bash
./minish -c "echo hello" | grep hello
./minish -c "echo hello > /tmp/t.txt && cat /tmp/t.txt" | grep hello
./minish -c "echo -e 'a\nb\nc' | grep b" | grep b
./minish -c "cd /tmp && pwd" | grep /tmp
./minish -c "history"  # after running some commands
```

- [ ] **Step 2: Test edge cases manually**
- Empty input -> re-prompts
- Ctrl+C during command -> cancels command, shell continues
- Ctrl+D -> exits shell
- Command not found -> error, shell continues
- Redirection to read-only file -> perror, shell continues
- Pipe with left command failing -> right command handled correctly
- Very long command line -> no buffer overflow
- History limit -> circular buffer works

- [ ] **Step 3: Memory leak check**
Run: `valgrind --leak-check=full ./minish` (run some commands, exit)
Expected: No definite leaks

- [ ] **Step 4: Commit**
```bash
git add . && git commit -m "test: integration tests and edge case handling"
```

---

### Task 9: Documentation & Cleanup

**Files:**
- Modify: `README.md`
- Modify: `Makefile` (add install target if desired)

**Interfaces:**
- User-facing documentation

- [ ] **Step 1: Write README.md**
- Project description
- Build instructions (`make`)
- Usage examples
- Supported features list
- Known limitations

- [ ] **Step 2: Final code review**
- Consistent error handling
- No compiler warnings (`-Wall -Wextra -Wpedantic`)
- Code style consistency
- All malloc has matching free

- [ ] **Step 3: Final commit**
```bash
git add . && git commit -m "docs: README and final polish"
```