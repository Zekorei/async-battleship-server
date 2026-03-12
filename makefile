CC = gcc
CFLAGS = -Iinclude -Werror -Wall -std=c99
BUILD_DIR = build

TARGET = server
SOURCES = $(wildcard src/*.c)
OBJECTS = $(SOURCES:src/%.c=build/%.o)

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CC) $^ -lm -o $@

build/%.o : src/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean :
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: clean all
