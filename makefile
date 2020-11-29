LANG := c

# TOOLS
ifeq ($(LANG),c)
	CC := gcc
else
	CC := g++
endif
# OPTIONS
LEVEL ?= 3
DEBUG ?= -DNDEBUG -D_FORTIFY_SOURCE=0
OPTFL ?= -march=native -mtune=native -pipe -fno-plt
WARNINGS ?= -Wall -Werror -Wpedantic -Wunused-result
CFLAGS ?= -std=gnu11 $(WARNINGS) $(DEBUG) -O$(LEVEL) $(OPTFL)

# SOURCE
SRC := kmin.$(LANG) rotinas_$(LANG).o
# ARTIFACT
PROG := kmin

# GENERAL COMMANDS
.PHONY: all run clean debug zip

all: $(PROG)

run: $(PROG)
	./$(PROG)

debug: LEVEL = g
debug: DEBUG = -DDEBUG -D_FORTIFY_SOURCE=2 -ggdb3
debug: OPTFL = -march=native
debug: clean all

clean:
	rm -rf $(PROG) $(PROG).o

show: show.c
	$(CC) $(CFLAGS) $^ -o $@

# BUILDING
$(PROG): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@
