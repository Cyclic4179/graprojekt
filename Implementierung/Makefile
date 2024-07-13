CC := gcc

# -Wall:			Print warnings
# -Wextra:			Enable additional warnings not covered by "-Wall"
# -Wpedantic:		Issue warnings needed for strict compliance to the standard
# -g				Generates debug information to be used by GDB debugger
# -std=c17			Use c 17 standard
# -std=gnu17		Use gnu 17 standard (for getline and <time.h>)
# -D_POSIX_C_SOURCE=199309L
#					Include libs to use clock_gettime(CLOCK_MONOTONIC, ...);
CFLAGS := -Wall -Wextra -Wpedantic -std=gnu17
CRELEASEFLAGS := -O2
CDEBUGFLAGS := -g -Og -DDEBUG
CSANITIZEFLAGS := $(CDEBUGFLAGS) -fsanitize=address -fsanitize=undefined -fsanitize-address-use-after-scope


TARGET_EXEC := main

BUILD_DIR := ./build
SRC_DIR := .
TESTS_DIR := ./tests
INPUT_DIR := ./sample-inputs

SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)


MODE_FILE := $(BUILD_DIR)/.last-mode
LAST_MODE := $(shell [ -f $(MODE_FILE) ] && cat $(MODE_FILE) || echo none)
RELEASE_MODE := release
DEBUG_MODE := debug
SANITIZE_MODE := sanitize


.PHONY: build debug sanitize run test clean help .check-mode


build: CFLAGS += $(CRELEASEFLAGS)
build: MODE := $(RELEASE_MODE)
build: .check-mode $(TARGET_EXEC)

debug: CFLAGS += $(CDEBUGFLAGS)
debug: MODE := $(DEBUG_MODE)
debug: .check-mode $(TARGET_EXEC)

sanitize: CFLAGS += $(CSANITIZEFLAGS)
sanitize: MODE := $(SANITIZE_MODE)
sanitize: .check-mode $(TARGET_EXEC)


.check-mode:
	@if test $(LAST_MODE) != $(MODE); then $(MAKE) clean; mkdir -p $(BUILD_DIR); echo $(MODE) > $(MODE_FILE); fi


# final build
$(TARGET_EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

# build steps
$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@



run: debug
	./$(TARGET_EXEC) -a $(INPUT_DIR)/1.txt -b $(INPUT_DIR)/2.txt


test: build
	@(cd $(TESTS_DIR) ; ./test.sh $(realpath $(TARGET_EXEC)))


clean:
	if test -d $(BUILD_DIR); then rm -r $(BUILD_DIR); fi
	if test -e $(TARGET_EXEC); then rm $(TARGET_EXEC); fi


help:
	@echo available targets:
	@echo - build \(default target, no debug output, performance\)
	@echo - debug \(debug symbols and extra output when running\)
	@echo - sanitize \(same as debug, but also include sanitizers\)
	@echo - test
	@echo - clean \(remove generate files\)
	@echo - help \(display this help\)

# vim: noexpandtab
