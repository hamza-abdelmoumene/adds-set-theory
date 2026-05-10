CC      = gcc
CFLAGS  = -Wall -Wextra -Iinclude
LDFLAGS = -lm
SRC     = $(wildcard src/*.c)
TARGET  = adds-set-theory

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
