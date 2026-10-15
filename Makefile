CC     = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -g -Iinclude -MMD -MP

BUILD  = build
TARGET = $(BUILD)/wallet

SRC = src/main.c src/wallet.c src/operation_list.c src/date.c
OBJ = $(SRC:src/%.c=$(BUILD)/%.o)
DEP = $(OBJ:.o=.d)

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD)/%.o: src/%.c
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD)

-include $(DEP)
