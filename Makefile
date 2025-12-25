# Compiler and Flags
CC = gcc
CFLAGS = -Wall -O3
LIBS = -lncurses

# Target executable name
TARGET =  cbonsai-timer

# Source files
SRC = cbonsai-timer.c

# Default rule: Build the program
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LIBS)

# Rule to run the program
run: all
	./$(TARGET)

# Rule to clean up compiled files
clean:
	rm -f $(TARGET)

# Helpful for debugging - tells make these aren't actual files
.PHONY: all run clean
