# Makefile for your project

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -g `pkg-config --cflags gtk+-3.0`

# Source files
SRCS = src/main.c \
       src/cpu.c \
       src/disk.c \
       src/ram_swap.c \
       src/process.c 

# Object files
OBJS = $(SRCS:.c=.o)

# Executable name
TARGET = vtop

# Build the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) `pkg-config --libs gtk+-3.0` -lcairo -lm

# Compile source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up generated files
clean:
	rm -f $(OBJS) $(TARGET)

# Default target
all: $(TARGET)
