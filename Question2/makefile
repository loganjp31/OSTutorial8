# Makefile for Question2

CC := gcc
CFLAGS := -std=c99 -Wall -Wextra -O2
TARGET := Question2.exe
SRCS := Question2.c queue.c

MOCKSRC := mock.c
MOCKS :=  systemd.exe bash.exe vim.exe emacs.exe chrome.exe gedit.exe eclipse.exe clang.exe

.PHONY: all clean run

all: $(TARGET) compileMocks

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

run: $(TARGET) compileMocks
	./$(TARGET)

compileMocks: $(MOCKS)

$(MOCKS): %: $(MOCKSRC)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET) $(MOCKS)

