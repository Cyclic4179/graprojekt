CC := gcc

# -Wall:			Print warnings
# -Wextra:			Enable additional warnings not covered by "-Wall"
# -g				Generates debug information to be used by GDB debugger
CFLAGS := -Wall -Wextra -g -O2

TARGET_EXEC := main

BUILD_DIR := ./build
SRC_DIR := ./Implementierung
TESTS_DIR := ./tests

SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

.PHONY: build run debug test clean

# final build
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@

# build steps
$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


build: $(BUILD_DIR)/$(TARGET_EXEC)

run: build
	$(BUILD_DIR)/$(TARGET_EXEC) -a ./Implementierung/sample-inputs/1.txt -b ./Implementierung/sample-inputs/1.txt -o ./result

debug: build
	gdb --args $(BUILD_DIR)/$(TARGET_EXEC) -a ./Implementierung/sample-inputs/1.txt -b ./Implementierung/sample-inputs/1.txt -o /dev/null


test: build
	for i in $(shell ls $(TESTS_DIR)); do \
		got=$$($(BUILD_DIR)/$(TARGET_EXEC) -a $(TESTS_DIR)/$$i/a -b $(TESTS_DIR)/$$i/b); \
		expected=$$(cat $(TESTS_DIR)/$$i/res); \
		( ! "$$got" = "$$expected" ) && { \
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
		}; \
	done && echo SUCCESS || exit 0

clean:
	if [[ -d $(BUILD_DIR) ]]; then rm -r $(BUILD_DIR); fi


# vim set noexpandtab

