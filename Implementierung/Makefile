CC := gcc

# -Wall:			Print warnings
# -Wextra:			Enable additional warnings not covered by "-Wall"
# -Wpedantic:		Issue warnings needed for strict compliance to the standard
# -g				Generates debug information to be used by GDB debugger
# -std=c17          Use c 17 standard
# -std=gnu17        Use gnu 17 standard
# -D_POSIX_C_SOURCE=199309L
#                   Include libs to use clock_gettime(CLOCK_MONOTONIC, ...);
CFLAGS := -Wall -Wextra -Wpedantic -std=gnu17

TARGET_EXEC := main

BUILD_DIR := ./build
SRC_DIR := .
TESTS_DIR := ./tests
INPUT_DIR := ./sample-inputs

SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

.PHONY: build run debug test clean

# final build
$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@

# build steps
$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


build: $(TARGET_EXEC)

run: build
	./$(TARGET_EXEC) -a $(INPUT_DIR)/1.txt -b $(INPUT_DIR)/2.txt

debug: CFLAGS += -g
debug: build
	gdb --args ./$(TARGET_EXEC) -a $(INPUT_DIR)/1.txt -b $(INPUT_DIR)/2.txt -o /dev/null

O2: CFLAGS += -O2
O2: build
	gdb --args ./$(TARGET_EXEC) -a $(INPUT_DIR)/1.txt -b $(INPUT_DIR)/2.txt -o /dev/null

test: build
	for i in $(shell ls $(TESTS_DIR)); do \
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
	if test -d ./build; then rm -r ./build; fi
	if test -e ./main; then rm ./main; fi

# vim set noexpandtab
