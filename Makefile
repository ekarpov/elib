###########################################################
# ELib Makefile

# project
ELIB := elib

# target names
ELIB_LIB := lib$(ELIB).a
ELIB_TESTS := $(ELIB)_tests

# recursive wildcard (credit: https://stackoverflow.com/a/18258352)
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

###########################################################
# git version

GIT_COMMIT := $(shell git rev-parse --short HEAD)
GIT_DATE := $(shell git log -1 --format=%cd --date=format:"%Y%m%d")

ELIB_VERSION := $(GIT_COMMIT)-$(GIT_DATE)

###########################################################
# compliler options

CCX := gcc
ARX := ar
CFLAGS := -m64 -O -Wall -MMD -DELIB_VERSION="\"$(ELIB_VERSION)\""
LIBS := -lm
INCLUDE :=

# folders
BINDIR := build/binaries
SRCDIR := src


# sources (exclude windows variants)
SRC_LIB_WIN := $(call rwildcard, $(SRCDIR), *_win.c)
SRC_LIB := $(filter-out $(SRC_LIB_WIN), $(call rwildcard, $(SRCDIR), *.c))
SRC_TESTS := 

# object files
OBJ_LIB := $(SRC_LIB:%.c=$(BINDIR)/%.o)

# object file dependencies
-include $(OBJ_LIB:.o=.d)

###########################################################
# targets

.PHONY: lib 

all: clean lib tests

lib: $(BINDIR)/$(ELIB_LIB)

tests: $(BINDIR)/$(ELIB_TESTS)

clean: 
	rm -rf $(BINDIR)
	
$(BINDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CCX) -c -o $(BINDIR)/$(dir $<)$(notdir $@) $< $(CFLAGS) $(INCLUDE)


$(BINDIR)/$(ELIB_LIB): $(OBJ_LIB)
	@mkdir -p $(dir $@)
	$(ARX) rcs -o $@ $^

$(BINDIR)/$(ELIB_TESTS):
	echo $(SRC_LIB)

