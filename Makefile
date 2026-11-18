CC = gcc
CFLAGS = -std=c99 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -g -Iinclude
SRC = src/minish.c src/parser.c src/builtins.c src/executor.c src/redir.c src/pipe.c src/signal.c src/history.c
OBJ = $(SRC:.c=.o)
TARGET = minish

TEST_SRC = tests/test_parser.c tests/test_builtins.c tests/test_executor.c tests/test_redir.c tests/test_pipe.c tests/test_signal.c
TEST_OBJ = $(TEST_SRC:.c=.o)
TEST_TARGETS = $(TEST_SRC:tests/test_%.c=test_%)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Test targets
test_%: tests/test_%.o $(filter-out src/minish.o,$(OBJ))
	$(CC) $(CFLAGS) -o $@ $^

test: $(TEST_TARGETS)
	@for t in $(TEST_TARGETS); do echo "Running $$t..."; ./$$t || exit 1; done

clean:
	rm -f $(OBJ) $(TARGET) $(TEST_OBJ) $(TEST_TARGETS)

.PHONY: all test clean