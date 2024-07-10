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
CDEBUGFLAGS := -g -O0 -DDEBUG
CRELEASEFLAGS := -O2


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

.PHONY: build run debug test clean help

build: CFLAGS += $(CRELEASEFLAGS)
build: MODE := $(RELEASE_MODE)
build: .check-mode $(TARGET_EXEC)
	echo $(RELEASE_MODE) > $(MODE_FILE)

#run: build
#	./$(TARGET_EXEC) -a $(INPUT_DIR)/1.txt -b $(INPUT_DIR)/2.txt

debug: CFLAGS += $(CDEBUGFLAGS)
debug: MODE := $(DEBUG_MODE)
debug: .check-mode $(TARGET_EXEC)
	echo $(DEBUG_MODE) > $(MODE_FILE)
	@#gdb --args ./$(TARGET_EXEC) -a $(INPUT_DIR)/1.txt -b $(INPUT_DIR)/2.txt -o /dev/null

.check-mode:
	if test $(LAST_MODE) != $(MODE); then $(MAKE) clean; fi

# final build
$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@

# build steps
$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

test: build
	@for i in $(shell ls $(TESTS_DIR)); do \
		got=$$(./$(TARGET_EXEC) -a $(TESTS_DIR)/$$i/a -b $(TESTS_DIR)/$$i/b 2>/dev/null); \
		expected=$$(cat $(TESTS_DIR)/$$i/res); \
		if [[ "$$got" = "$$expected" ]] then { \
			echo ---------------------; \
			echo PASSED: testcase $$i; \
			echo ; \
		} else { \
			echo ; \
			echo ---------------------; \
			echo FAILED: testcase $$i; \
			echo ; \
			echo factor a:; \
			echo $$(cat $(TESTS_DIR)/$$i/a); \
			echo ; \
			echo factor b:; \
			echo $$(cat $(TESTS_DIR)/$$i/b); \
			echo ; \
			echo expected result:; \
			echo $$expected; \
			echo ; \
			echo but got:; \
			echo $$got; \
			false; \
		}; fi \
	done && echo SUCCESS || exit 0

clean:
	if test -d $(BUILD_DIR); then rm -r $(BUILD_DIR); fi
	if test -e $(TARGET_EXEC); then rm $(TARGET_EXEC); fi

help:
	@echo available targets:
	@echo - build \(default target\)
	@echo - debug \(debug symbols and extra output when running\)
	@echo - release \(no debug output, performance\)
	@echo - test
	@echo - clean
	@echo - help \(display this help\)

# vim: noexpandtab
