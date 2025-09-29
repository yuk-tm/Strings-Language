# Compiler and flags
CC = gcc
CFLAGS = -Wall -g -std=c99

# Source files
SRCS = main.c lexer.c parser.c interpreter.c

# Object files
OBJS = $(SRCS:.c=.o)

# Executable name
TARGET = interpreter

# Default rule
all: $(TARGET)

# Linking the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) -lm

# Compiling source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJS) $(TARGET)

# Rebuild everything
re: clean all

.PHONY: all clean re
