CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude
TARGET = sentinel
SRCS = main.c src/canary.c src/fanotify.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)