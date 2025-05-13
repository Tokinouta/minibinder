# Makefile for lib/minibinder.c

CC = gcc
CFLAGS = -Wall -Wextra -O2
SRC = minibinder.c
OBJ = $(SRC:.c=.o)
TARGET = libminibinder.so

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -shared -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean