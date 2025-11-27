# Compiler
CC = gcc-15

# Compiler flags
CFLAGS = -Wall -Wextra -pedantic -std=c11 -O2

# Source files
SRCS = main.c terminal.c input.c output.c

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
