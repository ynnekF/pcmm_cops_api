NAME=pcmm_cops_api

SHELL=/bin/bash

# GNU make knows how to execute several recipes at once. Normally, make will execute only one
# recipe at a time, waiting for it to finish before executing the next. However, the ‘-j’ or
# ‘--jobs’ option tells make to execute many recipes simultaneously. Not defining the option
# SINGLE_CPU_EXEC will allow this makefile to use the maximum number of physical CPUs.
# See link for more info (https://www.gnu.org/software/make/manual/html_node/Parallel.html)
ifndef SINGLE_CPU_EXEC
    NCPU 		?= $(shell (nproc --all || sysctl -n hw.physicalcpu) 2>/dev/null || echo 1)
    MAKEFLAGS	+= --jobs=$(NCPU)
endif

# Path to executable `gcc` (/usr/bin/gcc). MUST be installed prior to using this makefile.
# (brew install gcc, apt-get install -y gcc, etc.)
CC=$(shell which gcc)

# Included a GCC version check and GNU make check for visibility but it may not be required.
ifneq ($(firstword $(shell $(firstword $(MAKE)) --version)),GNU)
$(error GNU make is not installed.)
endif

GCC_VERSION=$(shell $(CC) -dumpversion)

ifneq ($(GCC_VERSION),$(filter $(GCC_VERSION),15.0.0 14.1.0))
$(warning Untested GCC version)
endif

# Directory Structure Options
CWD=$(shell pwd)

BID=$(CWD)/data/.bid

# Source file suffix
CEXT=c
# Header file suffix
HEXT=h

# Directory with source files
SRC_DIR=src
# C Source test file directory
TPU_DIR=test
# Directory for compiled output
BIN_DIR=bin

# C Compiler flags
CFLAGS= -g

# Enfore executable is recompiled when header files are changed. When used with -M or -MM,
# specifies a file to write the dependencies to. If no -MF switch is given the preprocessor
# sends the rules to the same place it would send preprocessed output. When used with the
# driver options -MD or -MMD, -MF overrides the default dependency output file.
C_DEPS=-MMD -MF $(@:.o=.d)

# Preprocessor macro definitions. These values will be predefined on each
# source file before compilation. Currently there are two optional macros.
# ex. -DUNIT_TEST
C_DEFINES=-DBUILD_UUID_FILE=\"data/.bid\"

# List of directories and libraries to be searched for header files.
# The files under include will be searched before anything else.
# ex. -I/opt/homebrew/include/
C_INCLUDES=-Isrc/cops -Itest

# Compiler warning flags
#
# Wall: 			Enable all warnings
# Wextra: 			Enable extra warnings
# Wpedantic: 		Enable pedantic warnings
# Wformat=2:		Enable Wformat plus checks
# Wshawdow:			Warn on shadowed variables
# Wwrite-strings:   Copying the address of one into a non-const char * pointer produces a warning
#
C_WARNINGS= -Wall -Wextra -Wpedantic -Wshadow -Wstrict-prototypes -Wredundant-decls\
			-Wnested-externs -Wmissing-include-dirs -Wold-style-definition -Wno-unused-parameter

CFLAGS+=$(C_INCLUDES)
CFLAGS+=$(C_WARNINGS)
CFLAGS+=$(C_DEFINES)
CFLAGS+=$(C_DEPS)

ifeq ($(OS),Windows_NT)
    OS = windows
else
    KERNEL=$(shell uname -s)
	ifeq ($(KERNEL),Darwin)
        OS = macos
	else ifeq ($(KERNEL),Linux)
        OS = linux
	else
$(error unsupported OS)
	endif
endif

ifeq ($(OS),$(filter $(OS), macos linux))
    HEADER_FILES := $(shell find src test -name *.$(HEXT))
    FORMAT_FILES := $(shell find src test -name *.$(CEXT) -o -name *.$(HEXT))
    SOURCE_FILES := $(shell find src test -type f -name "*.c" -exec basename {} \;)
    OBJECT_FILES := $(patsubst %.c,$(BIN_DIR)/obj/%.o,$(SOURCE_FILES))
endif

########################################################################
##                          	Targets		                          ##
########################################################################

define ctemplate
$(BIN_DIR)/obj/%.o: $(1)/%.c $(HEADER_FILES) | $(BIN_DIR)
	$$(CC) $$(CFLAGS) -c -o $$@ $$< 
endef

$(VERBOSE).SILENT:
.PHONY: test clean compilation-database format config

DIRS:=src src/cops test
$(eval $(foreach f,$(DIRS),$(eval $(call ctemplate,$(f)))))

$(BIN_DIR)/$(NAME): $(OBJECT_FILES)
	$(eval BINS	:= $(shell find $(BIN_DIR) -name '*.o'))
	$(CC) -o $@ $(BINS) $(CFLAGS) $(LIBS)
	@if ! test -f $(BID); then echo 0 > $(BID); fi
	@echo $$(($$(cat $(BID)) + 1)) > $(BID)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)/obj

test: $(BIN_DIR)/$(NAME)
	$(BIN_DIR)/$(NAME)

clean:
	rm -rf $(CWD)/$(BIN_DIR)

compilation-database: clean
	compiledb make

format:
	clang-format --style=file:.clang-format --verbose -i $(shell find src test -iname *.$(CEXT) -o -iname *.$(HEXT))
