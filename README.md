# minish - A Minimal POSIX Shell in C

A simple command-line interpreter (shell) written in C that implements basic shell functionality including built-in commands, external command execution, I/O redirection, piping, and signal handling.

## Features

- **REPL (Read-Eval-Print Loop)**: Custom prompt (`minish>`), reads input using `getline()`
- **Command Parsing**: Tokenizes input by whitespace
- **Built-in Commands**:
  - `cd [path]` - Change directory (defaults to `$HOME`)
  - `exit` - Exit the shell
  - `history` - Display command history with line numbers
- **External Command Execution**: Uses `fork()`/`execvp()`/`waitpid()`
- **I/O Redirection**:
  - `< file` - Input redirection
  - `> file` - Output redirection (truncate)
  - `>> file` - Output redirection (append)
- **Piping**: `cmd1 | cmd2` - Pipe output of first command to input of second
- **Signal Handling**: Catches `SIGINT` (Ctrl+C) to cancel foreground commands without killing the shell

## Building

```bash
make
```

This produces the `minish` executable.

## Running

```bash
./minish
```

## Usage Examples

```bash
# Basic commands
minish> echo hello world
minish> ls -la

# Built-in commands
minish> cd /tmp
minish> pwd
minish> history
minish> exit

# I/O Redirection
minish> echo hello > output.txt
minish> cat < output.txt
minish> echo world >> output.txt

# Piping
minish> ls | grep minish
minish> cat file.txt | grep "pattern" | wc -l

# Signal handling
minish> sleep 10
^C
minish>  # Ctrl+C cancels the sleep command, shell continues
```

## Architecture

The shell is organized into modular components:

- `src/minish.c` - Main REPL loop and command dispatch
- `src/parser.c` - Command line tokenization
- `src/builtins.c` - Built-in command implementations
- `src/history.c` - Command history (circular buffer, 100 entries)
- `src/executor.c` - External command execution (fork/exec/wait)
- `src/redir.c` - I/O redirection parsing and setup
- `src/pipe.c` - Pipe handling (fork twice, wire file descriptors)
- `src/signal.c` - Signal handling (SIGINT, SIGTSTP, SIGQUIT)

## Requirements

- C99 compiler (GCC/Clang)
- POSIX-compliant system (Linux, macOS, BSD, etc.)

## Limitations

- Single pipe only (`cmd1 | cmd2`, not `cmd1 | cmd2 | cmd3`)
- No support for quotes or escape characters
- No job control (bg/fg)
- No command substitution (`$(...)` or `` `...` ``)
- No environment variable expansion (`$VAR`)
- Signal handling for foreground process cancellation is basic

## License

This is a learning project.