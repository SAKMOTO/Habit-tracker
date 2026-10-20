CC = gcc
CFLAGS = -Wall -Wextra -Werror
TARGET = habittracker

all: $(TARGET)

$(TARGET): habit_tracker.c
	$(CC) $(CFLAGS) -o $(TARGET) habit_tracker.c

clean:
	rm -f $(TARGET)

.PHONY: all clean
