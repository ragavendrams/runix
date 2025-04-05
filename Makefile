CC := gcc
CFLAGS := -Wextra -Wall -Wpedantic -Werror -O3

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

TARGET = runix

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET) -v -r ${PWD}/ubuntu-fs run "/bin/bash"

clean:
	rm -rf $(TARGET) $(OBJS)