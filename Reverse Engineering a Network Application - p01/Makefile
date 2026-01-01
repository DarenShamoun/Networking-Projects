CC = gcc
CFLAGS = -Wall -Wextra -g -std=c11

TARGET = client

all: $(TARGET)

client: client.c io_helpers.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) $(TARGET)
