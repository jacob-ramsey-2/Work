# === Project Configuration ===
TARGET := ./work
CC := gcc
CFLAGS := -Werror -O3

# === Source Files ===
SRC := main.c

# === Default Build ===
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)
	@echo "Build complete: $(TARGET)"

# === Clean Up ===
clean:
	rm -f $(TARGET)

# === Run Program ===
run: all
	./$(TARGET)

.PHONY: all clean run
