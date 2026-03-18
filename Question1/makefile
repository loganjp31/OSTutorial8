# Makefile for Question1 (process tree program)

CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -O2
TARGET := main.exe
SRCS := main.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
