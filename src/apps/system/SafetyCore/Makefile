CC=gcc
CFLAGS=-Wall -O2
TARGET=safety_core

all: $(TARGET)

$(TARGET): safety_core.c
	$(CC) $(CFLAGS) safety_core.c -o $(TARGET)

clean:
	rm -f $(TARGET)
