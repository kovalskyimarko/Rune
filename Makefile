# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -pedantic -std=c11 -O2

# Run with memory checks
debug: CFLAGS += -g
debug: rebuild

# Source files
SRCS = main.c terminal.c  input_processkeys.c input.c output.c file.c syntax_parser.c xmemory.c commands.c config.c action.c

OBJS_NO_MAIN = $(filter-out main.o tests/%, $(OBJS))

test: $(OBJS_NO_MAIN)
	@echo "Compiling and running tests..."
	$(CC) $(CFLAGS) -o run_test_actions tests/test_actions.c $(OBJS_NO_MAIN)
	./run_test_actions
	$(CC) $(CFLAGS) -o run_test_buffer tests/test_buffer.c $(OBJS_NO_MAIN)
	./run_test_buffer
	$(CC) $(CFLAGS) -o run_test_rows tests/test_rows.c $(OBJS_NO_MAIN)
	./run_test_rows
	$(CC) $(CFLAGS) -o run_test_search tests/test_search.c $(OBJS_NO_MAIN)
	./run_test_search
	$(CC) $(CFLAGS) -o run_test_commands tests/test_commands.c $(OBJS_NO_MAIN)
	./run_test_commands
	$(CC) $(CFLAGS) -o run_test_syntax tests/test_syntax.c $(OBJS_NO_MAIN)
	./run_test_syntax
	$(CC) $(CFLAGS) -o run_test_config tests/test_config.c $(OBJS_NO_MAIN)
	./run_test_config

# Object files
OBJS = $(SRCS:.c=.o)

# Executable name
TARGET = rune

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Compile source files into objects
%.o: %.c rune.h
	$(CC) $(CFLAGS) -c $< -o $@

# Clean objects and executable
clean:
	rm -f $(OBJS) $(TARGET)

# Rebuild everything
rebuild: clean all
