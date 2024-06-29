.PHONY: all

all:

INCLUDEDIRS = include
SOURCE = main.c

# -Wall:		        Print warnings
# -Wextra:		        Enable additional warnings not covered by "-Wall"
# -g					Generates debug information to be used by GDB debugger
WFLAGS = -Wall -Wextra -g

main : FORCE
	$(CC) $(SOURCE) $(INCLUDEDIRS:%=-I%) -o $@.out -std=c17 -O0 $(WFLAGS)