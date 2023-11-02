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

# C compiler
CCX := gcc
ARX := ar
CFLAGS := -m64 -O -Wall -MMD -DELIB_VERSION="\"$(ELIB_VERSION)\""
LIBS := -lm
INCLUDE :=

# C++ compiler for google test
CXX = g++
CXXFLAGS = -std=c++17 -Wall -I h -I /usr/local/include/gtest/ -c
LXXFLAGS = -std=c++17 -I h -pthread

# folders
BINDIR := build/binaries
SRCDIR := src
TESTSDIR := tests

# PREFIX is environment variable, but if it is not set, then set default value
ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

# for custom installtion path overwrite this in make parameters
# make install INSTALL_DIR=/path/to/install
INSTALL_DIR := $(PREFIX)

###########################################################
# common targets

all: clean lib

clean: 
	rm -rf $(BINDIR)

###########################################################
# library

lib: $(BINDIR)/$(ELIB_LIB)

# sources (exclude windows variants)
SRC_LIB_WIN := $(call rwildcard, $(SRCDIR), *_win.c)
SRC_LIB := $(filter-out $(SRC_LIB_WIN), $(call rwildcard, $(SRCDIR), *.c))

# object files
OBJ_LIB := $(SRC_LIB:%.c=$(BINDIR)/%.o)

$(BINDIR)/$(ELIB_LIB): $(OBJ_LIB)
	@mkdir -p $(dir $@)
	$(ARX) rcs -o $@ $^

$(BINDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CCX) -c -o $(BINDIR)/$(dir $<)$(notdir $@) $< $(CFLAGS) $(INCLUDE)

# object file dependencies
-include $(OBJ_LIB:.o=.d)


###########################################################
# tests

tests: $(BINDIR)/$(ELIB_LIB) $(BINDIR)/$(ELIB_TESTS)

# sources 
SRC_TESTS := $(call rwildcard, $(TESTSDIR), *.cpp)

# object files
OBJ_TESTS := $(SRC_TESTS:%.cpp=$(BINDIR)/%.o)

# gtest library
GTEST = /usr/local/lib/libgtest.a /usr/local/lib/libgtest_main.a

$(BINDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) -c -o $(BINDIR)/$(dir $<)$(notdir $@) $< $(CXXFLAGS) 
    
$(BINDIR)/$(ELIB_TESTS): $(OBJ_TESTS)
	$(CXX) $(LXXFLAGS) -o $@ $^ $(GTEST) $(BINDIR)/$(ELIB_LIB)

# object file dependencies
-include $(OBJ_TESTS:.o=.d)

###########################################################
# Install

ELIB_HEADERS := $(call rwildcard, $(SRCDIR), *.h)
ELIB_INSTALL_DIR_HEADERS := $(INSTALL_DIR)/include/$(ELIB)
ELIB_INSTALL_DIR_LIB := $(INSTALL_DIR)/lib
ELIB_INSTALL_HEADERS := $(subst $(SRCDIR), $(ELIB_INSTALL_DIR_HEADERS), $(ELIB_HEADERS))

$(ELIB_INSTALL_DIR_HEADERS)/%.h: $(SRCDIR)/%.h
	@mkdir -p $(dir $@)
	install -m 644 $< $@

install_dirs:
	install -d $(ELIB_INSTALL_DIR_HEADERS)
	install -d $(ELIB_INSTALL_DIR_LIB)

install: $(BINDIR)/$(ELIB_LIB) install_dirs $(ELIB_INSTALL_HEADERS)
	install -m 644 $(BINDIR)/$(ELIB_LIB) $(ELIB_INSTALL_DIR_LIB)


