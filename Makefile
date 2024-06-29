CC := gcc

# -Wall:			Print warnings
# -Wextra:			Enable additional warnings not covered by "-Wall"
# -g				Generates debug information to be used by GDB debugger
CFLAGS := -Wall -Wextra -g -O2

TARGET_EXEC := main

BUILD_DIR := ./build
SRC_DIR := ./Implementierung
SRCS := $(shell find $(SRC_DIR) -name '*.c')

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

.PHONY: build test debug run clean
# TODO: test things

# final build
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@

# build steps
$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


build: $(BUILD_DIR)/$(TARGET_EXEC)

run: build
	$(BUILD_DIR)/$(TARGET_EXEC)

debug: build
	gdb $(BUILD_DIR)/$(TARGET_EXEC)

clean:
	if [[ -d $(BUILD_DIR) ]]; then rm -r $(BUILD_DIR); fi

