CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -g -Iinclude
TARGET = build/wallet

$(TARGET): $(wildcard src/*.c) $(wildcard include/*.h)
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ $(filter %.c,$^)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf build

.PHONY: run clean

# SETUP DE COMANDOS HECHO CON IA
